
#include "global.h"

/********************** Internal macros declaration ************************/
#define DOWNPARA_FILE	"SYS_DOWN_EDC"

/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static void RemoveEmvAppCapk(void);
static int  SelectDownloadMode(uchar *pucCommType);
static void TransformIP(const uchar * ip_in, uchar * ip_out);
static void TransformPort(const uchar * port_in, uchar * port_out);

static int  UnpackInstPara(const uchar *psPara);
static int  UnpackDescPara(const uchar *psPara);
static void AfterLoadParaProc(void);
static void  SearchIssuerKeyArraySub(uchar *sIssuerKey, uchar ucAcqKey);
static int  GetDownLoadTelNo(void);
static int  GetDownLoadGprsPara(void);
static int  GetDownLoadLanPara(void);
static int  GetDownLoadWIFIPara(void);
static int  GetDownLoadComm(uchar ucCommType);
static int  GetDownLoadTID(uchar *pszID);
static int  SaveEmvMisc(const uchar *psPara);
static int  SaveEmvApp(const uchar *psPara);
static int  SaveEmvCapk(const uchar *psPara);
static void GetNextAutoDayTime(uchar *pszDateTimeInOut, ushort uiInterval);
static int  SaveDetail(const uchar *psData);
static int  SaveCardBin(uchar *psCardBinInOut);

void InitEdcParam(void);

/********************** Internal variables declaration *********************/
static uchar	sgSyncDial, sgNewTMS;
static uchar	sgTempBuf[1024*20];
static uchar	sgEMVDownloaded;

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
static void GetDefCurrency(CURRENCY_CONFIG *pstConfig)
{
//#ifdef AREA_HK
//	*pstConfig = glCurrency[0];
	*pstConfig = glPosParams.currency;
//#elif defined(AREA_Arabia)
//	*pstConfig = glCurrency[33];
//#else
//	*pstConfig = glCurrency[15];
//#endif
}

// 设置EDC缺省参数
// Set to default EDC parameter
void LoadEdcDefault(void)
{
	logTrace(__func__);
	int			iCnt;

	ResetAllPara(TRUE);

	DelFilesbyPrefix(GetCurSignPrefix(ACQ_ALL));

	memset(&glSysCtrl, 0, sizeof(SYS_CONTROL));
	glSysCtrl.ulInvoiceNo = 1L;
	glSysCtrl.ulSTAN      = 1L;
	glSysCtrl.uiLastRecNo = 0xFFFF;
	glSysCtrl.uiErrLogNo  = 0;
	for(iCnt=0; iCnt<MAX_ACQ; iCnt++)
	{
		glSysCtrl.sAcqStatus[iCnt]		   = S_RESET;
		glSysCtrl.stField56[iCnt].uiLength = 0;
		glSysCtrl.uiLastRecNoList[iCnt]    = 0xFFFF;
	}

	for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
	{
		glSysCtrl.sAcqKeyList[iCnt]    = INV_ACQ_KEY;		// set to invalid acquirer key
		glSysCtrl.sIssuerKeyList[iCnt] = INV_ISSUER_KEY;	// set to invalid issuer key
	}
	glSysCtrl.stWriteInfo.bNeedSave = SAVE_NONEED;

	SaveSysCtrlAll();

	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();
}


void LoadEmvDefault(void)
{
	CURRENCY_CONFIG		stLocalCurrency = glPosParams.currency;
	RemoveEmvAppCapk();

	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();
	
	// set default EMV library parameters
	EMVGetParameter(&glEmvParam);
	memcpy(glEmvParam.Capability,    EMV_CAPABILITY, 3);
	memcpy(glEmvParam.ExCapability,  "\xE0\x00\xF0\xA0\x01", 5);
	// 这里先设默认货币。参数下载之后根据protims设定的货币来修改EMV库参数
	// set with default currency, and modify those EMV data with settings from Protims after remote download. 
	memcpy(glEmvParam.CountryCode,   stLocalCurrency.sCountryCode,  2);
	memcpy(glEmvParam.TransCurrCode, stLocalCurrency.sCurrencyCode, 2);
	memcpy(glEmvParam.ReferCurrCode, stLocalCurrency.sCurrencyCode, 2);
	memcpy(glEmvParam.MerchCateCode, "\x00\x00", 2);
//	glEmvParam.ReferCurrCon  = 0;
	glEmvParam.TransCurrExp  = stLocalCurrency.ucDecimal;
	glEmvParam.ReferCurrExp  = stLocalCurrency.ucDecimal;
	glEmvParam.ForceOnline   = 0;
	glEmvParam.GetDataPIN    = 1;
	glEmvParam.SurportPSESel = 1;
	EMVSetParameter(&glEmvParam);

	InitTestApps();
	InitTestKeys();
	InitLiveApps();
	InitLiveKeys();



}


void LoadDefCommPara(void)
{
	uchar	sBuff[HWCFG_END+1];

	// ================================ 交易参数 ================================
	// ========================= Transaction parameters =========================
	// 设置回调函数
	// Setup callback function used in waiting response display
	glSysParam.stTxnCommCfg.pfUpdWaitUI   = DispWaitRspStatus;

	// TMS comm type
	glSysParam.stTMSCommCfg.ucCommType    = CT_GPRS;	// will not be set until fun0
	glSysParam.stTMSCommCfg.ucCommTypeBak = CT_GPRS;

	// TXN default comm type
	GetTermInfo(sBuff);
	if ((sBuff[HWCFG_MODEL]==_TERMINAL_S90_))
	{
		if (sBuff[HWCFG_GPRS]!=0)
		{
			glSysParam.stTxnCommCfg.ucCommType = CT_GPRS;
		}
		else if (sBuff[HWCFG_CDMA]!=0)
		{
			glSysParam.stTxnCommCfg.ucCommType = CT_CDMA;
		}
		else if (sBuff[HWCFG_WCDMA]!=0)     // added by  Gillian 2015/11/23
		{
			glSysParam.stTxnCommCfg.ucCommType = CT_WCDMA;
		}
	}
	else if (sBuff[HWCFG_MODEM]!=0)
	{
		glSysParam.stTxnCommCfg.ucCommType = CT_MODEM;
	}
	else
	{
		glSysParam.stTxnCommCfg.ucCommType = CT_RS232;
	}

	glSysParam.stTxnCommCfg.ucCommTypeBak  = CT_NONE;

	// TCP length header format.
	glSysParam.stTMSCommCfg.ucTCPClass_BCDHeader = TRUE;
	glSysParam.stTxnCommCfg.ucTCPClass_BCDHeader = TRUE;

	// 交易拨号缺省参数
	// Default dial parameter in transaction
	// please see details in API manual
	glSysParam._TxnPSTNPara.ucSendMode = CM_SYNC;
    glSysParam._TxnPSTNPara.ucSignalLevel = 0;
	glSysParam._TxnModemPara.DP      = 0;
	glSysParam._TxnModemPara.CHDT    = 0;
	glSysParam._TxnModemPara.DT1     = 20;		// 等候拨号音的最长时间(20~~255), 单位: 100ms
	glSysParam._TxnModemPara.DT2     = 10;		// 拨外线时","等待时间(0~~255), 单位: 100ms
	glSysParam._TxnModemPara.HT      = 70;		// 双音拨号单一号码保持时间(单位:1ms,有效范围50~255)
	glSysParam._TxnModemPara.WT      = 5;		// 双音拨号两个号码之间的间隔时间(单位:10ms,有效范围5~25)
	glSysParam._TxnModemPara.SSETUP  = 0x05;	// 0x45: 9600bps, 0x05:1200 bps	// 通讯字节
	glSysParam._TxnModemPara.DTIMES  = 1;		// 循环拨号总次数,拨完拨号串的所有号码为一次[有效范围1~255]
	glSysParam._TxnModemPara.TimeOut = 6;		// 没有数据交换MODEM挂断等待时间;以10秒为单位,为0时无超时,最大值65
	glSysParam._TxnModemPara.AsMode  = 0;		// 异步通讯的速率

	// RS232缺省参数
	// Default parameter in transaction for RS232
#ifdef _WIN32
#if defined(_Sxx_)
	glSysParam._TxnRS232Para.ucPortNo   = PINPAD;
#else
	glSysParam._TxnRS232Para.ucPortNo   = COM2;
#endif
#else
	glSysParam._TxnRS232Para.ucPortNo   = COM1;
#endif
	glSysParam._TxnRS232Para.ucSendMode = CM_SYNC;
	sprintf((char *)glSysParam._TxnRS232Para.szAttr, "9600,8,n,1");
#ifdef _Dxxx_
	glSysParam._TxnBlueToothPara.stCommParam.ucPortNo =  COM_BT;
	glSysParam._TxnBlueToothPara.stCommParam.ucSendMode = CM_RAW;//hdadd // Modified by Kim_LinHB 2014-08-18 v1.01.0004 from Sync to Raw
	sprintf((char *)glSysParam._TxnBlueToothPara.stCommParam.szAttr, "115200,8,n,1");//hdadd // Modified by Kim_LinHB 2014-08-15 v1.01.0004

	// Added by Kim_LinHB 2014-08-15 v1.01.0004
#ifdef _MIPS_
	glSysParam._TxnBlueToothPara.stConfig.role = BT_ROLE_SLAVE;
	glSysParam._TxnBlueToothPara.stConfig.baud = 115200;
#endif
	strcpy(glSysParam._TxnBlueToothPara.stConfig.name, "Dxxx ");
	ReadSN(glSysParam._TxnBlueToothPara.stConfig.name + strlen(glSysParam._TxnBlueToothPara.stConfig.name));
	strcpy(glSysParam._TxnBlueToothPara.stConfig.pin, "0000");
#endif

	// TCP/IP缺省参数
	// Default parameter in transaction for TCPIP
	glSysParam._TxnTcpIpPara.ucDhcp = 1;
	glSysParam._TxnTcpIpPara.szNetMask[0] = 0;
	glSysParam._TxnTcpIpPara.szGatewayIP[0] = 0;
	glSysParam._TxnTcpIpPara.szLocalIP[0] = 0;
    memset(&glSysParam._TxnTcpIpPara.stHost1, 0, sizeof(IP_ADDR));
    memset(&glSysParam._TxnTcpIpPara.stHost2, 0, sizeof(IP_ADDR));
	glSysParam._TxnTcpIpPara.szDNSIP[0] = 0;
	
	// Default parameter in transaction for WIFI
	memset(&glSysParam._TxnWifiPara,0x00,sizeof(WIFI_PARA));
	memcpy(&glSysParam._TxnWifiPara,&glSysParam._TxnTcpIpPara,sizeof(WIFI_PARA));
	//glSysParam._TxnWifiPara.ucPortNo = 5;
	glSysParam._TxnWifiPara.stParam.DhcpEnable = TRUE;//Dhcp

	// GPRS/CDMA缺省参数
	// Default parameter in TMS for GPRS/CDMA
	glSysParam._TxnWirlessPara.szAPN[0] = 0;
	glSysParam._TxnWirlessPara.szUID[0] = 0;
	glSysParam._TxnWirlessPara.szPwd[0] = 0;
	glSysParam._TxnWirlessPara.szSimPin[0] = 0;
	glSysParam._TxnWirlessPara.szDNS[0] = 0;
    memset(&glSysParam._TxnWirlessPara.stHost1, 0, sizeof(IP_ADDR));
    memset(&glSysParam._TxnWirlessPara.stHost2, 0, sizeof(IP_ADDR));

	// ================================ 下载参数 ================================
	// ====================== TMS communication parameters ======================
	// 设置回调函数
	// Setup callback function in TMS download
	glSysParam.stTMSCommCfg.pfUpdWaitUI = DispWaitRspStatus;
	glSysParam.stTMSCommCfg.ucCommType  = CT_MODEM;

	// 参数下载缺省参数
	// Default dial parameter in TMS download
	glSysParam._TmsModemPara.DP      = 0;
	glSysParam._TmsModemPara.CHDT    = 0x40;
	glSysParam._TmsModemPara.DT1     = 5;
	glSysParam._TmsModemPara.DT2     = 7;
	glSysParam._TmsModemPara.HT      = 70;
	glSysParam._TmsModemPara.WT      = 5;
	glSysParam._TmsModemPara.SSETUP  = 0x87;	/* asynchronise link */
	glSysParam._TmsModemPara.DTIMES  = 1;
	glSysParam._TmsModemPara.TimeOut = 6;
	glSysParam._TmsModemPara.AsMode  = 0xF0;

	// RS232缺省参数(TMS)
	// RS232 para in TMS download
	memcpy(&glSysParam._TmsRS232Para, &glSysParam._TxnRS232Para, sizeof(glSysParam._TmsRS232Para));

	// TCP/IP缺省参数
	// TCP/IP para in TMS download
	memcpy(&glSysParam._TmsTcpIpPara, &glSysParam._TxnTcpIpPara, sizeof(glSysParam._TmsTcpIpPara));

	// GPRS/CDMA缺省参数
	// GPRS/CDMA para in TMS download
	memcpy(&glSysParam._TmsWirlessPara, &glSysParam._TxnWirlessPara, sizeof(glSysParam._TmsWirlessPara));
}

void ResetAllPara(uchar bFirstTime)
{
	int				iCnt;
	uchar			ucNewTmsBak, ucTMSSyncDial;
	uchar			szDownTelNo[25+1], szDownLoadTID[8+1], szPabx[10+1];
	IP_ADDR			stTmsIP;
	uchar			sEdcExtOptions[sizeof(glSysParam.stEdcInfo.sExtOption)];
	uchar			ucCommType, ucCommTypeBak, ucIdleMin, ucIdleOpt;
	TCPIP_PARA		stBakTmsTcpip, stBakTxnTcpip;
	WIRELESS_PARAM	stBakTmsWireless, stBakTxnWireless;
	// and WIFI, ...
	LANG_CONFIG		stLangBak;

	// Backup
	if( !bFirstTime )
	{
		ucNewTmsBak   = glSysParam.ucNewTMS;
		ucTMSSyncDial = glSysParam.ucTMSSyncDial;
		sprintf((char *)szDownTelNo,   "%.25s", glSysParam.stEdcInfo.szDownTelNo);
		sprintf((char *)szDownLoadTID, "%.8s",  glSysParam.stEdcInfo.szDownLoadTID);
		memcpy(&stTmsIP, &glSysParam.stEdcInfo.stDownIpAddr, sizeof(IP_ADDR));
		memcpy(sEdcExtOptions, glSysParam.stEdcInfo.sExtOption, sizeof(sEdcExtOptions));

		ucCommType    = glSysParam.stTxnCommCfg.ucCommType;
		ucCommTypeBak = glSysParam.stTxnCommCfg.ucCommTypeBak;
		memcpy(&stBakTmsTcpip, &glSysParam._TmsTcpIpPara, sizeof(TCPIP_PARA));
		memcpy(&stBakTxnTcpip, &glSysParam._TxnTcpIpPara, sizeof(TCPIP_PARA));
		memcpy(&stBakTmsWireless, &glSysParam._TmsWirlessPara, sizeof(WIRELESS_PARAM));
		memcpy(&stBakTxnWireless, &glSysParam._TxnWirlessPara, sizeof(WIRELESS_PARAM));
		memcpy(szPabx, glSysParam.stEdcInfo.szPabx, sizeof(szPabx));

		stLangBak = glSysParam.stEdcInfo.stLangCfg;
		ucIdleMin = glSysParam.stEdcInfo.ucIdleMinute;
		ucIdleOpt = glSysParam.stEdcInfo.ucIdleShutdown;
	}

	memset(&glSysParam, 0, sizeof(SYS_PARAM));

	LoadDefCommPara();
	if (bFirstTime)
	{
		LoadDefaultLang();
	}
	
	glSysParam.ucTermStatus              = INIT_MODE;
	glSysParam.stEdcInfo.bPreDial        = TRUE;
	glSysParam.stEdcInfo.ucScrGray       = 4;
	glSysParam.stEdcInfo.ucAcceptTimeout = 3;
	glSysParam.stEdcInfo.ucTMSTimeOut    = 60;
	glSysParam.stEdcInfo.ucIdleMinute    = 1;
	glSysParam.stEdcInfo.ucIdleShutdown  = 0;
	sprintf((char *)glSysParam.stEdcInfo.szTMSNii, "000");

	ResetPwdAll();

	// Recover
	if( !bFirstTime )
	{
		glSysParam.ucNewTMS      = ucNewTmsBak;
		glSysParam.ucTMSSyncDial = ucTMSSyncDial;
		sprintf((char *)glSysParam.stEdcInfo.szDownTelNo,   "%.25s", szDownTelNo);
		sprintf((char *)glSysParam.stEdcInfo.szDownLoadTID, "%.8s",  szDownLoadTID);
		memcpy(&glSysParam.stEdcInfo.stDownIpAddr, &stTmsIP, sizeof(IP_ADDR));
		memcpy(glSysParam.stEdcInfo.sExtOption, sEdcExtOptions, sizeof(glSysParam.stEdcInfo.sExtOption));
		
		glSysParam.stTxnCommCfg.ucCommType    = ucCommType;
		glSysParam.stTxnCommCfg.ucCommTypeBak = ucCommTypeBak;

		memcpy(&glSysParam._TmsTcpIpPara, &stBakTmsTcpip, sizeof(TCPIP_PARA));
		memcpy(&glSysParam._TxnTcpIpPara, &stBakTxnTcpip, sizeof(TCPIP_PARA));

		memcpy(&glSysParam._TmsWirlessPara, &stBakTmsWireless, sizeof(WIRELESS_PARAM));
		memcpy(&glSysParam._TxnWirlessPara, &stBakTxnWireless, sizeof(WIRELESS_PARAM));

		memcpy(glSysParam.stEdcInfo.szPabx, szPabx, sizeof(glSysParam.stEdcInfo.szPabx));

		glSysParam.stEdcInfo.stLangCfg = stLangBak;
		glSysParam.stEdcInfo.ucIdleMinute = ucIdleMin;
		glSysParam.stEdcInfo.ucIdleShutdown = ucIdleOpt;
	}

	glSysParam.stEdcInfo.ucAutoMode     = 0;	// Don't auto update parameter
	glSysParam.stEdcInfo.uiAutoInterval = 90;
	GetNextAutoDayTime(glSysParam.stEdcInfo.szAutoDayTime, glSysParam.stEdcInfo.uiAutoInterval);

	UpdateTermInfo();
	glSysParam.stEdcInfo.ucPedMode = PED_INT_PCI;

	InitMultiAppInfo();

	SaveSysParam();
}

// Modified by Kim_LinHB 2014-6-8
void NoDownloadInit(void)
{
	logTrace(__func__);
	// Modified by Kim_LinHB 2014-4-4

	Gui_ClearScr();

	InitEdcParam();
	glSysParam.ucTermStatus = TRANS_MODE;
	SaveSysParam();
	SaveSysCtrlAll();
	
	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();

	SetSystemParamAll();
	SaveSysParam();

}


#ifdef ENABLE_EMV
void RemoveEmvAppCapk(void)
{
	int				iCnt;
	int				iRet;
	EMV_CAPK		stEmvCapk;
	EMV_APPLIST		stEmvApp;

	for(iCnt=0; iCnt<MAX_KEY_NUM; iCnt++)
	{
		memset(&stEmvCapk, 0, sizeof(EMV_CAPK));
		iRet = EMVGetCAPK(iCnt, &stEmvCapk);
		if( iRet==EMV_OK )
		{
			EMVDelCAPK(stEmvCapk.KeyID, stEmvCapk.RID);
		}
	}
	for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
	{
		memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
		iRet = EMVGetApp(iCnt, &stEmvApp);
		if( iRet==EMV_OK )
		{
			EMVDelApp(stEmvApp.AID, (int)stEmvApp.AidLen);
		}
	}
}
#endif

// Modified by Kim_LinHB 2014-6-8
int SelectDownloadMode(uchar *pucCommType)
{
	int		iRet;
	uchar	ucComm;
	GUI_MENU	stSmDownMode;
	int iSelected;
	GUI_MENUITEM stDefCommMenuItem[] =
	{
		{ "MODEM", 1, TRUE, NULL},
		{ "TCPIP", 2,TRUE,  NULL},
		{ "GPRS", 3,TRUE,  NULL},
		{ "CDMA", 4,TRUE,  NULL},
		{ "OLD ASYNC", 5,TRUE,  NULL},
		{ "OLD SYNC", 6,TRUE,  NULL},
		{ "RS232", 7,TRUE,  NULL},
		{ "LOAD DEFAULT", 8,TRUE,  NULL},
		{ "WIFI", 9, TRUE,  NULL}, 
		{ "BLUETOOTH", 10, FALSE,  NULL},
		{ "USB", 11, TRUE,  NULL},
		{ "WCDMA", 12,TRUE,  NULL},
		{ "", -1,FALSE,  NULL},
	};// This menu does not provide translation
	GUI_MENUITEM stCommMenuItem[20];
	int iMenuItemNum = 0;

	//--------------------------------------------------

	if (!ChkHardware(HWCFG_MODEM, HW_NONE) && stDefCommMenuItem[0].bVisible)
	{
	    memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[0], sizeof(GUI_MENUITEM));
		sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[0].szText);
	    ++iMenuItemNum;
	}
	if (!ChkHardware(HWCFG_LAN, HW_NONE) && stDefCommMenuItem[1].bVisible)
	{
        memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[1], sizeof(GUI_MENUITEM));
        sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[1].szText);
        ++iMenuItemNum;
	}
	if (!ChkHardware(HWCFG_GPRS, HW_NONE) && stDefCommMenuItem[2].bVisible)
	{
        memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[2], sizeof(GUI_MENUITEM));
        sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[2].szText);
        ++iMenuItemNum;
	}
	if (!ChkHardware(HWCFG_CDMA, HW_NONE) && stDefCommMenuItem[3].bVisible)
	{
	    memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[3], sizeof(GUI_MENUITEM));
        sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[3].szText);
        ++iMenuItemNum;
	}
	if (!ChkHardware(HWCFG_WCDMA, HW_NONE) && stDefCommMenuItem[11].bVisible)  // added by  Gillian 2015/11/23
	{
	    memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[11], sizeof(GUI_MENUITEM));
	    sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[11].szText);
	    ++iMenuItemNum;
	}

    memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[6], sizeof(GUI_MENUITEM));
    sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[6].szText);
    ++iMenuItemNum;

   	if (!ChkHardware(HWCFG_WIFI, HW_NONE) && stDefCommMenuItem[8].bVisible)	//
	{
         memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[8], sizeof(GUI_MENUITEM));
         sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[8].szText);
         ++iMenuItemNum;
	}

    if (!ChkHardware(HWCFG_MODEM, HW_NONE))
    {
        if(stDefCommMenuItem[4].bVisible)
        {
            memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[4], sizeof(GUI_MENUITEM));
            sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[4].szText);
            ++iMenuItemNum;
        }

        if(stDefCommMenuItem[5].bVisible)
        {
            memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[5], sizeof(GUI_MENUITEM));
            sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[5].szText);
            ++iMenuItemNum;
        }
    }

    memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[10], sizeof(GUI_MENUITEM));
    sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[10].szText);
    ++iMenuItemNum;

#ifdef ALLOW_NO_TMS
   	memcpy(&stCommMenuItem[iMenuItemNum], &stDefCommMenuItem[7], sizeof(GUI_MENUITEM));
    sprintf(stCommMenuItem[iMenuItemNum].szText, "%s", stDefCommMenuItem[7].szText);
    ++iMenuItemNum;
#endif

    stCommMenuItem[iMenuItemNum].szText[0] = 0;

	memset(&stSmDownMode, 0, sizeof(stSmDownMode));
	Gui_BindMenu("SELECT MODE", gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stCommMenuItem, &stSmDownMode);

	Gui_ClearScr();
	iSelected = 0;
	iRet = Gui_ShowMenuList(&stSmDownMode, 0, USER_OPER_TIMEOUT, &iSelected);

	if (iRet != GUI_OK)
	{
		return ERR_USERCANCEL;
	}

	sgSyncDial  = FALSE;
	sgNewTMS    = TRUE;
	ucComm      = CT_NONE;
	switch(iSelected)
	{
	case 1:
		ucComm = CT_MODEM;
		break;
	case 2:
		ucComm = CT_TCPIP;
		break;
	case 3:
		ucComm = CT_GPRS;
	    break;
	case 4:
		ucComm = CT_CDMA;
	    break;
	case 5:
		ucComm = CT_MODEM;
		sgNewTMS = FALSE;
	    break;
	case 6:
		ucComm = CT_MODEM;
		sgNewTMS = FALSE;
		sgSyncDial = TRUE;
		break;
	case 7:
		ucComm = CT_RS232;
		sgSyncDial = TRUE;
		break;
	case 8:
		NoDownloadInit();
		return ERR_NO_DISP;
	case 9:
		ucComm = CT_WIFI;
		break;
	case 10:
		ucComm = CT_BLTH;
		//Hidden by Kim_LinHB 2014-8-16 v1.01.0004
		//sgNewTMS = FALSE;
		//sgSyncDial = TRUE;
		break;
    case 11:
        ucComm = CT_USB;
        break;
    case 12:
    	ucComm = CT_WCDMA;  // added by  Gillian 2015/11/23
    	break;
	default:
	    return ERR_NO_DISP;
	}

	if( GetDownLoadComm(ucComm)!=0 )
	{
		return ERR_NO_DISP;
	}

	iRet = GetDownLoadTID(glSysParam.stEdcInfo.szDownLoadTID);
	if (iRet!=0)
	{
		return ERR_NO_DISP;
	}

	// save TMS download settings.
	glSysParam.stTMSCommCfg.ucCommType = ucComm;
	glSysParam.stTMSCommCfg.stRS232Para.ucSendMode = (sgSyncDial ? CM_SYNC : CM_ASYNC);
	glSysParam.stTMSCommCfg.stPSTNPara.ucSendMode  = (sgSyncDial ? CM_SYNC : CM_ASYNC);
	glSysParam.ucNewTMS      = sgNewTMS;
	glSysParam.ucTMSSyncDial = sgSyncDial;
	SaveSysParam();

	*pucCommType = ucComm;
	return 0;
}

// Monitor下载文件的参数下载
// New Protims download protocol, done by monitor level.
int NewTmsDownLoad(uchar ucCommType)
{
#ifndef _WIN32
	int				iRet;
	T_INCOMMPARA	stCommPara;
	//TMS_LOADSTATUS	stLoadStatus;
	T_LOADSTATUS	stLoadStatus;
	COMM_PARA		stModemPara;
	uchar			szTelNo[25+1+1], szTermID[8+1];
	uchar szBuff[100];

	TCPIP_PARA stLocalInfo;

	memset(&stCommPara, 0, sizeof(T_INCOMMPARA));

    stCommPara.psProtocol = 0;
    stCommPara.ucCallMode = 0;
    stCommPara.bLoadType = 0xFF;

	stCommPara.psAppName  = (uchar *)"";		//(uchar *)AppInfo.AppName;
	sprintf((char *)szTermID, "%.8s", glSysParam.stEdcInfo.szDownLoadTID);
	stCommPara.psTermID   = szTermID;


	switch(ucCommType)
	{
	case CT_MODEM:
		memset(&stModemPara, 0, sizeof(COMM_PARA));
		stModemPara.CHDT    = 0x00;
		stModemPara.DT1     = 0x0A;//5;
		stModemPara.DT2     = 0x0A;//5;
		stModemPara.HT      = 0x64;
		stModemPara.WT      = 0x0A;
		stModemPara.SSETUP  = 0xE7; //0x87;	/* asynchronise link */
		stModemPara.DTIMES  = 0;
		stModemPara.AsMode  = 0x70;
		stModemPara.TimeOut = 6;	// 60 seconds
		stCommPara.bCommMode  = 1;		// modem
		if( glProcInfo.bAutoDownFlag )
		{
			stCommPara.ucCallMode = 0x11;
		}
		sprintf((char *)szTelNo, "%.25s.", glSysParam.stEdcInfo.szDownTelNo);
		stCommPara.tUnion.tModem.psTelNo   = szTelNo;
		stCommPara.tUnion.tModem.bTimeout  = 1;
		stCommPara.tUnion.tModem.ptModPara = &stModemPara;
		break;
	case CT_RS232:
		stCommPara.bCommMode = 0;
		stCommPara.tUnion.tSerial.psPara = (uchar *)glSysParam._TmsRS232Para.szAttr;
		break;
	case CT_TCPIP:
		stCommPara.bCommMode = 2;
		stCommPara.tUnion.tLAN.psLocal_IP_Addr = glSysParam._TmsTcpIpPara.szLocalIP;
		stCommPara.tUnion.tLAN.wLocalPortNo = 1010;
		//stCommPara.tUnion.tLAN.wLocalPortNo = 2;
		stCommPara.tUnion.tLAN.psSubnetMask	= glSysParam._TmsTcpIpPara.szNetMask; 
		stCommPara.tUnion.tLAN.psGatewayAddr = glSysParam._TmsTcpIpPara.szGatewayIP;
		stCommPara.tUnion.tLAN.psRemote_IP_Addr = glSysParam.stEdcInfo.stDownIpAddr.szIP;
		stCommPara.tUnion.tLAN.wRemotePortNo = (ushort)atol((char *)glSysParam.stEdcInfo.stDownIpAddr.szPort);
		break;
	case CT_GPRS:
	case CT_CDMA:
		stCommPara.bCommMode = ((ucCommType==CT_GPRS) ? 3 : 4);
		stCommPara.tUnion.tGPRS.psAPN      = stCommPara.tUnion.tCDMA.psTelNo    = glSysParam._TmsWirlessPara.szAPN;
		stCommPara.tUnion.tGPRS.psUserName = stCommPara.tUnion.tCDMA.psUserName = glSysParam._TmsWirlessPara.szUID;
		stCommPara.tUnion.tGPRS.psPasswd   = stCommPara.tUnion.tCDMA.psPasswd   = glSysParam._TmsWirlessPara.szPwd;
		stCommPara.tUnion.tGPRS.psPIN_CODE = stCommPara.tUnion.tCDMA.psPIN_CODE = glSysParam._TmsWirlessPara.szSimPin;
		stCommPara.tUnion.tGPRS.psIP_Addr  = stCommPara.tUnion.tCDMA.psIP_Addr  = glSysParam.stEdcInfo.stDownIpAddr.szIP;
		stCommPara.tUnion.tGPRS.nPortNo    = stCommPara.tUnion.tCDMA.nPortNo    = (ushort)atol((char *)glSysParam.stEdcInfo.stDownIpAddr.szPort);
		break;
		// added by  Gillian 2015/11/23
	case CT_WCDMA:
		stCommPara.bCommMode = 9;
		stCommPara.tUnion.tGPRS.psAPN      = stCommPara.tUnion.tCDMA.psTelNo    = glSysParam._TmsWirlessPara.szAPN;
		stCommPara.tUnion.tGPRS.psUserName = stCommPara.tUnion.tCDMA.psUserName = glSysParam._TmsWirlessPara.szUID;
		stCommPara.tUnion.tGPRS.psPasswd   = stCommPara.tUnion.tCDMA.psPasswd   = glSysParam._TmsWirlessPara.szPwd;
		stCommPara.tUnion.tGPRS.psPIN_CODE = stCommPara.tUnion.tCDMA.psPIN_CODE = glSysParam._TmsWirlessPara.szSimPin;
		stCommPara.tUnion.tGPRS.psIP_Addr  = stCommPara.tUnion.tCDMA.psIP_Addr  = glSysParam.stEdcInfo.stDownIpAddr.szIP;
		stCommPara.tUnion.tGPRS.nPortNo    = stCommPara.tUnion.tCDMA.nPortNo    = (ushort)atol((char *)glSysParam.stEdcInfo.stDownIpAddr.szPort);
		break;
	case CT_WIFI:
		sprintf(stLocalInfo.szLocalIP, "%d.%d.%d.%d", 
			glSysParam._TmsWifiPara.stParam.Ip[0],
			glSysParam._TmsWifiPara.stParam.Ip[1],
			glSysParam._TmsWifiPara.stParam.Ip[2],
			glSysParam._TmsWifiPara.stParam.Ip[3]);
		sprintf(stLocalInfo.szNetMask, "%d.%d.%d.%d", 
			glSysParam._TmsWifiPara.stParam.Mask[0],
			glSysParam._TmsWifiPara.stParam.Mask[1],
			glSysParam._TmsWifiPara.stParam.Mask[2],
			glSysParam._TmsWifiPara.stParam.Mask[3]);
		sprintf(stLocalInfo.szGatewayIP, "%d.%d.%d.%d", 
			glSysParam._TmsWifiPara.stParam.Gate[0],
			glSysParam._TmsWifiPara.stParam.Gate[1],
			glSysParam._TmsWifiPara.stParam.Gate[2],
			glSysParam._TmsWifiPara.stParam.Gate[3]);
		sprintf(stLocalInfo.szDNSIP, "%d.%d.%d.%d", 
			glSysParam._TmsWifiPara.stParam.Dns[0],
			glSysParam._TmsWifiPara.stParam.Dns[1],
			glSysParam._TmsWifiPara.stParam.Dns[2],
			glSysParam._TmsWifiPara.stParam.Dns[3]);

		stCommPara.bCommMode = 6;//

		stCommPara.tUnion.tWIFI.Wifi_SSID =  glSysParam._TmsWifiPara.stLastAP.Ssid;
		if(WLAN_SEC_WEP == glSysParam._TmsWifiPara.stLastAP.SecMode)
		{
#ifdef _MIPS_
			stCommPara.tUnion.tWIFI.psPasswd =  glSysParam._TmsWifiPara.stParam.Wep;
#else
			stCommPara.tUnion.tWIFI.psPasswd =  glSysParam.stTMSCommCfg.stWifiPara.stParam.Wep.Key[0];
#endif
			stCommPara.tUnion.tWIFI.Encryption_Mode = 2;
			stCommPara.tUnion.tWIFI.Encryption_Index = 1;
		}
		else
		{	
			stCommPara.tUnion.tWIFI.psPasswd = glSysParam._TmsWifiPara.stParam.Wpa;
			if(WLAN_SEC_WPA_WPA2 == glSysParam._TmsWifiPara.stLastAP.SecMode)
			{
				stCommPara.tUnion.tWIFI.Encryption_Mode = 3;
			}
			else
			{
				stCommPara.tUnion.tWIFI.Encryption_Mode = 4;
			}
		}
		stCommPara.tUnion.tWIFI.Local_IP = stLocalInfo.szLocalIP;
		stCommPara.tUnion.tWIFI.Local_PortNo = 1010;
		stCommPara.tUnion.tWIFI.SubnetMask	=stLocalInfo.szNetMask;
		stCommPara.tUnion.tWIFI.GatewayAddr = stLocalInfo.szGatewayIP;
		stCommPara.tUnion.tWIFI.Dns1 = stLocalInfo.szDNSIP;
		stCommPara.tUnion.tWIFI.DHCP_Flag = glSysParam._TmsWifiPara.stParam.DhcpEnable;

		stCommPara.tUnion.tWIFI.Remote_IP_Addr = glSysParam.stEdcInfo.stDownIpAddr.szIP;
		stCommPara.tUnion.tWIFI.RemotePortNo = (ushort)atol((char *)glSysParam.stEdcInfo.stDownIpAddr.szPort);
		break;
		
	case CT_BLTH:
		stCommPara.bCommMode = 0;
		stCommPara.tUnion.tSerial.psPara = (uchar *)"115200,8,n,1";
		break;
	case CT_USB:
	    stCommPara.bCommMode = 7;
	    break;
	default:
		return ERR_NO_DISP;
	}
	
	iRet = RemoteLoadApp(&stCommPara);
	// Modified by Kim_LinHB 2014-6-8
	SaveEdcParam();
	CommOnHook(FALSE);
	Gui_ClearScr();
	sprintf(szBuff, "%d", iRet);
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, 3, NULL);
	if( iRet!=0 )
	{
		return iRet;
	}

	//memset(&stLoadStatus, 0, sizeof(TMS_LOADSTATUS));
	// Removed by Kim_LinHB 2014-08-14 1.01.0003 cuz it is just for old version
// 	memset(&stLoadStatus, 0, sizeof(T_LOADSTATUS));
// 	iRet = GetLoadedAppStatus((uchar *)"", &stLoadStatus);
// 	if( iRet!=0 )
// 	{
// 		return iRet;
// 	}
//
//	if( stLoadStatus.bAppTotal!=0 )
	{
		strcpy(szBuff, _T("SYSTEM UPDATED."));
		strcat(szBuff,"\n");
		if (!ChkTerm(_TERMINAL_S90_))	// S90 do not support soft-reboot
		{
			strcat(szBuff, _T("REBOOT..."));
			Gui_ClearScr();
			Gui_ShowMsgBox(NULL, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, 3, NULL);
			Reboot();
		}
		strcat(szBuff, _T("PLS REBOOT POS."));
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		while(1);
	}

	return 0;
#else
	Gui_ClearScr();
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("NOT IMPLEMENT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	return ERR_TRAN_FAIL;
#endif
}

void TransformIP(const uchar * ip_in, uchar * ip_out)
{
	sprintf(ip_out, "%u.%u.%u.%u", ip_in[0],ip_in[1],ip_in[2],ip_in[3]);
}

void TransformPort(const uchar * port_in, uchar * port_out)
{
	int iPortNum;
	iPortNum = port_in[0]*256+port_in[1];
	sprintf(port_out, "%d", iPortNum );
}



// save card range parameters
// Modified by Kim_LinHB 2014-6-8
int UnpackParaCard(uchar ucIndex, const uchar *psPara)
{
	CARD_TABLE	*pstCardTbl;

	if( glSysParam.ucCardNum >= MAX_CARD )
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("MAX # OF CARD"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return ERR_NO_DISP;
	}

	pstCardTbl = &glSysParam.stCardTable[glSysParam.ucCardNum];
	memcpy(pstCardTbl, psPara, sizeof(CARD_TABLE));
	pstCardTbl->ucPanLength = (uchar)PubBcd2Long(&pstCardTbl->ucPanLength, 1);
	glSysParam.ucCardNum++;

	return 0;
}

// save instalment parameters
int UnpackInstPara(const uchar *psPara)
{
	uchar					ucNum;
	TMS_INSTALMENT_PLAN		*pstPlan;

	if( glSysParam.ucPlanNum >= MAX_PLAN )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("MAX # OF PLAN"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return ERR_NO_DISP;
	}

	pstPlan = (TMS_INSTALMENT_PLAN *)psPara;
	ucNum   = glSysParam.ucPlanNum;
	glSysParam.stPlanList[ucNum].ucIndex    = pstPlan->ucIndex;
	glSysParam.stPlanList[ucNum].ucAcqIndex = pstPlan->ucAcqIndex;
	sprintf((char *)glSysParam.stPlanList[ucNum].szName, "%.7s", pstPlan->sName);
	glSysParam.stPlanList[ucNum].ucMonths    = (uchar)PubBcd2Long(&pstPlan->ucMonths, 1);
	glSysParam.stPlanList[ucNum].ulBottomAmt = PubBcd2Long(pstPlan->sBottomAmt, 6);
	glSysParam.ucPlanNum++;

	return 0;
}

// save product descriptors
int UnpackDescPara(const uchar *psPara)
{
	uchar				ucNum;
	TMS_DESCRIPTOR		*pstDesc;

	if( glSysParam.ucDescNum >= MAX_DESCRIPTOR )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("MAX # OF DESC"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return ERR_NO_DISP;
	}

	pstDesc = (TMS_DESCRIPTOR *)(psPara+1);
	ucNum   = glSysParam.ucDescNum;
	glSysParam.stDescList[ucNum].ucKey = pstDesc->ucKey;
	sprintf((char *)glSysParam.stDescList[ucNum].szCode, "%.2s", pstDesc->sCode);
	sprintf((char *)glSysParam.stDescList[ucNum].szText, "%.20s", pstDesc->sText);
	glSysParam.ucDescNum++;

	return 0;
}

#ifdef ENABLE_EMV
// save EMV parameters
int SaveEmvMisc(const uchar *psPara)
{
	TMS_EMV_MISC	*pstEmvMisc;

	pstEmvMisc = (TMS_EMV_MISC *)psPara;
	EMVGetParameter(&glEmvParam);

	memcpy(glEmvParam.CountryCode,   pstEmvMisc->sCourtryCode,  2);
	memcpy(glEmvParam.TransCurrCode, pstEmvMisc->sCurcyCode,    2);
	memcpy(glEmvParam.ReferCurrCode, pstEmvMisc->sRefCurcyCode, 2);
	glEmvParam.TransCurrExp = pstEmvMisc->ucCurcyExp;
	glEmvParam.ReferCurrExp = pstEmvMisc->ucRefCurcyExp;
//	pstEmvMisc->Language;	// Unused

	EMVSetParameter(&glEmvParam);
	return 0;
}
#endif

#ifdef ENABLE_EMV
// save EMV application
int SaveEmvApp(const uchar *psPara)
{
	int				iRet;
	TMS_EMV_APP		*pstApp;
	EMV_APPLIST		stEmvApp;

	pstApp = (TMS_EMV_APP *)(psPara+1);
	memset(&stEmvApp, 0, sizeof(EMV_APPLIST));

	if( pstApp->bLocalName )
	{
		memcpy(stEmvApp.AppName, pstApp->sLocalName, (int)MIN(16, pstApp->ucLocalNameLen));
	}

	memcpy(stEmvApp.AID, pstApp->sAID, (int)MIN(16, pstApp->ucAIDLen));
	stEmvApp.AidLen   = pstApp->ucAIDLen;
	stEmvApp.SelFlag  = (pstApp->ucASI==0) ? PART_MATCH : FULL_MATCH;
//	stEmvApp.Priority = returned by card, not used here;

	stEmvApp.TargetPer       = pstApp->ucTargetPer;
	stEmvApp.MaxTargetPer    = pstApp->ucMaxTargetPer;
	if( sgNewTMS )
	{
		stEmvApp.FloorLimitCheck = ((pstApp->ucTermRisk & TRM_FLOOR_CHECK)!=0);
		stEmvApp.RandTransSel    = ((pstApp->ucTermRisk & TRM_RANDOM_TRAN_SEL)!=0);
		stEmvApp.VelocityCheck   = ((pstApp->ucTermRisk & TRM_VELOCITY_CHECK)!=0);
	}
	else
	{	// 旧版协议不下载这些数据
		// those data are undefined in the old version of protocol
		stEmvApp.FloorLimitCheck = 1;
		stEmvApp.RandTransSel    = 1;
		stEmvApp.VelocityCheck   = 1;
	}

	stEmvApp.FloorLimit = PubChar2Long(pstApp->sFloorLimit, 4);
	stEmvApp.FloorLimit *= 100;		// floor, unit is YUAN
	stEmvApp.Threshold = PubChar2Long(pstApp->sThreshold,  4);

	memcpy(stEmvApp.TACDenial,  pstApp->sTACDenial,  5);
	memcpy(stEmvApp.TACOnline,  pstApp->sTACOnline,  5);
	memcpy(stEmvApp.TACDefault, pstApp->sTACDefault, 5);
//	memcpy(stEmvApp.AcquierId, // not set here

	stEmvApp.dDOL[0] = pstApp->ucTermDDOLLen;
	memcpy(&stEmvApp.dDOL[1], pstApp->sTermDDOL, stEmvApp.dDOL[0]);
	if( sgNewTMS )
	{
		stEmvApp.tDOL[0] = strlen((char *)pstApp->sTDOL)/2;
		PubAsc2Bcd(pstApp->sTDOL, (uint)stEmvApp.tDOL[0], &stEmvApp.tDOL[1]);
	}
	memcpy(stEmvApp.Version, pstApp->sAppVer, 2);
//	stEmvApp.RiskManData

	iRet = EMVAddApp(&stEmvApp);
	if( iRet!=EMV_OK )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("ERR SAVE EMV APP"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return iRet;
	}

	return 0;
}
#endif

#ifdef ENABLE_EMV
// save CAPK
int SaveEmvCapk(const uchar *psPara)
{
	int				iRet;
	TMS_EMV_CAPK	*pstCapk;
	EMV_CAPK		stEmvCapk;

	pstCapk = (TMS_EMV_CAPK *)(psPara+1);
	memset(&stEmvCapk, 0, sizeof(EMV_CAPK));

	memcpy(stEmvCapk.RID, pstCapk->sRID, 5);
	stEmvCapk.KeyID    = pstCapk->ucKeyID;
	stEmvCapk.HashInd  = pstCapk->ucHashInd;
	stEmvCapk.ArithInd = pstCapk->ucArithInd;
	stEmvCapk.ModulLen = pstCapk->ucModulLen;
	memcpy(stEmvCapk.Modul, pstCapk->sModul, stEmvCapk.ModulLen);
	stEmvCapk.ExponentLen = pstCapk->ucExpLen;
	memcpy(stEmvCapk.Exponent, pstCapk->sExponent, stEmvCapk.ExponentLen);
// PubTRACE3("%02X %02X %02X", pstCapk->sExpiry[0], pstCapk->sExpiry[1], pstCapk->sExpiry[2]);
	if( memcmp(pstCapk->sExpiry, "\x00\x00\x00", 3)!=0 )
	{
		memcpy(stEmvCapk.ExpDate, pstCapk->sExpiry, 3);
	}
	else
	{
		memcpy(stEmvCapk.ExpDate, "\x20\x12\x31", 3); // unknown expiry
	}
	memcpy(stEmvCapk.CheckSum, pstCapk->sCheckSum, 20);

	iRet = EMVAddCAPK(&stEmvCapk);
	if( iRet!=EMV_OK )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("ERR SAVE CAPK"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
//		return iRet;
	}

	return 0;
}
#endif

void SearchIssuerKeyArraySub(uchar *sIssuerKey, uchar ucAcqKey)
{
	uchar	ucCnt, sTempKey[256];
	int		iTemp, iKeyNum;

	memset(sTempKey, (uchar)INV_ISSUER_KEY, sizeof(sTempKey));
	for(ucCnt=0; ucCnt<glSysParam.ucCardNum; ucCnt++)
	{
		if( glSysParam.stCardTable[ucCnt].ucAcqKey==ucAcqKey &&
			glSysParam.stCardTable[ucCnt].ucAcqKey!=INV_ACQ_KEY )
		{	// 消除重复,因为可能一个issuer对多个card
			// for ignoring repeats, because several cards may map to the same issuer 
			sTempKey[glSysParam.stCardTable[ucCnt].ucIssuerKey] = glSysParam.stCardTable[ucCnt].ucIssuerKey;
		}
	}

	for(iKeyNum=iTemp=0; iTemp<256; iTemp++)
	{
		if( sTempKey[iTemp]!=INV_ISSUER_KEY )
		{
			sIssuerKey[iKeyNum] = sTempKey[iTemp];
			iKeyNum++;
		}
	}
}

// get TMS telepnone number
// Modified by Kim_LinHB 2014-6-8
int GetDownLoadTelNo(void)
{
	int iRet;
	uchar	szBuff[30];
	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 25;

	sprintf((char *)szBuff, "%.25s", glSysParam.stEdcInfo.szDownTelNo);
	
	Gui_ClearScr();
	iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("PHONE NO"), gl_stLeftAttr, 
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	if( iRet!=GUI_OK )
	{
		return ERR_USERCANCEL;
	}

	sprintf((char *)glSysParam.stEdcInfo.szDownTelNo, "%.25s", szBuff);
	SaveEdcParam();

	return 0;
}

int GetDownLoadGprsPara(void)
{
	uchar	ucRet = SetWirelessParam(&glSysParam._TmsWirlessPara);
	SyncWirelessParam(&glSysParam._TxnWirlessPara, &glSysParam._TmsWirlessPara);

	return ucRet;
}

// Modified by Kim_LinHB 2014-6-8
int GetDownLoadLanPara(void)
{
	uchar	ucRet;

	SetCurrTitle("SETUP TCPIP");
	Gui_ClearScr();
	
	ucRet = SetTcpIpParam(&glSysParam._TmsTcpIpPara);
	if (ucRet==0)
	{
		SyncTcpIpParam(&glSysParam._TxnTcpIpPara, &glSysParam._TmsTcpIpPara);
		return 0;
	}

	return ucRet;
}

int GetDownLoadWIFIPara(void)
{
	int ret;

	SetCurrTitle("SETUP WIFI");

	ret = SetWiFiApp(&glSysParam._TmsWifiPara);
	if(ret != 0)
	{
		Beep();	
		DispWifiErrorMsg(ret);
		return 0;
	}
	SyncWifiParam(&glSysParam._TxnWifiPara, &glSysParam._TmsWifiPara);
	return 0;
}

int GetDownLoadComm(uchar ucCommType)
{
	int		iRet;

	switch(ucCommType)
	{
	case CT_RS232:
        iRet = SetRs232Param(&glSysParam._TmsRS232Para);
		break;
	case CT_MODEM:
		iRet = GetDownLoadTelNo();
		break;
	case CT_TCPIP:
		iRet = GetDownLoadLanPara();
		if (iRet==0)
		{
			iRet = GetRemoteIp("PROTIMS ", FALSE, FALSE, &glSysParam.stEdcInfo.stDownIpAddr);
		}
		break;
	case CT_WCDMA:
	case CT_CDMA:
	case CT_GPRS:
		iRet = GetDownLoadGprsPara();
		if (iRet==0)
		{
			iRet = GetRemoteIp("PROTIMS ", FALSE, FALSE, &glSysParam.stEdcInfo.stDownIpAddr);
		}
		break;
	case CT_WIFI:
		iRet=GetDownLoadWIFIPara();
		if (iRet==0)
		{
			iRet = GetRemoteIp("PROTIMS ", FALSE, FALSE, &glSysParam.stEdcInfo.stDownIpAddr);
		}
 		break;
	case CT_BLTH:
	case CT_USB:
		iRet = 0;
		break;
	default:
		iRet = ERR_NO_DISP;
		break;
	}

	return iRet;
}

// get TMS download terminal ID
// Modified by Kim_LinHB 2014-6-8
int GetDownLoadTID(uchar *pszID)
{
	uchar	szBuff[8+1];
	int		iRet;
	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 8;
	stInputAttr.nMaxLen = 8;

	while (1)
	{
		sprintf((char *)szBuff, "%.8s", pszID);
		Gui_ClearScr();
		iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("DOWNLOAD ID"), gl_stLeftAttr, 
			szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
		if( iRet!=GUI_OK )
		{
			return ERR_USERCANCEL;
		}
		if (atol((char *)szBuff)!=0)
		{
			break;
		}
	}

	sprintf((char *)pszID, "%.8s", szBuff);
	return 0;
}

//void UpdateCommType(void)
// 产生下一个自动下载时间
// To figure when will be the next time to download auto
void GetNextAutoDayTime(uchar *pszDateTimeInOut, ushort uiInterval)
{
	uchar	szBuff[50];
	int		iYear, iMonth, iDate, iHour, iMinute, iMaxDay;

	GetDateTime(szBuff);
	iYear  = (int)PubAsc2Long(&szBuff[2], 2);
	iMonth = (int)PubAsc2Long(&szBuff[4], 2);
	iDate  = (int)PubAsc2Long(&szBuff[6], 2);
	srand((uint)PubAsc2Long(&szBuff[10], 4));
	iHour   = rand()%6;		// hour generated must be in range 0 - 5
	iMinute = rand()%60;	// minute generated must be in range 0 - 59
	if( uiInterval>999 )
	{
		uiInterval = 999;
	}

	iDate += uiInterval;
	while( 1 )
	{
		switch( iMonth )
		{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			iMaxDay = 31;
			break;

		case 2:
			iMaxDay = 29;
			break;

		default:
			iMaxDay = 30;
			break;
		}

		if( iDate<=iMaxDay )
		{
			break;
		}
		iDate -= iMaxDay;
		iMonth++;

		if( iMonth>12 )
		{
			iMonth -= 12;
			iYear++;
		}
	}	// while( 1

	sprintf((char *)pszDateTimeInOut, "%02d%02d%02d%02d%02d", iYear%100, iMonth,
			iDate, iHour, iMinute);
}

void GetOneLine(uchar **psCurPtr, uchar *psData, int Maxlen)
{
#define ISSPACE(ch) ( ((ch)==' ')  || ((ch)=='\t') || \
					  ((ch)=='\n') || ((ch)=='\r') )
#define ISLINEEND(ch) ( ((ch)=='\n') || ((ch)=='\r') )
	uchar	*p, *q;

	for(p=*psCurPtr; *p && ISSPACE(*p); p++);

	*psData = 0;
	for(q=psData; *p && (q-psData<Maxlen) && !ISLINEEND(*p); )	*q++ = *p++;
	*q = 0;
	PubTrimStr(psData);

	for(; *p && !ISLINEEND(*p); p++);  // erase characters of the left lines
	*psCurPtr = p;
#undef ISSPACE
#undef ISLINEEND
}

int SaveDetail(const uchar *psData)
{
	uchar	*psCurPtr, *psBack, ucLen, ucCmpLen;
	uchar	szBuf[80], szStartNo[20+1], szEndNo[20+1];
	ushort	uiIndex;

	// exceed the size of card table
	if( glSysParam.uiCardBinNum >= MAX_CARDBIN_NUM )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("CARDBIN OVERFLOW"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return 1;
	}

	psBack   = (uchar *)psData;
	psCurPtr = (uchar *)strchr((char *)psBack, ',');
	if( psCurPtr==NULL )
	{
		return 1;
	}
	sprintf((char *)szBuf, "%.*s", (int)MIN(psCurPtr-psBack, 9), psBack);
	ucLen = (uchar)atol((char *)szBuf);

	psBack   = psCurPtr+1;
	psCurPtr = (uchar *)strchr((char *)psBack, ',');
	if( psCurPtr==NULL )
	{
		return 1;
	}
	sprintf((char *)szStartNo, "%.*s", (int)MIN(psCurPtr-psBack, 19), psBack);
	sprintf((char *)szEndNo,   "%.*s", (int)MIN(psCurPtr-psBack, 19), psCurPtr+1);
	if( strlen((char *)szStartNo)!=strlen((char *)szEndNo) )
	{
		return 1;
	}

	// save card bin record
	uiIndex = glSysParam.uiCardBinNum;
	glSysParam.stCardBinTable[uiIndex].ucPanLen      = ucLen;
	ucCmpLen = (uchar)strlen((char *)szStartNo);
	glSysParam.stCardBinTable[uiIndex].ucMatchLen    = ucCmpLen;
	glSysParam.stCardBinTable[uiIndex].ucIssuerIndex = (uchar)glSysParam.uiIssuerNameNum;

	memset(glSysParam.stCardBinTable[uiIndex].sStartNo, 0, sizeof(glSysParam.stCardBinTable[uiIndex].sStartNo));
	memset(glSysParam.stCardBinTable[uiIndex].sEndNo, (uchar)0xFF, sizeof(glSysParam.stCardBinTable[uiIndex].sEndNo));
	if( ucCmpLen % 2 )	// card length is odd, pad with a specific character
	{
		szStartNo[ucCmpLen] = '0';
		PubAsc2Bcd(szStartNo, (uint)(ucCmpLen+1), glSysParam.stCardBinTable[uiIndex].sStartNo);
		szEndNo[ucCmpLen]   = 'F';
		PubAsc2Bcd(szEndNo,   (uint)(ucCmpLen+1), glSysParam.stCardBinTable[uiIndex].sEndNo);
	}
	else
	{
		PubAsc2Bcd(szStartNo, (uint)ucCmpLen, glSysParam.stCardBinTable[uiIndex].sStartNo);
		PubAsc2Bcd(szEndNo,   (uint)ucCmpLen, glSysParam.stCardBinTable[uiIndex].sEndNo);
	}

	glSysParam.uiCardBinNum++;

	return 0;
}

// save card bin table for HK
int SaveCardBin(uchar *psCardBinInOut)
{
	uchar	*psCurPtr, *psBack, ucFlag;
	uchar	szBuf[80], szChineseName[16+1], szEnglishName[MAX_CARBIN_NAME_LEN+1];
	ushort	uiIndex;

	psCurPtr = psCardBinInOut;
	while( *psCurPtr )
	{
		if( glSysParam.uiIssuerNameNum >= MAX_CARDBIN_ISSUER )
		{
			return 1;
		}

		// search Tag [Issuer]
		GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
		if( PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")!=0 )
		{
			continue;
		}

		// get Chinese name
		psBack = psCurPtr;
		GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
		if(PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")==0 )
		{
			psCurPtr = psBack;
			continue;
		}
		sprintf((char *)szChineseName, "%.16s", szBuf);

		// get English name
		psBack = psCurPtr;
		GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
		if( PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")==0 )
		{
			psCurPtr = psBack;
			continue;
		}
		sprintf((char *)szEnglishName, "%.*s", MAX_CARBIN_NAME_LEN, szBuf);

		// get details
		ucFlag = 0;
		while( *psCurPtr )
		{
			psBack = psCurPtr;
			GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
			if( (szBuf[0]==0) || (PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")==0) )
			{
				psCurPtr = psBack;
				break;
			}
			if( SaveDetail(szBuf)!=0 )
			{
				return 1;
			}
			ucFlag = 1;
		}
		if( !ucFlag )	// ignore null detail lines!
		{
			continue;
		}

		// save Chinese/English name of issuer
		uiIndex = glSysParam.uiIssuerNameNum;
		sprintf((char *)glSysParam.stIssuerNameList[uiIndex].szChineseName, "%.16s", szChineseName);
		sprintf((char *)glSysParam.stIssuerNameList[uiIndex].szEnglishName, "%.*s", MAX_CARBIN_NAME_LEN, szEnglishName);
		glSysParam.uiIssuerNameNum++;
	}

	return 0;
}

void LoadEdcLang(void)
{
	SetSysLang(1);
#ifdef AREA_Arabia
    CustomizeAppLibForArabiaLang( strcmp(LANGCONFIG, "Arabia")==0 );
#endif
}

void LoadDefaultLang(void)
{
	SetCurrTitle(_T("SELECT LANG")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	SetSysLang(0);
	SetCurrTitle("");
#ifdef AREA_Arabia
    CustomizeAppLibForArabiaLang( strcmp(LANGCONFIG, "Arabia")==0 );
#endif
}


#ifdef _WIN32
int GetLoadedAppStatus(uchar *psAppName, TMS_LOADSTATUS *ptStat)
{
	// Modified by Kim_LinHB 2014-6-8
	Gui_ClearScr();
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("NOT IMPLEMENT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	return -1;
}
#endif

// processing of loading default parameters without downloading
void InitEdcParam(void)
{
	logTrace(__func__);
	CURRENCY_CONFIG		stLocalCurrency;
	int iIAcqCnt = 0;
	int iIssuerCnt = 0;
	int iCardTableCnt = 0;

	GetDefCurrency(&stLocalCurrency);
	sprintf((char *)glSysParam.stEdcInfo.szMerchantName, "MERCHANT NAME");
	sprintf((char *)glSysParam.stEdcInfo.szMerchantAddr, "MERCHANT ADDR");
	sprintf((char *)&glSysParam.stEdcInfo.szDownLoadTID, "00000000");
	memcpy(glSysParam.stEdcInfo.sOption, "\xE6\x28\x00\x09\x00", 5);
	glSysParam.stEdcInfo.ucTranAmtLen     = 10;
	glSysParam.stEdcInfo.ucStlAmtLen      = 12;

	// 这里先设默认。之后会有一个手动修改的机会
	// set with default parameters, enable to modify later
	glPosParams.currency  = stLocalCurrency;
	glSysParam.stEdcInfo.ucCurrencySymbol = ' ';

	if (ChkHardware(HWCFG_PRINTER, 'S')==TRUE)
	{
		glSysParam.stEdcInfo.ucPrinterType = 0;
	}
	else
	{
		glSysParam.stEdcInfo.ucPrinterType = 1;
	}

	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// set card range
	// VISA
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x40\x00\x00\x00\x00", 5);
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x49\x99\x99\x99\x99", 5);
	glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x01;
	glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
	glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
	glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
	++iCardTableCnt;

	//MASTER
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x50\x00\x00\x00\x00", 5);
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x59\x99\x99\x99\x99", 5);
	glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x02;
	glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
	glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
	glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
	++iCardTableCnt;

	//AE1 added by Kevinliu 20160505
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x34\x00\x00\x00\x00", 5);
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x34\x99\x99\x99\x99", 5);
	//AE2 added by Kevinliu 20160505
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[1].sPanRangeLow,  "\x37\x00\x00\x00\x00", 5);
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[1].sPanRangeHigh, "\x37\x99\x99\x99\x99", 5);
    glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x03;
    glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
    glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
    glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
    ++iCardTableCnt;

	//Diners added by Kevinliu 20160505
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x36\x00\x00\x00\x00", 5);
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x36\x99\x99\x99\x99", 5);
    glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x04;
    glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
    glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
    glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
    ++iCardTableCnt;

	//JCB
    memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x35\x00\x00\x00\x00", 5);
//    memcpy(glSysParam.stCardTable[iCardTableCnt].sPanRangeHigh, "\x39\x99\x99\x99\x99", 5);	//modified by Kevinliu 20160505
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x35\x99\x99\x99\x99", 5);
    glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x05;
    glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
    glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
    glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
    ++iCardTableCnt;

	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeLow,  "\x60\x00\x00\x00\x00", 5);
	memcpy(glSysParam.stCardTable[iCardTableCnt].stCardRange[0].sPanRangeHigh, "\x99\x99\x99\x99\x99", 5);
	glSysParam.stCardTable[iCardTableCnt].ucIssuerKey = 0x06;
	glSysParam.stCardTable[iCardTableCnt].ucAcqKey    = 0x01;
	glSysParam.stCardTable[iCardTableCnt].ucPanLength = 0;
	glSysParam.stCardTable[iCardTableCnt].ucOption    = 0x02;
	++iCardTableCnt;

	glSysParam.ucCardNum = iCardTableCnt;
}


// end of file

