
#include "global.h"

// Modified by Kim_LinHB 2014-7-9  Update GUI to new version

/********************** Internal macros declaration ************************/

/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/
static int SetSystemParam(void);
static void SetSystemParamSub(uchar ucPermission);
static int  SetCommType(uchar ucMode);
void SetSysCommParam(uchar ucPermission);
static int  SetCommDetails(uchar mode, uchar *pucCommType);
static int  SetPabx(void);
static int  SetModemParam(void);
static int  SetTcpIpSharedPara(COMM_CONFIG *pstCommCfg);

static int  SetTcpIpParam_S80(TCPIP_PARA *pstParam);

static int  GetHostDNS(const uchar *pszPrompts, uchar bAllowNull, uchar *pszName);
int  GetIPAddress(const uchar *pszPrompts, uchar bAllowNull, uchar *pszIPAddress);
static uchar IsValidIPAddress(const char *pszIPAddr);
int  GetIPPort(const uchar *pszPrompts, uchar bAllowNull, uchar *pszPortNo);
static int SetTel(uchar *pszTelNo, const uchar *pszPromptInfo);
static void SetEdcParam(uchar ucPermission);
static int SetTermCountry(uchar ucPermission);
static int  SetTermCurrency(uchar ucPermission);
static int SetTermDecimalPosition(uchar ucPermission);
static int SetTermIgnoreDigit(uchar ucPermission);
static int  SetMerchantName(uchar ucPermission);
static int SetMerchantAddr(uchar ucPermission);
static int  SetGetSysTraceNo(uchar ucPermission);
static int  SetGetSysInvoiceNo(uchar ucPermission);
static int  SetPEDMode(void);
static int  SetAcceptTimeOut(void);
static int  SetPrinterType(void);
static int  SetNumOfReceipt(void);
static int  SetCallInTime(void);
static uchar IsValidTime(const uchar *pszTime);
static int  ModifyOptList(uchar *psOption, uchar ucMode, uchar ucPermission);
static int ChangePassword(void);
static int SetSysTime(void);
static int SetEdcLang(void);
static int SetPowerSave(void);
static int TestMagicCard1(void);
static int TestMagicCard2(void);
static int TestMagicCard3(void);
static void TestMagicCard(int iTrackNum);
static int ToolsViewPreTransMsg(void);
static int ShowExchangePack(void);
static int PrnExchangePack(void);
static void DebugNacTxd(uchar ucPortNo, const uchar *psTxdData, ushort uiDataLen);

/********************** Internal variables declaration *********************/

static const GUI_MENUITEM sgFuncMenuItem[] =
{
	{ _T_NOOP("INIT"),	0,FALSE,  NULL},
	{ _T_NOOP("VIEW RECORD"),	1,TRUE,  ViewTranList},
	{ _T_NOOP("SETUP"),	2,FALSE,  SetSystemParam},
	{ _T_NOOP("LANGUAGE"),	3,TRUE,  SetEdcLang},
	{ _T_NOOP("LOCK TERM"),	4,TRUE,  LockTerm},
	{ _T_NOOP("VIEW TOTAL"),	5,TRUE,  ViewTotal},
#ifdef ENABLE_EMV
	{ _T_NOOP("LAST TSI TVR"),	9,FALSE,  ViewTVR_TSI},
#endif
	{ _T_NOOP("SET TIME"),	10,TRUE,  SetSysTime},
	{ _T_NOOP("PRINT PARA"),	11,FALSE,  PrintParam},
	{ _T_NOOP("TXN REVIEW"),	21,TRUE,  ViewSpecList},
	{ _T_NOOP("APP UPDATE"),	50,FALSE,  NULL},	// hidden, not for public use until confirm.
	{ _T_NOOP("CHECK FONTS"),	60,FALSE,  EnumSysFonts},
	{ _T_NOOP("REPRN SETTLE"),	71,TRUE,  RePrnSettle},
	{ _T_NOOP("REPRINT LAST"),	72,TRUE,  PrnLastTrans},
	{ _T_NOOP("REPRINT BILL"),	73,TRUE,  RePrnSpecTrans},
	{ _T_NOOP("PRINT TOTAL"),	74,TRUE,  PrnTotal},
	{ _T_NOOP("PRINT LOG"),	75,TRUE,  PrnAllList},
	{ _T_NOOP("POWER SAVE"),	81,TRUE,  SetPowerSave},
//	{ _T_NOOP("TEST TRACK1"),	87,FALSE,  TestMagicCard1},
//	{ _T_NOOP("TEST TRACK2"),	88,FALSE,  TestMagicCard2},
//	{ _T_NOOP("TEST TRACK3"),	89,FALSE,  TestMagicCard3},
	{ _T_NOOP("MODIFY PWD"),	90,TRUE,  ChangePassword},
	{ _T_NOOP("DISP VER"),	91,TRUE,  NULL},
	{ _T_NOOP("SHOW PACKAGE"),	95,FALSE,  ToolsViewPreTransMsg},
#ifdef ENABLE_EMV
	{ _T_NOOP("PRINT ERR LOG"),	96,FALSE,  PrintEmvErrLog},
#endif
	{ _T_NOOP("CLEAR"),	99,FALSE,  DoClear},
	{ "", -1,FALSE,  NULL},
};

static const GUI_MENUITEM sgInitMenuItem[] =
{
	{ _T_NOOP("INIT"),	0,FALSE,  NULL},
//	{ _T_NOOP("SETUP"),	2,FALSE,  SetSystemParam},
	{ _T_NOOP("LANGUAGE"),	3,TRUE,  SetEdcLang},
	{ _T_NOOP("SET TIME"),	10,TRUE,  SetSysTime},
	{ _T_NOOP("CHECK FONTS"),	60,FALSE,  EnumSysFonts},
	{ _T_NOOP("TEST TRACK1"),	87,FALSE,  TestMagicCard1},
	{ _T_NOOP("TEST TRACK2"),	88,FALSE,  TestMagicCard2},
	{ _T_NOOP("TEST TRACK3"),	89,FALSE,  TestMagicCard3},
	{ _T_NOOP("MODIFY PWD"),	90,TRUE,  ChangePassword},
	{ _T_NOOP("DISP VER"),	91,TRUE,  NULL},
	{ _T_NOOP("CLEAR"),	99,FALSE,  DoClear},
	{ "", -1,FALSE,  NULL},
};

/********************** external reference declaration *********************/



/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
void GetAllSupportFunc(char *pszBuff)
{
	int	ii;

	pszBuff[0] = 0;
	for (ii=0; ii<sizeof(sgFuncMenuItem)/sizeof(sgFuncMenuItem[0]); ii++)
	{
		if (sgFuncMenuItem[ii].szText[0]!=0)
		{
			if (strlen(pszBuff)!=0)
			{
				strcat(pszBuff, ",");
			}
			sprintf(pszBuff+strlen(pszBuff), "%lu", (unsigned long)sgFuncMenuItem[ii].nValue);
		}
	}
}

// 执行指定功能号的函数
// call function with a specific id
void FunctionExe(uchar bUseInitMenu, int iFuncNo)
{
	int			iCnt;
	GUI_MENUITEM	*pstMenu;

	pstMenu = (GUI_MENUITEM *)(bUseInitMenu ? sgInitMenuItem : sgFuncMenuItem);
	for(iCnt=0; pstMenu[iCnt].szText[0]!=0; iCnt++)
	{
	
		if( pstMenu[iCnt].nValue == iFuncNo)
		{
			if( !pstMenu[iCnt].vFunc )
			{
				break;
			}
			pstMenu[iCnt].vFunc();
			return;
		}
	}

	Gui_ClearScr();
	PubBeepErr();
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("FUNC NUMBER ERR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL); // Modified by Kim_LinHB 2014-8-6 v1.01.0001 bug493
}

void FunctionMenu(void)
{
	logTrace(__func__);
	GUI_MENU stMenu;
	Gui_BindMenu(_T("FUNCTION:"), gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)sgFuncMenuItem, &stMenu);
	Gui_ClearScr();
	Gui_ShowMenuList(&stMenu, GUI_MENU_MANUAL_INDEX, USER_OPER_TIMEOUT, NULL);
}

void FunctionInit(void)
{
	int		iFuncNo;
	iFuncNo = FunctionInput();
	if( iFuncNo>=0 )
	{
		FunctionExe(TRUE, iFuncNo);
	}
}

// 设置系统参数
// set system parameters
int SetSystemParam(void)
{
	uchar ucPermission;

#ifdef FUN2_READ_ONLY
	ucPermission = PM_LOW;		// 低权限
#else
	ucPermission = PM_MEDIUM;	// 中等权限
#endif

	SetCurrTitle(_T("TERM SETUP"));
	if( PasswordBank()!=0 )
	{
		return ERR_NO_DISP;
	}

	SetSystemParamSub(ucPermission);
	return 0;
}
		
void SetSystemParamSub(uchar ucPermission)
{
	int iSelected;
	GUI_MENU stMenu;
	GUI_MENUITEM stMenuItem[] = {
		{ _T_NOOP("COMM PARA"), 1,TRUE,  NULL},
		{ _T_NOOP("VIEW EDC"), 2,TRUE,  NULL},
		{ _T_NOOP("VIEW ISSUER"), 3,TRUE,  NULL},
		{ _T_NOOP("VIEW ACQUIRER"), 4,TRUE,  NULL},
		{ "", -1,FALSE,  NULL},
	};

	Gui_BindMenu(NULL, gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stMenuItem, &stMenu);
	iSelected = 0;
	while( 1 )
	{
		Gui_ClearScr();
		
		if( GUI_OK != Gui_ShowMenuList(&stMenu, 0, USER_OPER_TIMEOUT, &iSelected))
		{
			break;
		}
		
		if( 1 == iSelected )
		{
			SetSysCommParam(ucPermission);
		}
		else if( 2 == iSelected )
		{
			SetEdcParam(ucPermission);
		}
		else if( 3 == iSelected )
		{

		}
		else if( 4 == iSelected )
		{

		}
	}
	Gui_ClearScr();
}

void SetSystemParamAll(void)
{
	// 最高权限，可以修改所有参数
	// using the highest Permission
	SetSystemParamSub(PM_HIGH);
}

int GetCommName(uchar ucCommType, uchar *pszText)
{
	switch(ucCommType)
	{
	case CT_RS232:
		sprintf((char *)pszText, "COM");
		return 0;
	case CT_MODEM:
		sprintf((char *)pszText, "MODEM");
	    return 0;
	case CT_TCPIP:
		sprintf((char *)pszText, "TCPIP");
	    return 0;
	case CT_WCDMA:
		sprintf((char *)pszText, "WCDMA");
		return 0;
	case CT_CDMA:
		sprintf((char *)pszText, "CDMA");
		return 0;
	case CT_GPRS:
		sprintf((char *)pszText, "GPRS");
		return 0;
	case CT_WIFI:
		sprintf((char *)pszText, "WIFI");
	    return 0;
	case CT_DEMO:
		sprintf((char *)pszText, "DEMO");
	    return 0;	
	case CT_BLTH:
		sprintf((char *)pszText, "BLUETOOTH");
	    return 0;
	default:
		sprintf((char *)pszText, "DISABLED");
	    return -1;
	}
}

// ucForAcq : set custom comm type for ACQ
int SetCommType(uchar ucMode)
{
	int		iRet, iSelected;
	char	szTitle[32];
	uchar	*pucCommType;
	GUI_MENU	stSmDownMode;
	// Modified by Kim_LinHB 2014-8-6 v1.01.0001 bug492  remove static
	GUI_MENUITEM stDefCommMenu[] =
	{
		{ "DISABLE",	CT_NONE,TRUE, 	NULL}, //0
		{ "MODEM",	CT_MODEM,TRUE, 	NULL}, //1
		{ "TCPIP",	CT_TCPIP,TRUE, 	NULL}, //2
		{ "GPRS",	CT_GPRS,TRUE, 	NULL}, //3
		{ "CDMA",	CT_CDMA,TRUE, 	NULL}, //4
		{ "WIFI",	CT_WIFI, TRUE, 	NULL}, //5
		{ "RS232",	CT_RS232,TRUE, 	NULL}, //6
		{ "BLUETOOTH",	CT_BLTH,FALSE, 	NULL}, //7
		{ "WCDMA",	CT_WCDMA,TRUE, 	NULL}, //8
		{ "DEMO ONLY",	CT_DEMO,TRUE, 	NULL}, //9
		{ "", -1,FALSE,  NULL},
	};// This menu does not provide translation
	GUI_MENUITEM stCommMenu[20];
	int iMenuItemNum = 0;

	//--------------------------------------------------
	memset(&stSmDownMode, 0, sizeof(stSmDownMode));

	glSysParam.stTxnCommCfg = glPosParams.commConfig;
	
	if (ucMode!=0)
	{
	    memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[0], sizeof(GUI_MENUITEM));
	    sprintf(stCommMenu[iMenuItemNum].szText, "%s", stDefCommMenu[0].szText);
	    ++iMenuItemNum;
	}
	if (!(ChkHardware(HWCFG_MODEM, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_MODEM)))
	{
	    if(stDefCommMenu[1].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[1], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%s", stDefCommMenu[1].szText);
            ++iMenuItemNum;
        }
	}
	if (!(ChkHardware(HWCFG_LAN, HW_NONE) ||									// If no LAN module
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_TCPIP)))	// and now is selecting 2nd comm && 1st comm already selected LAN
	{
	    if(stDefCommMenu[2].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[2], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%s", stDefCommMenu[2].szText);
            ++iMenuItemNum;
        }
	}
	
	if (!(ChkHardware(HWCFG_GPRS, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_GPRS)))
	{
	    if(stDefCommMenu[3].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[3], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%s", stDefCommMenu[3].szText);
            ++iMenuItemNum;
        }
	}
	if (!(ChkHardware(HWCFG_CDMA, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_CDMA)))
	{
	    if(stDefCommMenu[4].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[4], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%s", stDefCommMenu[4].szText);
            ++iMenuItemNum;
        }
	}
	if (!(ChkHardware(HWCFG_WIFI, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_WIFI)))
	{
	    if(stDefCommMenu[5].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[5], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%s", stDefCommMenu[5].szText);
            ++iMenuItemNum;
        }
	}
	if (!(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_RS232))
	{
	    if(stDefCommMenu[6].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[6], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%s", stDefCommMenu[6].szText);
            ++iMenuItemNum;
        }
	}
	if(!(ChkHardware(HWCFG_BLTH, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_BLTH)))
	{
	    if(stDefCommMenu[7].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[7], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%s", stDefCommMenu[7].szText);
            ++iMenuItemNum;
        }
	}
	if (!(ChkHardware(HWCFG_WCDMA, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_WCDMA)))
	{
	    if(stDefCommMenu[8].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[8], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%s", stDefCommMenu[8].szText);
            ++iMenuItemNum;
        }
	}
    if (ucMode==0)
    {
        // Only primary comm type can be "demo"
        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[9], sizeof(GUI_MENUITEM));
        sprintf(stCommMenu[iMenuItemNum].szText, "%s", stDefCommMenu[9].szText);
        ++iMenuItemNum;
    }

    stCommMenu[iMenuItemNum].szText[0] = 0;

	memset(szTitle, 0, sizeof(szTitle));
	if (ucMode==0)
	{
		pucCommType = &glSysParam.stTxnCommCfg.ucCommType;
		strcpy(szTitle, "1st:");
	}
	else
	{
		pucCommType = &glSysParam.stTxnCommCfg.ucCommTypeBak;
		strcpy(szTitle, "2nd:");
	}

	GetCommName(*pucCommType, szTitle+strlen(szTitle));

	Gui_BindMenu(szTitle, gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stCommMenu, &stSmDownMode);
	Gui_ClearScr();
	iSelected = 0;
	iRet = Gui_ShowMenuList(&stSmDownMode, 0, USER_OPER_TIMEOUT, &iSelected);
	if (iRet != GUI_OK)
	{
		return ERR_USERCANCEL;
	}

	glPosParams.commConfig = glSysParam.stTxnCommCfg;

	*pucCommType = (uchar)iSelected;
	return 0;
}

// 设置通讯参数
// set communication parameters
void SetSysCommParam(uchar ucPermission)
{
	SetCurrTitle("Network Config");
	Gui_ClearScr();
	while (1)
	{
		if (SetCommType(0)!=0)
		{
			break;
		}

		if (SetCommDetails(0, &glSysParam.stTxnCommCfg.ucCommType))
		{
			break;
		}

		if (SetCommType(1)!=0)
		{
			break;
		}

		if (SetCommDetails(1, &glSysParam.stTxnCommCfg.ucCommTypeBak))
		{
			break;
		}

		break;
	}
	SaveSysParam();
	SavePosParams();
}

int SetCommDetails(uchar mode, uchar *pucCommType)
{
	uchar	szDispBuff[32];
	int		iRet;

	sprintf((char *)szDispBuff, "SETUP ");
	GetCommName(*pucCommType, szDispBuff+strlen((char *)szDispBuff));
	SetCurrTitle(szDispBuff);

	iRet = 0;
	switch( *pucCommType )
	{
	case CT_RS232:
	    iRet = SetRs232Param(&glSysParam._TxnRS232Para);
		break;
		
	 case CT_BLTH:
		iRet = SetBTParam(&glSysParam._TxnBlueToothPara.stConfig);
		if(iRet != 0)
			break;
		SyncBTParam(&glSysParam._TmsBlueToothPara.stConfig, &glSysParam._TxnBlueToothPara.stConfig);
		CommOnHook(TRUE);
		DispWait();
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		break;
		
	case CT_WIFI:
		iRet = SetWiFiApp(&glSysParam._TxnWifiPara);
		if(iRet != 0)
		{
			DispWifiErrorMsg(iRet);
			break;
		}
		DispWait();
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		if(iRet != 0){
			DispWifiErrorMsg(iRet);
			break;
		}
		//SetTcpIpSharedPara(&glSysParam.stTxnCommCfg);
		SyncWifiParam(&glSysParam._TmsWifiPara, &glSysParam._TxnWifiPara);
	    break;

	case CT_MODEM:
		SetModemParam();
		break;

	case CT_TCPIP:
		//SetTcpIpSharedPara(&glSysParam.stTxnCommCfg);
		SetTcpIpParam(&glSysParam._TxnTcpIpPara);
		SyncTcpIpParam(&glSysParam._TmsTcpIpPara, &glSysParam._TxnTcpIpPara);
		DispWait();
		CommInitModule(&glSysParam.stTxnCommCfg);
	    break;

	case CT_GPRS:
	case CT_CDMA: 
	case CT_WCDMA:
		//SetTcpIpSharedPara(&glSysParam.stTxnCommCfg);
		SetWirelessParam(&glSysParam._TxnWirlessPara);
		SyncWirelessParam(&glSysParam._TmsWirlessPara, &glSysParam._TxnWirlessPara);
		CommOnHook(TRUE);
		DispWait();
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		break;

	case CT_DEMO:
	default:
	    break;
	}

	return iRet;
}

// 输入PABX
// enter PABX
int SetPabx(void)
{
	GUI_INPUTBOX_ATTR stInputBoxAttr;

	memset(&stInputBoxAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputBoxAttr.eType = GUI_INPUT_MIX;
	stInputBoxAttr.nMinLen = 0;
	stInputBoxAttr.nMaxLen = 10;
	stInputBoxAttr.bEchoMode = 1;

	Gui_ClearScr();
	if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "MODIFY PABX", gl_stLeftAttr, 
		glSysParam.stEdcInfo.szPabx, gl_stRightAttr, &stInputBoxAttr, USER_OPER_TIMEOUT)){
		return ERR_USERCANCEL;
	}

	return 0;
}

int SetRs232Param(RS232_PARA *rs232)
{
    uchar   ucCurBaud;

    GUI_MENU stBaudRateMenu;
    GUI_MENUITEM stBaudRateMenuItem[] = {
        { "9600", 0,TRUE,  NULL},
        { "38400", 1,TRUE,  NULL},
        { "57600", 2,TRUE,  NULL},
        { "115200", 3,TRUE,  NULL},
        { "", -1,FALSE,  NULL},
    };
    int iSelected = 0;

    int i;
     //---------------------------------------------------
    for(i = 0; i < sizeof(stBaudRateMenuItem)/ sizeof(GUI_MENUITEM)-1; ++i)
    {
        if(0 == memcmp(stBaudRateMenuItem[i].szText, rs232->szAttr, strlen(stBaudRateMenuItem[i].szText)))
        {
            ucCurBaud = i;
            iSelected = ucCurBaud;
            break;
        }
    }

    Gui_BindMenu("BAUD RATE:", gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stBaudRateMenuItem, &stBaudRateMenu);
    Gui_ClearScr();
    if(GUI_OK == Gui_ShowMenuList(&stBaudRateMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iSelected))
    {
        char szAttr[21] = {0};
        sprintf(szAttr, "%s%s", stBaudRateMenuItem[iSelected].szText, strchr(rs232->szAttr, ','));
        strcpy(rs232->szAttr, szAttr);
    }
    else
    {
        return ERR_USERCANCEL;
    }
    return 0;
}

// 修改Modem参数
// set Modem parameters
int SetModemParam(void)
{
	uchar	szPrompt[30], szBuff[50], ucCurBaud, ucTemp;

	GUI_MENU stBaudRateMenu;
	GUI_MENUITEM stBaudRateMenuItem[] = {
		{ "1200", 0,TRUE,  NULL},
		{ "2400", 1,TRUE,  NULL},
		{ "9600", 2,TRUE,  NULL},
		{ "14400", 3,TRUE,  NULL},
		{ "", -1,FALSE,  NULL},
	};
	int iSelected = 0;
	int iValue = 0;
	GUI_INPUTBOX_ATTR stInputAttr;

	if( SetPabx()!=0 )
	{
		return ERR_USERCANCEL;
	}

    iValue = glSysParam.stEdcInfo.bPreDial;
   //---------------------------------------------------
    Gui_ClearScr();
    Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "PRE DIAL", gl_stCenterAttr,
            "ON", TRUE, "OFF", FALSE, USER_OPER_TIMEOUT, &iValue);
    glSysParam.stEdcInfo.bPreDial = iValue;

    //---------------------------------------------------
	iValue = glSysParam._TxnModemPara.DP;
    Gui_ClearScr();
    Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "DIAL MODE", gl_stCenterAttr,
        "DTMF", 0, "PULSE", 1, USER_OPER_TIMEOUT, &iValue);
    glSysParam._TxnModemPara.DP = iValue;

    //---------------------------------------------------
    iValue = glSysParam._TxnModemPara.CHDT;
    Gui_ClearScr();
    Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "DIAL TONE", gl_stCenterAttr,
        "DETECT", 0, "IGNORE", 1, USER_OPER_TIMEOUT, &iValue);
    glSysParam._TxnModemPara.CHDT = iValue;

    //---------------------------------------------------
	memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 2;
	stInputAttr.bEchoMode = 1;

	sprintf((char *)szPrompt, "DIAL WAIT:");
	sprintf((char *)szBuff, "OLD:%u(*100ms)", (uint)glSysParam._TxnModemPara.DT1);

	Gui_ClearScr();
	if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.DT1 = (uchar)atoi((char *)szBuff);

    //---------------------------------------------------
	memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 2;
	stInputAttr.bEchoMode = 1;

	sprintf((char *)szPrompt, "PABX PAUSE:");
	sprintf((char *)szBuff, "OLD:%u(*100ms)", (uint)glSysParam._TxnModemPara.DT2);

	Gui_ClearScr();
	if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.DT2 = (uchar)atoi((char *)szBuff);

    //---------------------------------------------------
	memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 3;
	stInputAttr.bEchoMode = 1;

	sprintf((char *)szPrompt, "ONE DTMF HOLD:");
	sprintf((char *)szBuff, "OLD:%u(*1ms)", (uint)glSysParam._TxnModemPara.HT);

	Gui_ClearScr();
	if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.HT = (uchar)atoi((char *)szBuff);
  

    //---------------------------------------------------
	memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 3;
	stInputAttr.bEchoMode = 1;

	sprintf((char *)szPrompt, "DTMF CODE SPACE:");
	sprintf((char *)szBuff, "OLD:%u(*10ms)", (uint)glSysParam._TxnModemPara.WT);

	Gui_ClearScr();
	if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.WT = (uchar)atoi((char *)szBuff);

    //---------------------------------------------------
	memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 3;
	stInputAttr.bEchoMode = 1;

	sprintf((char *)szPrompt, "SIGNAL LEVEL:");
	sprintf((char *)szBuff, "OLD:%u(0, 1~15)", (uint)glSysParam._TxnPSTNPara.ucSignalLevel);
    while (1)
    {
		Gui_ClearScr();
		if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
			szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}
        ucTemp = (uchar)atoi((char *)szBuff);
        if (ucTemp<16)
        {
	        glSysParam._TxnPSTNPara.ucSignalLevel = ucTemp;
            break;
        }
    }

    //---------------------------------------------------
	ucCurBaud = (glSysParam._TxnModemPara.SSETUP>>5) & 0x03;
	iSelected = ucCurBaud;

	Gui_BindMenu("BAUD RATE:", gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stBaudRateMenuItem, &stBaudRateMenu);
	Gui_ClearScr();
	if(GUI_OK == Gui_ShowMenuList(&stBaudRateMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iSelected))
	{
		ucCurBaud = (unsigned char)(iSelected % 0xFF);
		glSysParam._TxnModemPara.SSETUP &= 0x9F;	// 1001 1111
		glSysParam._TxnModemPara.SSETUP |= (ucCurBaud<<5);
	}
	else
	{
		return ERR_USERCANCEL;
	}

	return 0;
}

int GetIpLocalSettings(void *pstParam)
{
	int		iRet;
	TCPIP_PARA *pstTcpipPara;

	pstTcpipPara = (TCPIP_PARA *)pstParam;

	iRet = GetIPAddress((uchar *)"LOCAL IP", TRUE, pstTcpipPara->szLocalIP);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetIPAddress((uchar *)"IP MASK", TRUE, pstTcpipPara->szNetMask);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetIPAddress((uchar *)"GATEWAY IP", TRUE, pstTcpipPara->szGatewayIP);
	if( iRet!=0 )
	{
		return iRet;
	}
	
	iRet = GetIPAddress((uchar *)"DNS", TRUE, pstTcpipPara->szDNSIP);
	if( iRet!=0 )
	{
		return iRet;
	}

	return 0;
}

int GetRemoteIp(const uchar *pszHalfText, uchar bAllowHostName, uchar bAllowNull, void *pstIPAddr)
{
	int		iRet;
	IP_ADDR	*pstIp;
	uchar	szBuff[51];

	pstIp = (IP_ADDR *)pstIPAddr;

	if(bAllowHostName)
	{
        sprintf((char *)szBuff, "%s Host", pszHalfText);
        iRet = GetHostDNS(szBuff, bAllowNull, pstIp->szIP);
        if( iRet!=0 )
        {
            return iRet;
        }
	}
	else
    {
        sprintf((char *)szBuff, "%s IP", pszHalfText);
        iRet = GetIPAddress(szBuff, bAllowNull, pstIp->szIP);
        if( iRet!=0 )
        {
            return iRet;
        }
    }

	sprintf((char *)szBuff, "%s PORT", pszHalfText);
	iRet = GetIPPort(szBuff, bAllowNull, pstIp->szPort);
	if( iRet<0 )
	{
		return iRet;
	}
	
	return 0;
}


//Add by Kevin_Wu for setting SSL ENV 2016-08-26
int SetSSLFlag()
{
	GUI_INPUTBOX_ATTR stInputBoxAttr;
	uchar szSLL[2] = {'\0'};
	uchar szCACert[20] = {'\0'};
	uchar szCliCert[20] = {'\0'};
	uchar szCliKey[20] = {'\0'};
	uchar szTemp[20] =  {'\0'};
	
	int iValue = 0;
	
	//Set SSL Flag
	if(GetEnv("E_SSL", szTemp) == 0)
	{
		sprintf((char *)szSLL, "%s", szTemp);
		iValue = szSLL[0] - '0';
	};
	
	Gui_ClearScr();
	
	if(GUI_OK != Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "E_SSL", gl_stLeftAttr,
        "ON", TRUE, "OFF", FALSE, USER_OPER_TIMEOUT, &iValue))
	{
		return ERR_USERCANCEL;
	}

	szSLL[0] = (uchar)(iValue + '0');

	PutEnv("E_SSL", szSLL);
	
	if(atoi(szSLL) != 0)
	{
		memset(&stInputBoxAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
		
		stInputBoxAttr.eType = GUI_INPUT_MIX;

		stInputBoxAttr.bEchoMode = 1;
		stInputBoxAttr.bSensitive = 0;

		stInputBoxAttr.nMinLen = 4;
		stInputBoxAttr.nMaxLen = 20;
		
		//Set ca-cert file name
		if(GetEnv("CA_CRT", szTemp) == 0)
		{
			sprintf((char *)szCACert, "%s", szTemp);
		}
		
		Gui_ClearScr();
		
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "CA_CRT", gl_stLeftAttr,
		szCACert, gl_stRightAttr, &stInputBoxAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}
		
		PutEnv("CA_CRT", szCACert);
		
		//Set cli-cert file name
		if(GetEnv("CLI_CRT", szTemp) == 0)
		{
			sprintf((char *)szCliCert, "%s", szTemp);
		}
		
		Gui_ClearScr();
		
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "CLI_CRT", gl_stLeftAttr,
		szCliCert, gl_stRightAttr, &stInputBoxAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}
		
		PutEnv("CLI_CRT", szCliCert);
		
		//Set cli-key file name
		if(GetEnv("CLI_KEY", szTemp) == 0)
		{
			sprintf((char *)szCliKey, "%s", szTemp);
		}
		Gui_ClearScr();
		
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "CLI_KEY", gl_stLeftAttr,
		szCliKey, gl_stRightAttr, &stInputBoxAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}

		PutEnv("CLI_KEY", szCliKey);	
	}
	
	return 0;
}

int ChkIfValidIp(const uchar *pszIP)
{
	return ((pszIP[0]!=0) && (IsValidIPAddress(pszIP)));
}

int ChkIfValidPort(const uchar *pszPort)
{
	return ((pszPort[0]!=0) &&
			(atol((uchar *)pszPort)>0) &&
			(atol((uchar *)pszPort)<65536));
}

int SetTcpIpSharedPara(COMM_CONFIG *pstCommCfg)
{
	int	iSel = pstCommCfg->ucTCPClass_BCDHeader;

	Gui_ClearScr();
	if(GUI_OK != Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "TCP LENGTH", gl_stCenterAttr, 
		_T("BCD"), 1, _T("HEX"), 0, USER_OPER_TIMEOUT, &iSel))
	{
		return -1;
	}

	if(1 == iSel)
	{
		pstCommCfg->ucTCPClass_BCDHeader = TRUE;
	}
	else
	{
		pstCommCfg->ucTCPClass_BCDHeader = FALSE;
	}
	return 0;
}

// 设置TCP/IP参数
// set TCP/IP parameters
int SetTcpIpParam(void *pstParam)
{
	int		iRet;

	// !!!! 需要应用到开机步骤
    iRet = SetTcpIpParam_S80((TCPIP_PARA *)pstParam);
	return iRet;
}

void SyncTcpIpParam(void *pstDst, const void *pstSrc)
{
	((TCPIP_PARA *)pstDst)->ucDhcp = ((TCPIP_PARA *)pstSrc)->ucDhcp;
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szLocalIP),   (char *)(((TCPIP_PARA *)pstSrc)->szLocalIP));
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szGatewayIP), (char *)(((TCPIP_PARA *)pstSrc)->szGatewayIP));
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szNetMask),   (char *)(((TCPIP_PARA *)pstSrc)->szNetMask));
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szDNSIP),     (char *)(((TCPIP_PARA *)pstSrc)->szDNSIP));
}

// Modified by Kim_LinHB 2014-5-31
// Added by Kim_LinHB 2014-5-31
int SetTcpIpParam_S80(TCPIP_PARA *pstParam)
{
	int		iRet;
	int iSelected = 0;
	uchar	szDispBuff[100];
	long	lTcpState;

	iRet = DhcpCheck();
	if (iRet==0)
	{
		sprintf((char *)szDispBuff, "DHCP: OK");
		iSelected = 1;
	}
	else
	{
		sprintf((char *)szDispBuff, "DHCP: STOPPED");
		iSelected = 0;
	}

	Gui_ClearScr();
	if(GUI_OK == Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, szDispBuff, gl_stCenterAttr,
		_T("START"), 1, _T("STOP"), 0, USER_OPER_TIMEOUT, &iSelected))
	{
		if(1 == iSelected)
		{
			pstParam->ucDhcp = 1;

			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("Getting IP..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
			if (SxxDhcpStart(FALSE, 30)==0)
			{
			    GUI_PAGE stPage;
			    GUI_PAGELINE stPageLines[4];
			    int iLines = 0;
				iRet = EthGet(pstParam->szLocalIP, pstParam->szNetMask, pstParam->szGatewayIP, pstParam->szDNSIP, &lTcpState);

				sprintf(stPageLines[iLines].szLine, "%s:%s", "LOCAL IP", pstParam->szLocalIP);
				stPageLines[iLines].stLineAttr.eAlign = GUI_ALIGN_LEFT;
				stPageLines[iLines].stLineAttr.eFontSize = GUI_FONT_NORMAL;
				stPageLines[iLines].stLineAttr.eStyle = GUI_FONT_STD;
				++iLines;

				sprintf(stPageLines[iLines].szLine, "%s:%s", "IP MASK", pstParam->szNetMask);
                stPageLines[iLines].stLineAttr.eAlign = GUI_ALIGN_LEFT;
                stPageLines[iLines].stLineAttr.eFontSize = GUI_FONT_NORMAL;
                stPageLines[iLines].stLineAttr.eStyle = GUI_FONT_STD;
                ++iLines;

				sprintf(stPageLines[iLines].szLine, "%s:%s", "GATEWAY IP", pstParam->szGatewayIP);
                stPageLines[iLines].stLineAttr.eAlign = GUI_ALIGN_LEFT;
                stPageLines[iLines].stLineAttr.eFontSize = GUI_FONT_NORMAL;
                stPageLines[iLines].stLineAttr.eStyle = GUI_FONT_STD;
                ++iLines;

				sprintf(stPageLines[iLines].szLine, "%s:%s", "DNS", pstParam->szDNSIP);
                stPageLines[iLines].stLineAttr.eAlign = GUI_ALIGN_LEFT;
                stPageLines[iLines].stLineAttr.eFontSize = GUI_FONT_NORMAL;
                stPageLines[iLines].stLineAttr.eStyle = GUI_FONT_STD;
                ++iLines;

                Gui_CreateInfoPage(GetCurrTitle(), gl_stTitleAttr, stPageLines, iLines, &stPage);
                Gui_ShowInfoPage(&stPage, 0, USER_OPER_TIMEOUT);
				return 0;
			}
		}
		else
		{
			pstParam->ucDhcp = 0;
		}
	}
	else
	{
		return -1;
	}
	

	// Manual setup
	if (iRet == 0)
	{
		DhcpStop();
	}

	if (pstParam->ucDhcp)
	{
		iRet = EthGet(pstParam->szLocalIP, pstParam->szNetMask, pstParam->szGatewayIP, pstParam->szDNSIP, &lTcpState);
	}

	iRet = GetIpLocalSettings(pstParam);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = EthSet(pstParam->szLocalIP, pstParam->szNetMask, pstParam->szGatewayIP, pstParam->szDNSIP);
	if (iRet < 0)
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, "SET STATIC IP\nFAILED.", gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
		return -1;
	}

	return 0;
}

static int  GetHostDNS(const uchar *pszPrompts, uchar bAllowNull, uchar *pszName)
{
    uchar   szTemp[50 + 1];
    GUI_INPUTBOX_ATTR stInputBoxAttr;
   
    memset(&stInputBoxAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	 stInputBoxAttr.eType = GUI_INPUT_MIX;
    stInputBoxAttr.bEchoMode = 0;
    if(bAllowNull)
        stInputBoxAttr.nMinLen = 0;
    else
        stInputBoxAttr.nMinLen = 1;
    stInputBoxAttr.nMaxLen = sizeof(szTemp) - 1;

    sprintf((char *)szTemp, "%.50s", pszName);
    Gui_ClearScr();
    if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, pszPrompts, gl_stLeftAttr,
      szTemp, gl_stRightAttr, &stInputBoxAttr, USER_OPER_TIMEOUT))
    {
      return ERR_USERCANCEL;
    }

    if( bAllowNull && szTemp[0]==0 )
    {
      *pszName = 0;
    }
    else{
      sprintf((char *)pszName, "%.50s", szTemp);
    }
    return 0;
}

// 输入IP地址
// get Ip address
int GetIPAddress(const uchar *pszPrompts, uchar bAllowNull, uchar *pszIPAddress)
{
	uchar	szTemp[20];
	GUI_INPUTBOX_ATTR stInputBoxAttr;

	memset(&stInputBoxAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputBoxAttr.eType = GUI_INPUT_MIX;
    stInputBoxAttr.nMinLen = (bAllowNull ? 0 : 1);
    stInputBoxAttr.nMaxLen = 15;
	stInputBoxAttr.bEchoMode = 1;

	sprintf((char *)szTemp, "%.15s", pszIPAddress);

	while(1){
        Gui_ClearScr();
        if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, pszPrompts, gl_stLeftAttr,
            szTemp, gl_stCenterAttr, &stInputBoxAttr, USER_OPER_TIMEOUT))
        {
            return ERR_USERCANCEL;
        }

        if( bAllowNull && szTemp[0]==0 )
        {
            *pszIPAddress = 0;
        }
        else{
            if(!IsValidIPAddress(szTemp)){
                Beep();
                continue;
            }
            sprintf((char *)pszIPAddress, "%.15s", szTemp);
        }
        return 0;
	}
	return 0;
}

// 检查IP地址
// verify the format of IP address
uchar IsValidIPAddress(const char *pszIPAddr)
{
	int		i;
	char	*p, *q, szBuf[5+1], szIp[16 + 1];

	sprintf(szIp, "%.*s",sizeof(szIp), pszIPAddr); // Modified by Kim_LinHB 2014-8-11 bug507

	PubTrimStr(szIp);
	if( *szIp==0 )
	{
		return FALSE;
	}

	p = strchr(szIp, ' ');
	if( p!=NULL )
	{
		return FALSE;
	}
	if( strlen(szIp)>15 )
	{
		return FALSE;
	}

	// 1st --- 3rd  part
	for(q=szIp, i=0; i<3; i++)
	{
		p = strchr(q, '.');
		if( p==NULL || p==q || p-q>3 )
		{
			return FALSE;
		}
		sprintf(szBuf, "%.*s", (int)(p-q), q);
		if( !IsNumStr(szBuf) || atoi(szBuf)>255 )
		{
			return FALSE;
		}
		q = p + 1;
	}

	// final part
	p = strchr((char *)q, '.');
	if( p!=NULL || !IsNumStr(q) || strlen(q)==0 || strlen(q)>3 || atoi(q)>255 )
	{
		return FALSE;
	}

	return TRUE;
}

// 输入端口
// get IP port
int GetIPPort(const uchar *pszPrompts, uchar bAllowNull, uchar *pszPortNo)
{
	int		iTemp;
	uchar	szTemp[15];

	GUI_INPUTBOX_ATTR stInputBoxAttr;

	memset(&stInputBoxAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputBoxAttr.eType = GUI_INPUT_NUM;
	stInputBoxAttr.nMinLen = (bAllowNull ? 0 : 1);
	stInputBoxAttr.nMaxLen = 5;
	stInputBoxAttr.bEchoMode = 1;

	while( 1 )
	{
		sprintf((char *)szTemp, "%.5s", pszPortNo);
		Gui_ClearScr();
		
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, pszPrompts, gl_stLeftAttr, 
			szTemp, gl_stCenterAttr, &stInputBoxAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}

		iTemp = atoi((char *)szTemp);
		if( iTemp>0 && iTemp<65535 )
		{
			sprintf((char *)pszPortNo, "%.5s", szTemp);
			break;
		}
		if (bAllowNull)
		{
			pszPortNo[0] = 0;
			break;
		}

		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("INV PORT #"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 4, NULL);
	}

	return 0;
}

// 设置电话号码
// set tel NO.
int SetTel(uchar *pszTelNo, const uchar *pszPromptInfo)
{
	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 24;
	stInputAttr.bEchoMode = 1;
   
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, pszPromptInfo, gl_stLeftAttr, 
		pszTelNo, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	return 0;
}

int SetWirelessParam(WIRELESS_PARAM *pstParam)
{
	GUI_INPUTBOX_ATTR stInputAttr;
	int iSelect = 0;
	if (pstParam==NULL)
	{
		return ERR_NO_DISP;
	}

	Gui_ClearScr();
	iSelect = pstParam->ucUsingSlot;
    if(GUI_OK != Gui_ShowAlternative("SETUP WIRELESS", gl_stTitleAttr, "Select SIM", gl_stCenterAttr,
        "SIM 1", 0, "SIM 2", 1, USER_OPER_TIMEOUT, &iSelect))
    {
        return ERR_USERCANCEL;
    }
    pstParam->ucUsingSlot = iSelect;


	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;

	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 32;
    Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP WIRELESS", gl_stTitleAttr, "APN", gl_stLeftAttr, pstParam->szAPN, 
		gl_stRightAttr, &stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 32;
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP WIRELESS", gl_stTitleAttr, "LOGIN NAME", gl_stLeftAttr, 
		pstParam->szUID, gl_stRightAttr, &stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 0;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 16;
	stInputAttr.bSensitive = 1;
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP WIRELESS", gl_stTitleAttr, "LOGIN PWD", gl_stLeftAttr, 
		pstParam->szPwd, gl_stRightAttr, &stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 16;
	stInputAttr.bSensitive = 1;
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP WIRELESS", gl_stTitleAttr, "SIM PIN", gl_stLeftAttr, pstParam->szSimPin, 
		gl_stRightAttr,&stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	return 0;
}

void SyncWirelessParam(WIRELESS_PARAM *pstDst, const WIRELESS_PARAM *pstSrc)
{
	strcpy((char *)(pstDst->szAPN),    (char *)(pstSrc->szAPN));
	strcpy((char *)(pstDst->szUID),    (char *)(pstSrc->szUID));
	strcpy((char *)(pstDst->szPwd),    (char *)(pstSrc->szPwd));
	strcpy((char *)(pstDst->szSimPin), (char *)(pstSrc->szSimPin));
	strcpy((char *)(pstDst->szDNS),    (char *)(pstSrc->szDNS));
}

//Added by Kim_LinHB 2014-8-16
//TODO for now, ignore master case
int  SetBTParam(ST_BT_CONFIG *pstParam)
{
	GUI_INPUTBOX_ATTR stInputAttr;

	if (pstParam==NULL)
	{
		return ERR_NO_DISP;
	}

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;

	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = sizeof(pstParam->name);
    Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP BULETOOTH", gl_stTitleAttr, "Name", gl_stLeftAttr, pstParam->name, 
		gl_stRightAttr, &stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = sizeof(pstParam->pin);
	stInputAttr.bSensitive = 1;
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP BULETOOTH", gl_stTitleAttr, "PIN", gl_stLeftAttr, pstParam->pin, 
		gl_stRightAttr,&stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	return 0;
}
void SyncBTParam(ST_BT_CONFIG *pstDst, const ST_BT_CONFIG *pstSrc)
{
#ifdef _MIPS_
	pstDst->role = pstSrc->role;
	pstDst->baud = pstSrc->baud;
#endif
	strcpy((char *)(pstDst->name),   (char *)(pstSrc->name));
	strcpy((char *)(pstDst->pin),    (char *)(pstSrc->pin));
	strcpy((char *)(pstDst->mac),    (char *)(pstSrc->mac));
}
//Add End

// 设置EDC参数
// set EDC parameters
void SetEdcParam(uchar ucPermission)
{
    int iSelected;
    GUI_MENU stMenu;
    GUI_MENUITEM stMenuItem[] = {
        { _T_NOOP("CURRENCY CODE"), 0,TRUE,  NULL},
        { _T_NOOP("CURRENCY CODE"), 1,TRUE,  NULL},
        { _T_NOOP("DECIMAL POSITION"), 2,TRUE,  NULL},
        { _T_NOOP("IGNORE DIGIT"), 3,TRUE,  NULL},
        { _T_NOOP("MERCHANT NAME"), 4,TRUE,  NULL},
        { _T_NOOP("MERCHANT ADDR"), 5, TRUE,  NULL},
        { _T_NOOP("PED MODE"), 6,TRUE,  NULL},
        { _T_NOOP("CONFIRM TIMEOUT"), 7,TRUE,  NULL},
        { _T_NOOP("PRINTER TYPE"), 8,TRUE,  NULL},
		//modified by kevin liu 20160628
//        { _T_NOOP("RECEIPT #"), 8,TRUE,  NULL},
		{ _T_NOOP("RECEIPT #"), 9,TRUE,  NULL},
        { _T_NOOP("TRACE NO"), 10,TRUE,  NULL},
        { _T_NOOP("INVOICE NO"), 11,TRUE,  NULL},
        { _T_NOOP("INFO"), 12,TRUE,  NULL},
        { _T_NOOP("EXT INFO"), 13,TRUE,  NULL},
        { "", -1,FALSE,  NULL},
    };

    Gui_BindMenu(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stMenuItem, &stMenu);

    iSelected = 0;
    while( 1 )
    {
        Gui_ClearScr();

        if( GUI_OK != Gui_ShowMenuList(&stMenu, 0, USER_OPER_TIMEOUT, &iSelected))
        {
            return;
        }

        switch(iSelected){
            case 0: SetTermCountry(ucPermission); break;
            case 1: SetTermCurrency(ucPermission); break;
            case 2: SetTermDecimalPosition(ucPermission); break;
            case 3: SetTermIgnoreDigit(ucPermission); break;
            case 4: SetMerchantName(ucPermission); break;
            case 5: SetMerchantAddr(ucPermission); break;
            case 6: SetPEDMode(); break;
            case 7: SetAcceptTimeOut(); break;
            case 8: SetPrinterType(); break;
            case 9: SetNumOfReceipt(); break;
            case 10: SetGetSysTraceNo(ucPermission); break;
            case 11: SetGetSysInvoiceNo(ucPermission); break;
            case 12: ModifyOptList(glSysParam.stEdcInfo.sOption, 'E', ucPermission); break;
            case 13: ModifyOptList(glSysParam.stEdcInfo.sExtOption, 'e', ucPermission); break;
        }
        SaveEdcParam();

#ifdef ENABLE_EMV
        if(iSelected <= 3)
            SyncEmvCurrency(glPosParams.currency.sCountryCode,
                            glPosParams.currency.sCurrencyCode,
                            glPosParams.currency.ucDecimal);
#endif
    }
}

// -1 : 值无改变 -2 : 超时或取消
// >=0 : 输入的合法值
// ucEdit      : 是否允许编辑
// pszFirstMsg : 标题下面第一行提示
// pszSecMsg   : 标题下面第二行提示
// ulMin,ulMax : 允许的取值范围
// lOrgValue   : 原值
// -1 : keeping -2 : timeout or cancel
// >=0 : valid input
// ucEdit      : if allowed to edit
// pszFirstMsg : the 1st line of prompt
// pszSecMsg   : the 2nd line of prompt
// ulMin,ulMax : value range
// lOrgValue   : original value
long ViewGetValue(uchar ucEdit, const void *pszFirstMsg, const void *pszSecMsg,
				  ulong ulMin, ulong ulMax, long lOrgValue)
{
	uchar	szBuff[32], szPrompt[200] = {0}, ucMinDigit, ucMaxDigit;
	ulong	ulTemp;
	int iRet;

	PubASSERT(ulMax<=0x07FFFFFF); // Modified by Kim_LinHB 2014-8-5 v1.01.0001
	//PubASSERT(ulMax<2147483648);

	ulTemp = ulMin;
	ucMinDigit = 0;
	do{ucMinDigit++;}while (ulTemp/=10);

	ulTemp = ulMax;
	ucMaxDigit = 0;
	do{ucMaxDigit++;}while (ulTemp/=10);

	memset(szBuff, 0, sizeof(szBuff));
	if (lOrgValue>=0)
	{
		sprintf((char *)szBuff, "%ld", lOrgValue);
	}

	if (pszFirstMsg!=NULL)
	{
		strcpy(szPrompt, _T(pszFirstMsg));
	}

	if (pszSecMsg!=NULL)
	{
		if(strlen(szPrompt) > 0){
			strcat(szPrompt, "\n");
		}
		strcat(szPrompt+strlen(szPrompt), _T(pszSecMsg));
	}

	if (ucEdit)
	{
		GUI_INPUTBOX_ATTR stInputAttr;

		memset(&stInputAttr, 0, sizeof(stInputAttr));
		stInputAttr.eType = GUI_INPUT_NUM;
		stInputAttr.nMinLen = ucMinDigit;
		stInputAttr.nMaxLen = ucMaxDigit;
		stInputAttr.bEchoMode = 1;

		while (1)
		{
			Gui_ClearScr();
			// Allow to modify 
			if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr, szBuff, 
				gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
			{
				return -2;
			}
			ulTemp = (ulong)atol((char *)szBuff);
			if ((ulTemp<ulMin) || (ulTemp>ulMax))
			{
				Gui_ClearScr();
				PubBeepErr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("INVALID VALUE"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
				continue;
			}

			if (ulTemp!=(ulong)lOrgValue)
			{
				return (long)ulTemp;
			}
			return -1;
		}
	}
	else
	{
		// Read only
		if(strlen(szPrompt) > 0){
			strcat(szPrompt, "\n");
			strcat(szPrompt, szBuff);
		}
		else{
			strcpy(szPrompt, szBuff);
		}
		Gui_ClearScr();
		iRet = Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL);
		
		if (iRet != GUI_OK)
		{
			return -2;
		}
		return -1;
	}
	return 0;
}

int SetTermCountry(uchar ucPermission)
{
	uchar	szBuff[32];

	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 1;

	stInputAttr.nMinLen = 3;
	stInputAttr.nMaxLen = 3;

	// Country Code
	PubBcd2Asc0(glPosParams.currency.sCountryCode, 2, szBuff);
    // Allow modify
	Gui_ClearScr();
    if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("AREA CODE"), gl_stLeftAttr, 
		szBuff+1, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
    {
        return ERR_USERCANCEL;
    }
        
    PubAsc2Bcd(szBuff, 3, glPosParams.currency.sCountryCode);
    return 0;
}
int SetTermCurrency(uchar ucPermission)
{
    uchar   szBuff[32];
    CURRENCY_CONFIG stCurrency;

    GUI_INPUTBOX_ATTR stInputAttr;
    memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
    stInputAttr.bEchoMode = 1;

    stInputAttr.nMinLen = 3;
    stInputAttr.nMaxLen = 3;
	// Currency
	PubBcd2Asc0(glPosParams.currency.sCurrencyCode, 2, szBuff);
	memmove(szBuff, szBuff+1, 4);

	if (ucPermission<PM_HIGH)
	{
		// Modified by Kim_LinHB 2014-08-18 v1.01.0004
		int iRet;
		unsigned char szBuff_Temp[200];
		sprintf(szBuff_Temp, "%s\n%s", _T("CURRENCY CODE"), szBuff);
		// Read only
		Gui_ClearScr();
		iRet = Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff_Temp, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
		if (iRet != GUI_OK)
		{
			return ERR_USERCANCEL;
		}
	}
	else
	{
		while(1)
		{
			while (2)
			{
				PubBcd2Asc0(glPosParams.currency.sCurrencyCode, 2, szBuff);
				memmove(szBuff, szBuff+1, 4);

				Gui_ClearScr();
				// Allow modify
				if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("CURRENCY CODE"), 
					gl_stLeftAttr, szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
				{
					return ERR_USERCANCEL;
				}
				if (FindCurrency(szBuff, &stCurrency)!=0)
				{   
					Gui_ClearScr();
					PubBeepErr();
					Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("INVALID CURRENCY"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
					continue;
				}
				break;
			}
       
			Gui_ClearScr();
			sprintf((char *)szBuff, "%.3s %02X%02X",
				stCurrency.szName, stCurrency.sCurrencyCode[0], stCurrency.sCurrencyCode[1]);
			if (GUI_ERR_USERCANCELLED == Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL))
			{
				continue;
			}
			break;
		}
		sprintf((char *)glPosParams.currency.szName, "%.3s", stCurrency.szName);
		memcpy(glPosParams.currency.sCurrencyCode, stCurrency.sCurrencyCode, 2);
	}
	return 0;
}

int SetTermDecimalPosition(uchar ucPermission)
{
	// Input decimal position value, 0<=x<=3
	// for JPY and KRW, x=0; for TWD, x=0 or x=2
    long lTemp = ViewGetValue((uchar)(ucPermission>PM_MEDIUM), _T("DECIMAL POSITION"), NULL,
						0, 3, (long)glPosParams.currency.ucDecimal);
	if (lTemp==-2)
	{
		return ERR_USERCANCEL;
	}
	if (lTemp>=0)
	{
		glPosParams.currency.ucDecimal = (uchar)lTemp;
	}
	return 0;
}

int SetTermIgnoreDigit(uchar ucPermission)
{
	// Input ignore digit value, 0<=x<=3
	// for JPY and KRW, x=2; for TWD, when decimal=0, x=2; decimal=2, x=0;
	long lTemp = ViewGetValue((uchar)(ucPermission>PM_MEDIUM), _T("IGNORE DIGIT"), NULL,
						0, 3, (long)glPosParams.currency.ucIgnoreDigit);
	if (lTemp==-2)
	{
		return ERR_USERCANCEL;
	}
	if (lTemp>=0)
	{
		glPosParams.currency.ucIgnoreDigit = (uchar)lTemp;
	}

	return 0;
}

int SetMerchantName(uchar ucPermission)
{
	uchar	szBuff[46+1];

	GUI_INPUTBOX_ATTR stInputAttr;

	if (ucPermission<PM_HIGH)	// Not allow to set
	{
	    uchar szTemp[255];
	    sprintf(szTemp, "%s\n%s", _T("MERCHANT NAME"), glSysParam.stEdcInfo.szMerchantName);
	    Gui_ClearScr();
	    if (GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szTemp, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL))
	    {
	        return ERR_USERCANCEL;
        }
	    return 0;
	}


	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	
	//NAME
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 23;
	sprintf((char *)szBuff, "%.23s", (char *)glSysParam.stEdcInfo.szMerchantName);
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("MERCHANT NAME"), gl_stLeftAttr, 
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	if (strcmp((char *)glSysParam.stEdcInfo.szMerchantName, (char *)szBuff)!=0)
	{
		sprintf((char *)glSysParam.stEdcInfo.szMerchantName, "%.23s", (char *)szBuff);
	}
	return 0;
}
int SetMerchantAddr(uchar ucPermission)
{
    uchar   szBuff[46+1];

    GUI_INPUTBOX_ATTR stInputAttr;

    if (ucPermission<PM_HIGH)   // Not allow to set
    {
        uchar szTemp[255];
        sprintf(szTemp, "%s\n%s", _T("MERCHANT ADDR"), glSysParam.stEdcInfo.szMerchantAddr);
        Gui_ClearScr();
        if (GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szTemp, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL))
        {
            return ERR_USERCANCEL;
        }
        return 0;
    }

    memset(&stInputAttr, 0, sizeof(stInputAttr));
    stInputAttr.bEchoMode = 1;

	//ADDRESS
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 46;
	sprintf((char *)szBuff, "%.46s", (char *)glSysParam.stEdcInfo.szMerchantAddr);
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("MERCHANT ADDR"), gl_stLeftAttr, 
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	if (strcmp((char *)glSysParam.stEdcInfo.szMerchantAddr, (char *)szBuff)!=0)
	{
		sprintf((char *)glSysParam.stEdcInfo.szMerchantAddr, "%.23s", (char *)szBuff);
	}

	return 0;
}

int SetGetSysTraceNo(uchar ucPermission)
{
	uchar	szBuff[20];

	Gui_ClearScr();
	if (ucPermission>PM_LOW)
	{
		GUI_INPUTBOX_ATTR stInputAttr;

		memset(&stInputAttr, 0, sizeof(stInputAttr));
		stInputAttr.eType = GUI_INPUT_NUM;
		stInputAttr.bEchoMode = 1;

		//NAME
		stInputAttr.nMinLen = 1;
		stInputAttr.nMaxLen = 6;

		sprintf((char *)szBuff, "%06ld", glSysCtrl.ulSTAN);
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "S.T.A.N.", gl_stLeftAttr, szBuff, 
			gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}

		glSysCtrl.ulSTAN = (ulong)atol((char *)szBuff);
		SaveSysCtrlBase();
	} 
	else
	{
		sprintf(szBuff, "S.T.A.N.\n%06ld", glSysCtrl.ulSTAN);
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
	}

	return 0;
}

int SetGetSysInvoiceNo(uchar ucPermission)
{
	uchar	szBuff[20];

	Gui_ClearScr();
	if (ucPermission>PM_LOW)
	{
		GUI_INPUTBOX_ATTR stInputAttr;

		memset(&stInputAttr, 0, sizeof(stInputAttr));
		stInputAttr.eType = GUI_INPUT_NUM;
		stInputAttr.bEchoMode = 1;

		//NAME
		stInputAttr.nMinLen = 1;
		stInputAttr.nMaxLen = 6;

		sprintf((char *)szBuff, "%06ld", glSysCtrl.ulInvoiceNo);
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "TRACE NO", gl_stLeftAttr, szBuff, 
			gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}

		glSysCtrl.ulInvoiceNo = (ulong)atol((char *)szBuff);
		SaveSysCtrlBase();
	} 
	else
	{
		sprintf(szBuff, "TRACE NO\n%06ld", glSysCtrl.ulInvoiceNo);
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
	}

	return 0;
}

// Select PED mode used.
int SetPEDMode(void)
{
	int	iSel = 0;

	GUI_MENU stPINPADMenu;
	GUI_MENUITEM stPINPADMenuItem[] = {
	    { "PCI PED", PED_INT_PCI, TRUE,  NULL},
		{ "PINPAD", PED_EXT_PP, TRUE,  NULL},
		{ "EXT PCI PINPAD", PED_EXT_PCI, TRUE,  NULL},
		{ "", -1, FALSE,  NULL},
	};

	iSel = glSysParam.stEdcInfo.ucPedMode;
	Gui_BindMenu(GetCurrTitle(), gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stPINPADMenuItem, &stPINPADMenu);
	
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowMenuList(&stPINPADMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iSel))
	{
		return ERR_USERCANCEL;
	}

	glSysParam.stEdcInfo.ucPedMode = (uchar)iSel;
	return 0;
}

// 输入交易成功时确认信息显示时间
// set the timeout for display "TXN accepted" message
int SetAcceptTimeOut(void)
{
	uchar	szBuff[2+1];

	GUI_TEXT_ATTR stPrompt, stContent;
	GUI_INPUTBOX_ATTR stInputAttr;

	stPrompt = stContent = gl_stCenterAttr;
	stPrompt.eAlign = GUI_ALIGN_LEFT;
#ifdef _Sxx_
	stPrompt.eFontSize = GUI_FONT_SMALL;
#endif
	stContent.eAlign = GUI_ALIGN_RIGHT;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 2;
	
	while( 1 )
	{
		sprintf((char *)szBuff, "%d", glSysParam.stEdcInfo.ucAcceptTimeout);
	   
		Gui_ClearScr();
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "Confirm Timeout", stPrompt, 
			szBuff, stContent, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}
		if( atoi((char *)szBuff)<=60 )
		{
			break;
		}

		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("INVALID INPUT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	}
	glSysParam.stEdcInfo.ucAcceptTimeout = (uchar)atoi((char *)szBuff);

	return 0;
}

int SetPrinterType(void)
{
	int 	iSel = glSysParam.stEdcInfo.ucPrinterType;

	// 仅适用于分离式打印机
	if (!ChkTerm(_TERMINAL_S60_))
	{
        uchar szTemp[255];
        sprintf(szTemp, "%s\n%s", _T("PRINTER TYPE"),
                glSysParam.stEdcInfo.ucPrinterType == 1
                ? _T("THERMAL")
                : _T("SPROCKET"));
        Gui_ClearScr();
        if (GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szTemp, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL))
        {
            return ERR_USERCANCEL;
        }
		return 0;
	}

	Gui_ClearScr();
	if(GUI_OK != Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "PRINTER TYPE", gl_stCenterAttr, 
		"THERMAL", 1, "SPROCKET", 0, USER_OPER_TIMEOUT, &iSel))
	{
		return ERR_USERCANCEL;
	}

	glSysParam.stEdcInfo.ucPrinterType = iSel;
	return 0;
}

// 输入热敏打印单据张数
// set receipt numbers, just for thermal terminal
int SetNumOfReceipt(void)
{
	uchar 	ucNum, szBuff[1+1];
	int iCnt = 0;
	GUI_TEXT_ATTR stPrompt, stContent;
	GUI_INPUTBOX_ATTR stInputAttr;

	if( !ChkIfThermalPrinter() )
	{
		return 0;
	}

	stPrompt = stContent = gl_stCenterAttr;
	stPrompt.eAlign = GUI_ALIGN_LEFT;
#ifdef _Sxx_
	stPrompt.eFontSize = GUI_FONT_SMALL;
#endif
	stContent.eAlign = GUI_ALIGN_RIGHT;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 1;

	iCnt = NumOfReceipt();
	while( 1 )
	{
		sprintf((char *)szBuff, "%d", iCnt);
		Gui_ClearScr();
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "Receipt pages", stPrompt, 
			szBuff, stContent, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}

		ucNum = (uchar)atoi((char *)szBuff);
		if( ucNum>=1 && ucNum<=4 )
		{
			ucNum--;
			break;
		}
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("INVALID INPUT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	}

	glSysParam.stEdcInfo.sOption[EDC_NUM_PRINT_LOW/0x100]  &= (0xFF^(EDC_NUM_PRINT_LOW%0x100));
	glSysParam.stEdcInfo.sOption[EDC_NUM_PRINT_HIGH/0x100] &= (0xFF^(EDC_NUM_PRINT_HIGH%0x100));
	if( ucNum & 0x01 )
	{
		glSysParam.stEdcInfo.sOption[EDC_NUM_PRINT_LOW/0x100] |= (EDC_NUM_PRINT_LOW%0x100);
	}
	if( ucNum & 0x02 )
	{
		glSysParam.stEdcInfo.sOption[EDC_NUM_PRINT_HIGH/0x100] |= (EDC_NUM_PRINT_HIGH%0x100);
	}

	return 0;
}

// 输入参数自动更新时间
// set timer for update parameters automatically
int SetCallInTime(void)
{
	return 0;
}

// TRUE:判断时间是否合法
// TRUE:it is a valid time
uchar IsValidTime(const uchar *pszTime)
{
	int		i, iHour, iMinute;

	for(i=0; i<4; i++)
	{
		if( pszTime[i]<'0' || pszTime[i]>'9' )
		{
			return FALSE;
		}
	}

	iHour   = (int)PubAsc2Long(pszTime, 2);
	iMinute = (int)PubAsc2Long(pszTime+2, 2);
	if( iHour>24 || iMinute>59 )
	{
		return FALSE;
	}
	if( iHour==24 && iMinute!=0 )
	{
		return FALSE;
	}

	return TRUE;
}

// 修改或者查看开关选项
// modify or view options
// Modified by Kim_LinHB 2014-08-18 v1.01.0004
int ModifyOptList(uchar *psOption, uchar ucMode, uchar ucPermission)
{
	// 通过FUN2进入设置时，若定义了FUN2_READ_ONLY，则用户权限为PM_LOW，否则用户权限为PM_MEDIUM
	// 使用无下载方式初始化时，用户权限为PM_HIGH
	// when setting in Function #2, if activate the macro FUN2_READ_ONLY, then user permission is PM_LOW, otherwise it is PM_MEDIUM
	// if initiated with "load default", then user permission is PM_HIGH

	// Protims可控的issuer option列表
	// available issuer options list in Protims
	static OPTION_INFO stIssuerOptList[] =
	{
// 		{"CAPTURE CASH",		ALLOW_EXTEND_PAY,			FALSE,	PM_MEDIUM},
		{"CAPTURE TXN",			ISSUER_CAPTURE_TXN,			FALSE,	PM_MEDIUM},
		{"ENABLE BALANCE",		ISSUER_EN_BALANCE,			FALSE,	PM_MEDIUM},
		{"ENABLE ADJUST",		ISSUER_EN_ADJUST,			FALSE,	PM_MEDIUM},
		{"ENABLE OFFLINE",		ISSUER_EN_OFFLINE,			FALSE,	PM_MEDIUM},
		{"ALLOW (PRE)AUTH",		ISSUER_NO_PREAUTH,			TRUE,	PM_MEDIUM},
		{"ALLOW REFUND",		ISSUER_NO_REFUND,			TRUE,	PM_MEDIUM},
		{"ALLOW VOID",			ISSUER_NO_VOID,				TRUE,	PM_MEDIUM},
		{"ENABLE EXPIRY",		ISSUER_EN_EXPIRY,			FALSE,	PM_MEDIUM},
		{"CHECK EXPIRY",		ISSUER_CHECK_EXPIRY,		FALSE,	PM_MEDIUM},
//		{"CHKEXP OFFLINE",		ISSUER_CHECK_EXPIRY_OFFLINE,FALSE,	PM_MEDIUM},
		{"CHECK PAN",			ISSUER_CHKPAN_MOD10,		FALSE,	PM_MEDIUM},
// 		{"CHECK PAN11",			ISSUER_CHKPAN_MOD11,		FALSE,	PM_MEDIUM},
//		{"EN DISCRIPTOR",		ISSUER_EN_DISCRIPTOR,		FALSE,	PM_MEDIUM},
		{"ENABLE MANUAL",		ISSUER_EN_MANUAL,			FALSE,	PM_MEDIUM},
		{"ENABLE PRINT",		ISSUER_EN_PRINT,			FALSE,	PM_MEDIUM},
		{"VOICE REFERRAL",		ISSUER_EN_VOICE_REFERRAL,	FALSE,	PM_MEDIUM},
		{"PIN REQUIRED",		ISSUER_EN_PIN,				FALSE,	PM_HIGH},
#ifdef ISSUER_EN_EMVPIN_BYPASS
		{"EMV PIN BYPASS",		ISSUER_EN_EMVPIN_BYPASS,	FALSE,	PM_MEDIUM},
#endif
//		{"ACCOUNT SELECT",		ISSUER_EN_ACCOUNT_SELECTION,FALSE,	PM_MEDIUM},
//		{"ROC INPUT REQ",		ISSUER_ROC_INPUT_REQ,		FALSE,	PM_MEDIUM},
//		{"DISP AUTH CODE",		ISSUER_AUTH_CODE,			FALSE,	PM_MEDIUM},
//		{"ADDTIONAL DATA",		ISSUER_ADDTIONAL_DATA,		FALSE,	PM_MEDIUM},
		{"4DBC WHEN SWIPE",		ISSUER_SECURITY_SWIPE,		FALSE,	PM_MEDIUM},
		{"4DBC WHEN MANUL",		ISSUER_SECURITY_MANUL,		FALSE,	PM_MEDIUM},
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims可控的acquirer option列表
	// available acquirer options list in Protims
	static OPTION_INFO stAcqOptList[] =
	{
		{"ONLINE VOID",			ACQ_ONLINE_VOID,			FALSE,	PM_MEDIUM},
		{"ONLINE REFUND",		ACQ_ONLINE_REFUND,			FALSE,	PM_MEDIUM},
		{"EN. TRICK FEED",		ACQ_DISABLE_TRICK_FEED,		TRUE,	PM_MEDIUM},
//		{"ADDTION PROMPT",		ACQ_ADDTIONAL_PROMPT,		FALSE,	PM_MEDIUM},
		{"AMEX ACQUIRER",		ACQ_AMEX_SPECIFIC_FEATURE,	FALSE,	PM_HIGH},
		{"DBS FEATURE",			ACQ_DBS_FEATURE,			FALSE,	PM_MEDIUM},
		{"BOC INSTALMENT",		ACQ_BOC_INSTALMENT_FEATURE,	FALSE,	PM_MEDIUM},
		{"CITI INSTALMENT",		ACQ_CITYBANK_INSTALMENT_FEATURE,FALSE,	PM_MEDIUM},
#ifdef ENABLE_EMV
		{"EMV ACQUIRER",		ACQ_EMV_FEATURE,			FALSE,	PM_HIGH},
#endif
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims不可控的acquirer option列表
	// invalid in Protims
	static OPTION_INFO stAcqExtOptList[] =
	{
		// 因为只能在且必须在POS上修改，因此权限设为PM_LOW
		// this options list can only be modified on POS, so user permission is set as PM_LOW
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims可控的edc option列表
	// available EDC options list in Protims
	static OPTION_INFO stEdcOptList[] =
	{
//		{"AUTH PAN MASKING",	EDC_AUTH_PAN_MASKING,	FALSE,	PM_LOW},
//		{"SELECT ACQ_CARD",		EDC_SELECT_ACQ_FOR_CARD,FALSE,	PM_LOW},
//		{"ENABLE ECR",			EDC_ECR_ENABLE,			FALSE,	PM_MEDIUM},
		{"FREE PRINT",			EDC_FREE_PRINT,			FALSE,  PM_LOW},
		{"EN. INSTALMENT?",		EDC_ENABLE_INSTALMENT,	FALSE,	PM_MEDIUM},
		{"CAPTURE CASH",		EDC_CASH_PROCESS,		FALSE,	PM_MEDIUM},
		{"REFERRAL DIAL",		EDC_REFERRAL_DIAL,		FALSE,	PM_MEDIUM},
		{"AUTH MODE",			EDC_AUTH_PREAUTH,		FALSE,	PM_MEDIUM},
//		{"PRINT TIME",			EDC_PRINT_TIME,			FALSE,	PM_MEDIUM},
		{"TIP PROCESSING",		EDC_TIP_PROCESS,		FALSE,	PM_MEDIUM},
//		{"USE PRINTER",			EDC_USE_PRINTER,		FALSE,	PM_MEDIUM},
		{"NEED ADJUST PWD",		EDC_NOT_ADJUST_PWD,		TRUE,	PM_HIGH},
		{"NEED SETTLE PWD",		EDC_NOT_SETTLE_PWD,		TRUE,	PM_HIGH},
		{"NEED REFUND PWD",		EDC_NOT_REFUND_PWD,		TRUE,	PM_HIGH},
		{"NEED VOID PWD",		EDC_NOT_VOID_PWD,		TRUE,	PM_HIGH},
		{"NEED MANUAL PWD",		EDC_NOT_MANUAL_PWD,		TRUE,	PM_HIGH},
//		{"LOCKED EDC",			EDC_NOT_KEYBOARD_LOCKED,TRUE,	PM_MEDIUM},
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims不可控的edc option列表
	// invalid in Protims
	static OPTION_INFO stEdcExtOptList[] =
	{
		// 因为只能在且必须在POS上修改，因此权限设为PM_LOW
		// this options list can only be modified on POS, so user permission is set as PM_LOW
		{NULL, 0, FALSE, PM_DISABLE},
	};

	OPTION_INFO		*pstCurOpt;
	uchar			ucCnt, ucOptIndex, ucOptBitNo;
	int                iOption = 0;
    int iSelected;
    GUI_MENU stMenu;
    GUI_MENUITEM stMenuItem[50] = {
        { "", -1,FALSE,  NULL},
    };

	switch(ucMode)
	{
	case 'I':
	case 'i':
		pstCurOpt = (OPTION_INFO *)stIssuerOptList;
		break;
	case 'E':
		pstCurOpt = (OPTION_INFO *)stEdcOptList;
		break;
	case 'e':
		pstCurOpt = (OPTION_INFO *)stEdcExtOptList;
		break;
	case 'A':
		pstCurOpt = (OPTION_INFO *)stAcqOptList;
		break;
	case 'a':
		pstCurOpt = (OPTION_INFO *)stAcqExtOptList;
		break;
	default:
		break;
	}

	if( pstCurOpt->pText==NULL )
	{
	    Gui_ClearScr();
        PubBeepErr();
        Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("UNSUPPORTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
		return 0;
	}

	ucCnt = 0;
	while( 1 )
	{
	    stMenuItem[ucCnt].bVisible = TRUE;
	    stMenuItem[ucCnt].nValue = ucCnt;
	    stMenuItem[ucCnt].vFunc = NULL;
        sprintf((char *)stMenuItem[ucCnt].szText, "%.16s", (char *)pstCurOpt[ucCnt].pText);
        if( pstCurOpt[ucCnt+1].pText==NULL )
        {
            strcpy(stMenuItem[ucCnt+1].szText, "");
            break;
        }
        ucCnt++;
	}

    Gui_BindMenu(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stMenuItem, &stMenu);

    while( 1 )
    {
        iSelected = 0;
        Gui_ClearScr();

        if( GUI_OK != Gui_ShowMenuList(&stMenu, 0, USER_OPER_TIMEOUT, &iSelected))
        {
            return ERR_USERCANCEL;
        }

		ucOptIndex = (uchar)(pstCurOpt[iSelected].uiOptVal>>8);
		ucOptBitNo = (uchar)(pstCurOpt[iSelected].uiOptVal & 0xFF);
		if (pstCurOpt[ucCnt].ucInverseLogic)
		{
		    iOption = (psOption[ucOptIndex] & ucOptBitNo) ? 0 : 1;
		}
		else
		{
		    iOption = (psOption[ucOptIndex] & ucOptBitNo) ? 1 : 0;
		}

		if (ucPermission>=pstCurOpt[iSelected].ucPermissionLevel)
		{
		    char *option1 = "ON", *option2 = "OFF";
		    if(EDC_AUTH_PREAUTH == pstCurOpt[iSelected].uiOptVal){
		        option1 = "AUTH";
		        option2 = "PREAUTH";
		    }
			Gui_ClearScr();
			if(GUI_OK == Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, pstCurOpt[iSelected].pText, gl_stCenterAttr,
			        option1, 1, option2, 0, USER_OPER_TIMEOUT, &iOption))
			{
				if(1 == iOption)
				{
					if (pstCurOpt[ucCnt].ucInverseLogic)
					{
						psOption[ucOptIndex] &= ~ucOptBitNo;
					}
					else
					{
						psOption[ucOptIndex] |= ucOptBitNo;
					}
				}
				else
				{
					if (pstCurOpt[iSelected].ucInverseLogic)
					{
						psOption[ucOptIndex] |= ucOptBitNo;
					}
					else
					{
						psOption[ucOptIndex] &= ~ucOptBitNo;
					}
				}
				SaveEdcParam();
			}
		}
		else{
			unsigned char szBuff[100];
			sprintf(szBuff, "%s\n%s", (char *)pstCurOpt[iSelected].pText, 1 == iOption ? "ON" : "OFF");
			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
		}
    }
	return 0;
}

// 修改口令
// change passwords
int ChangePassword(void)
{
	GUI_MENU stChgPwdMenu;
	GUI_MENUITEM stDefChgPwdMenuItem[] =
	{
		{ _T_NOOP("TERMINAL   PWD"), 1,TRUE,    ModifyPasswordTerm},
		{ _T_NOOP("BANK       PWD"), 2,TRUE,    ModifyPasswordBank},
		{ _T_NOOP("MERCHANT   PWD"), 3,TRUE,    ModifyPasswordMerchant},
		{ _T_NOOP("VOID       PWD"), 4,TRUE,    ModifyPasswordVoid},
		{ _T_NOOP("REFUND     PWD"), 5,TRUE,    ModifyPasswordRefund},
		{ _T_NOOP("ADJUST     PWD"), 6,TRUE,    ModifyPasswordAdjust},
		{ _T_NOOP("SETTLE     PWD"), 7,TRUE,    ModifyPasswordSettle},
		{ "", -1,FALSE,  NULL},
	};

	GUI_MENUITEM stChgPwdMenuItem[20];
	int iMenuItemNum = 0;
	int i;
	for(i = 0; i < sizeof(stDefChgPwdMenuItem)/sizeof(GUI_MENUITEM); ++i){
	    if(stDefChgPwdMenuItem[i].bVisible)
        {
	        memcpy(&stChgPwdMenuItem[iMenuItemNum], &stDefChgPwdMenuItem[i], sizeof(GUI_MENUITEM));
            sprintf(stChgPwdMenuItem[iMenuItemNum].szText, "%s", stDefChgPwdMenuItem[i].szText);
            ++iMenuItemNum;
        }
	}

	stChgPwdMenuItem[iMenuItemNum].szText[0] = 0;


	Gui_BindMenu(_T("CHANGE PWD"), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stChgPwdMenuItem, &stChgPwdMenu);
	Gui_ClearScr();
	Gui_ShowMenuList(&stChgPwdMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, NULL);
	return 0;
}

// 手工设置系统时间
// set system time manually
int SetSysTime(void)
{
	uchar	szBuff[14+1], sInputTime[6];
	int iRet;

	memset(szBuff,0,sizeof(szBuff));
	strcpy(szBuff + 10, "00"); //ss

	Gui_ClearScr();
	memset(szBuff,0,sizeof(szBuff));
	iRet = Gui_ShowTimeBox(_T("SET TIME"), gl_stTitleAttr, szBuff, gl_stCenterAttr, 0, USER_OPER_TIMEOUT);
 
	if(GUI_OK == iRet){
		Gui_ClearScr();
		iRet = Gui_ShowTimeBox(_T("SET TIME"), gl_stTitleAttr, szBuff + 6, gl_stCenterAttr, 1, USER_OPER_TIMEOUT);
	}
	else{
		return ERR_NO_DISP;
	}

	if (GUI_OK == iRet)
	{
		PubAsc2Bcd(szBuff, 12, sInputTime);
		SetTime(sInputTime);
	}
	return 0;
}

// provide manual select and prompt message when pszLngName==NULL
// mode:
// 0--auto load the first available non-english language (if language file available)
// 1--auto load the last time used language
// 2--provide a menu for selection
// Modified by Kim_LinHB 2014-8-7 v1.01.0002
void SetSysLang(uchar ucSelectMode)
{
	int	iCnt, iTotal, iRet, iSel = 0;

	GUI_MENU stLangMenu;
	GUI_MENUITEM stLangMenuItem[32];

REDO_SELECT_LANG:
	if (ucSelectMode==0 || ucSelectMode==2)
	{
		// 搜寻已下载的语言文件，准备菜单
		// search the existed translation files, and prepare the menu list
		for (iCnt=0, iTotal=0;
			iCnt<sizeof(stLangMenuItem)/sizeof(stLangMenuItem[0])-1;
			iCnt++)
		{
			if (glLangList[iCnt].szDispName[0]==0)
			{
				break;
			}
			if ((iCnt==0) || fexist((char *)glLangList[iCnt].szFileName)>=0)
			{
				strcpy(stLangMenuItem[iTotal].szText, _T((char *)glLangList[iCnt].szDispName));
				stLangMenuItem[iTotal].bVisible = TRUE;
				stLangMenuItem[iTotal].nValue = iTotal + 1;
				stLangMenuItem[iTotal].vFunc = NULL;
				if(0 == strcmp(glLangList[iCnt].szDispName, glSysParam.stEdcInfo.stLangCfg.szDispName))
					iSel = iTotal;

				iTotal++;
			}
		}

		strcpy(stLangMenuItem[iTotal].szText, "");
		stLangMenuItem[iTotal].bVisible = FALSE;
		stLangMenuItem[iTotal].nValue = -1;
		stLangMenuItem[iTotal].vFunc = NULL;

		if (ucSelectMode==0)
		{
			// 首次加载
			// 如果有一个或多个非英文语言，自动选择第一个；否则选择英语
			// the first time loading 
			// if there are one or more than one translation files, then will select the first non-English language as default,
			// otherwise set English as default

			iSel = ((iTotal>1) ? 2 : 1);
		}
		else
		{
			// 菜单手动选择
			// display a language menu list to select manually
			Gui_BindMenu(GetCurrTitle(), gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stLangMenuItem, &stLangMenu);
			Gui_ClearScr();
			iRet = Gui_ShowMenuList(&stLangMenu, GUI_MENU_DIRECT_RETURN, 60, &iSel);
			if (iRet != GUI_OK)
			{
				return;
			}
		}
		if(iSel < 1)
			iSel = 1; //English

		for (iCnt=0; glLangList[iCnt].szDispName[0]!=0; iCnt++)
		{
			if (strcmp((char *)glLangList[iCnt].szDispName,(char *)stLangMenuItem[iSel-1].szText)==0)
			{
				glSysParam.stEdcInfo.stLangCfg = glLangList[iCnt];
				break;
			}
		}
	}

	// 设为英语
	// set with English
	if (strcmp(glSysParam.stEdcInfo.stLangCfg.szFileName, "")==0)
	{
		iRet = SetLng(NULL);
		glSysParam.stEdcInfo.stLangCfg = glLangList[0]; // Added by Kim_LinHB 9/9/2014 v1.01.0007 bug521
		return;
	}

	iRet = SetLng(glSysParam.stEdcInfo.stLangCfg.szFileName);
	if (iRet!=0)
	{
		glSysParam.stEdcInfo.stLangCfg = glLangList[0];
		return;
	}
	
	if ((ucSelectMode==0) || (ucSelectMode==2))
	{
		// 在初次加载或者手动选择模式下，检查字库是否含有此内码
		// check if the character selected is included in the font lib
		if (CheckSysFont()!=0)
		{
			while(1)
			{
				int iKey;
				Gui_ClearScr();
				if(GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, "DISPLAY/PRINT\nMAY HAVE PROBLEM\nSET ANYWAY ?",
					gl_stCenterAttr, GUI_BUTTON_NONE, USER_OPER_TIMEOUT, &iKey))
				{
					break;
				}
				if(KEYENTER == iKey)
				{
					break;
				}
				else if(KEYCANCEL == iKey)
				{
					break;
				}
			}

			iRet = SetLng(NULL);
			glSysParam.stEdcInfo.stLangCfg = glLangList[0];
			ucSelectMode = 2;
			goto REDO_SELECT_LANG;
		}
	}
}

// Set system language
int SetEdcLang(void)
{
	LANG_CONFIG	stLangBak;

	memcpy(&stLangBak, &glSysParam.stEdcInfo.stLangCfg, sizeof(LANG_CONFIG));

	SetCurrTitle(_T("SELECT LANG")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	SetSysLang(2);
#ifdef AREA_Arabia
    CustomizeAppLibForArabiaLang( strcmp(LANGCONFIG, "Arabia")==0 );
#endif

	if (memcmp(&stLangBak, &glSysParam.stEdcInfo.stLangCfg, sizeof(LANG_CONFIG)) != 0)
	{
		SaveEdcParam();
	}
	return 0;
}

#ifndef APP_MANAGER_VER
void SetEdcLangExt(const char *pszDispName)
{
	int	ii;
	for (ii=0; glLangList[ii].szDispName[0]!=0; ii++)
	{
		if (PubStrNoCaseCmp((uchar *)glLangList[ii].szDispName, pszDispName)==0)
		{
			if ((ii==0) || (fexist((char *)glLangList[ii].szFileName)>=0))
			{
				glSysParam.stEdcInfo.stLangCfg = glLangList[ii];
				SetSysLang(1);
#ifdef AREA_Arabia
                CustomizeAppLibForArabiaLang( strcmp(LANGCONFIG, "Arabia")==0 );
#endif
			}
		}
	}
}
#endif

int SetPowerSave(void)
{
    int iSel = glSysParam.stEdcInfo.ucIdleShutdown;
	uchar	ucTemp, szPrompt[100], szBuff[100];
	int		iRet;

	SetCurrTitle(_T("POWERSAVE OPTION")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	if (!ChkTerm(_TERMINAL_S90_))
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("UNSUPPORTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
		return ERR_NO_DISP;
	}
	else
	{
		GUI_INPUTBOX_ATTR stInputAttr;
		memset(&stInputAttr, 0, sizeof(stInputAttr));

		sprintf((char *)szBuff, _T("IDLE: SLEEP"));
		if (glSysParam.stEdcInfo.ucIdleShutdown)
		{
			sprintf((char *)szBuff, _T("IDLE: SHUTDOWN  "));
		}
		Gui_ClearScr();
		iRet = Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, 
			_T("SLEEP"), 0, _T("SHUTDOWN"), 1, USER_OPER_TIMEOUT, &iSel);
		
		if(GUI_OK == iRet){
		    glSysParam.stEdcInfo.ucIdleShutdown = iSel;
			if (1 == glSysParam.stEdcInfo.ucIdleShutdown &&
				glSysParam.stEdcInfo.ucIdleMinute<5)
			{
				glSysParam.stEdcInfo.ucIdleMinute = 5;
			}
			SaveSysParam();
		}
		else{
			return ERR_NO_DISP;
		}

		ucTemp = glSysParam.stEdcInfo.ucIdleMinute;

		stInputAttr.eType = GUI_INPUT_NUM;
		stInputAttr.nMinLen = 1;
		stInputAttr.nMaxLen = 2;
		stInputAttr.bEchoMode = 1;

		if (glSysParam.stEdcInfo.ucIdleShutdown)
		{
			sprintf(szPrompt, "%s[5-60mins]", _T("SHUTDOWN TIMEOUT"));
		}
		else
		{
			sprintf(szPrompt, "%s[1-60mins]", _T("PWR SAVE TIMEOUT"));
		}

		while (1)
		{
			Gui_ClearScr();
			sprintf((char *)szBuff, "%d", (int)ucTemp);
			iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
				szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
			if (iRet !=GUI_OK)
			{
				return ERR_NO_DISP;
			}
			ucTemp = (uchar)atol((char *)szBuff);
			if (ucTemp>60 || ucTemp<1)
			{
				continue;
			}
			if (glSysParam.stEdcInfo.ucIdleShutdown && (ucTemp<5))
			{
				continue;
			}

			if (glSysParam.stEdcInfo.ucIdleMinute!=ucTemp)
			{
				glSysParam.stEdcInfo.ucIdleMinute = ucTemp;
				SaveSysParam();
			}
			break;
		}
	}
	return 0;
}

int TestMagicCard1(void)
{
	TestMagicCard(1);
	return 0;
}

int TestMagicCard2(void)
{
	TestMagicCard(2);
	return 0;
}

int TestMagicCard3(void)
{
	TestMagicCard(3);
	return 0;
}

void TestMagicCard(int iTrackNum)
{
	uchar	ucRet;
	uchar	szMagTrack1[79+1], szMagTrack2[40+1], szMagTrack3[104+1];
	uchar	szTitle[16+1], szBuff[200];

	MagClose();
	MagOpen();
	MagReset();
	while( 1 )
	{
		sprintf((char *)szTitle, "TRACK %d TEST", iTrackNum);
		Gui_ClearScr();
		Gui_ShowMsgBox(szTitle, gl_stTitleAttr, _T("PLS SWIPE CARD"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 0, NULL);
		while( 2 )
		{
			if( 0 == kbhit() && getkey()==KEYCANCEL )
			{
				MagClose();
				return;
			}

			if( MagSwiped()==0 )
			{
				break;
			}
		}

		memset(szMagTrack1, 0, sizeof(szMagTrack1));
		memset(szMagTrack2, 0, sizeof(szMagTrack2));
		memset(szMagTrack3, 0, sizeof(szMagTrack3));
		ucRet = MagRead(szMagTrack1, szMagTrack2, szMagTrack3);
		
		if( iTrackNum==1 )
		{
			sprintf(szBuff, "RET:%02X\n%.21s\n Length=[%d]", ucRet,
				szMagTrack1[0]==0 ? (uchar *)"NULL" : szMagTrack1, strlen((char *)szMagTrack1));
		}
		else if (iTrackNum == 2)
		{
			sprintf(szBuff, "RET:%02X\n%.21s\n Length=[%d]", ucRet,
				szMagTrack2[0]==0 ? (uchar *)"NULL" : szMagTrack2, strlen((char *)szMagTrack2));
		}
		else
		{
			sprintf(szBuff, "RET:%02X\n%.21s\n Length=[%d]", ucRet,
				szMagTrack3[0]==0 ? (uchar *)"NULL" : szMagTrack3, strlen((char *)szMagTrack3));
		}

		Gui_ClearScr();
		if(GUI_OK != Gui_ShowMsgBox(szTitle, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL))
		{
			return;
		}
	}
}

int ToolsViewPreTransMsg(void)
{
	GUI_MENU stViewMsgMenu;
	GUI_MENUITEM stViewMsgMenuItem[] =
	{
		{ "OUTPUT SEND/RECV", 1,TRUE,  ShowExchangePack},
		{ "PRINT SEND/RECV", 2,TRUE,  PrnExchangePack},
		{ "", -1,FALSE,  NULL},
	};

	SetCurrTitle(_T("VIEW MSG"));
	if( PasswordBank()!=0 )
	{
		return ERR_NO_DISP;
	}

	Gui_BindMenu(GetCurrTitle(), gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stViewMsgMenuItem, &stViewMsgMenu);
	Gui_ClearScr();
	Gui_ShowMenuList(&stViewMsgMenu, 0, USER_OPER_TIMEOUT, NULL);
	return 0;
}

// 发送通讯报文到COM1
// send comm package to COM1
int ShowExchangePack(void)
{
#if defined(WIN32) && defined(_Sxx_)
#define DEBUG_OUT_PORT	PINPAD
#else
#define DEBUG_OUT_PORT	0
#endif
	if (!glSendData.uiLength && !glRecvData.uiLength)
	{
		DispErrMsg(_T("NO DATA"), NULL, 5, 0);
		return ERR_NO_DISP;
	}
	
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("SENDING..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	DebugNacTxd(DEBUG_OUT_PORT, glSendData.sContent, glSendData.uiLength);
	DelayMs(2000);
	DebugNacTxd(DEBUG_OUT_PORT, glRecvData.sContent, glRecvData.uiLength);

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("SEND OK"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
	return 0;
}

// 打印通讯报文
// Print comm package
int PrnExchangePack(void)
{
	SetCurrTitle(_T("VIEW MSG"));
	if (!glSendData.uiLength && !glRecvData.uiLength)
	{
		DispErrMsg(_T("NO DATA"), NULL, 5, 0);
		return ERR_NO_DISP;
	}

	
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

//	PubDebugOutput(_T("VIEW MSG"), glSendData.sContent, glSendData.uiLength,
//					DEVICE_PRN, ISO_MODE);
//	PubDebugOutput(_T("VIEW MSG"), glRecvData.sContent, glRecvData.uiLength,
//					DEVICE_PRN, ISO_MODE);
    PubDebugOutput(_T("VIEW MSG"), glSendData.sContent, glSendData.uiLength,
                    DEVICE_PRN, HEX_MODE);
    PubDebugOutput(_T("VIEW MSG"), glRecvData.sContent, glRecvData.uiLength,
                    DEVICE_PRN, HEX_MODE);

	Gui_ClearScr();
	return 0;
}

void DebugNacTxd(uchar ucPortNo, const uchar *psTxdData, ushort uiDataLen)
{
	uchar	*psTemp, sWorkBuf[LEN_MAX_COMM_DATA+10];
	uchar  ucInit = 0;
	
	if( uiDataLen>LEN_MAX_COMM_DATA )
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("INVALID PACK"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return;
	}

	sWorkBuf[0] = STX;
	sWorkBuf[1] = (uiDataLen/1000)<<4    | (uiDataLen/100)%10;	// convert to BCD
	sWorkBuf[2] = ((uiDataLen/10)%10)<<4 | uiDataLen%10;
	memcpy(&sWorkBuf[3], psTxdData, uiDataLen);
	sWorkBuf[3+uiDataLen]   = ETX;

	//sWorkBuf[3+uiDataLen+1] = PubCalcLRC(psTxdData, uiDataLen, (uchar)(sWorkBuf[1] ^ sWorkBuf[2] ^ ETX));
	PubCalcLRC(sWorkBuf + 1, (ushort)(uiDataLen+3), &ucInit);
	sWorkBuf[3+uiDataLen+1] = ucInit;
	//end
	uiDataLen += 5;

	PortClose(ucPortNo);
	PortOpen(ucPortNo, (void *)"9600,8,n,1");
	psTemp = sWorkBuf;
	while( uiDataLen-->0 )
	{
		if( PortSend(ucPortNo, *psTemp++)!=0 )
		{
			break;
		}
	}
	PortClose(ucPortNo);
}




int GetIpLocalWifiSettings(void *pstParam)
{
	int 	iRet;
	ST_WIFI_PARAM *pstWifiPara;
	TCPIP_PARA stLocalInfo;

	pstWifiPara = (ST_WIFI_PARAM *)pstParam;
	memset(&stLocalInfo, 0, sizeof(TCPIP_PARA));

	iRet = GetIPAddress((uchar *)"LOCAL IP", TRUE, stLocalInfo.szLocalIP);
	if( iRet!=0 )
	{
		return iRet;
	}
	SplitIpAddress(stLocalInfo.szLocalIP, pstWifiPara->Ip);
	
	iRet = GetIPAddress((uchar *)"IP MASK", TRUE, stLocalInfo.szNetMask);
	if( iRet!=0 )
	{
		return iRet;
	}
	SplitIpAddress(stLocalInfo.szNetMask, pstWifiPara->Mask);
	
	iRet = GetIPAddress((uchar *)"GATEWAY IP", TRUE, stLocalInfo.szGatewayIP);
	if( iRet!=0 )
	{
		return iRet;
	}
	SplitIpAddress(stLocalInfo.szGatewayIP, pstWifiPara->Gate);
	
	iRet = GetIPAddress((uchar *)"DNS", TRUE, stLocalInfo.szDNSIP);
	if( iRet!=0 )
	{
		return iRet;
	}
	SplitIpAddress(stLocalInfo.szDNSIP, pstWifiPara->Dns);
	
	return 0;
}

// Modified by Kim_LinHB 2014-08-19 v1.01.0004
int SetWiFiApp(void* pstParam)
{
	int iRet = -1;

	int iAppNum;
	ST_WIFI_AP stWiFiApp[MAX_WiFiApp]; //list of SSID searched
	WIFI_PARA *pstWifiPara = (WIFI_PARA *)pstParam;
#ifdef _MIPS_
	unsigned char szPWD[(2 * KEY_WEP_LEN ) > KEY_WPA_MAXLEN ? (2 * KEY_WEP_LEN ) : KEY_WPA_MAXLEN];
#else
	unsigned char szPWD[(2 * KEY_WEP_LEN_MAX ) > KEY_WPA_MAXLEN ? (2 * KEY_WEP_LEN_MAX ) : KEY_WPA_MAXLEN];
#endif
	
	int	iMenuNo;
	uchar ucCnt;
	int iSel;

	GUI_MENU	stWiFiAppsMenu;
	GUI_MENUITEM	stWiFiAppsMenuItem[MAX_WiFiApp+1];
	GUI_INPUTBOX_ATTR stInputAttr;

	iRet = WifiCheck(NULL);
	if (-3 == iRet)
	{
		WifiOpen();
	}

#ifdef _MIPS_
	WifiDisconAp();
#else
	WifiDisconnect();
#endif

	//期望扫描到15个
	//expect to scan 15 SSID at most
	iAppNum = 15;
	memset(stWiFiApp,0,sizeof(stWiFiApp)); 
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, "WIFI SCANNING...", gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

#ifdef _MIPS_
	iRet = WifiScanAps(stWiFiApp,iAppNum);
#else
	iRet = WifiScan(stWiFiApp,iAppNum);
#endif
	if(iRet < 0)
	{
		return iRet;
	}

	if(0 == iRet)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, "NOT FIND APPS", gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_USERCANCEL;
	}

	memset(stWiFiAppsMenuItem,0,sizeof(stWiFiAppsMenuItem));
	for(ucCnt=0; ucCnt< iRet && ucCnt < MAX_WiFiApp; ucCnt++)
	{
		sprintf((char *)stWiFiAppsMenuItem[ucCnt].szText, "%s", stWiFiApp[ucCnt].Ssid);
		stWiFiAppsMenuItem[ucCnt].bVisible = TRUE;
		stWiFiAppsMenuItem[ucCnt].nValue = ucCnt;
		stWiFiAppsMenuItem[ucCnt].vFunc = NULL;
	}

	Gui_BindMenu(GetCurrTitle(), gl_stCenterAttr, gl_stLeftAttr, stWiFiAppsMenuItem, &stWiFiAppsMenu);

	Gui_ClearScr();
	iMenuNo = 0;
	if(GUI_OK != Gui_ShowMenuList(&stWiFiAppsMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iMenuNo))
	{
		return ERR_USERCANCEL;
	}

	memcpy(&pstWifiPara->stLastAP, &stWiFiApp[iMenuNo], sizeof(ST_WIFI_AP));

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.bEchoMode = 1;
	stInputAttr.bSensitive = 1;

	memset(szPWD, 0, sizeof(szPWD));
	if(pstWifiPara->stLastAP.SecMode == WLAN_SEC_WEP )
	{   
		stInputAttr.nMinLen = 0;
#ifdef _MIPS_
		stInputAttr.nMaxLen = 2 * KEY_WEP_LEN;
#else
		stInputAttr.nMaxLen = 2 * KEY_WEP_LEN_MAX;
#endif

		Gui_ClearScr();
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("Enter PassWord:"), gl_stLeftAttr, 
			szPWD, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}
#ifdef _MIPS_
		PubAsc2Bcd(szPWD, strlen(szPWD), pstWifiPara->stParam.Wep); 
#else
		PubAsc2Bcd(szPWD, strlen(szPWD), pstWifiPara->stParam.Wep.Key[0]); 
#endif
	}

	//WLAN_SEC_WPA_WPA2 =2       WLAN_SEC_WPAPSK_WPA2PSK= 3
	if (pstWifiPara->stLastAP.SecMode == WLAN_SEC_WPA_WPA2 || 
		pstWifiPara->stLastAP.SecMode ==  WLAN_SEC_WPAPSK_WPA2PSK)
	{
		stInputAttr.nMinLen = 0;
		stInputAttr.nMaxLen = KEY_WPA_MAXLEN;
		Gui_ClearScr();
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("Enter PassWord:"), gl_stLeftAttr, 
			szPWD, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}
		memcpy(pstWifiPara->stParam.Wpa, szPWD, strlen(szPWD));
	}
	
	iSel = pstWifiPara->stParam.DhcpEnable;
	if(iSel > 1)
	    iSel = 1;

	Gui_ClearScr();
	if(GUI_OK != Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, _T("DHCP ENABLE"), gl_stCenterAttr,
		"ON", 1, "OFF", 0, USER_OPER_TIMEOUT, &iSel))
	{
		return ERR_USERCANCEL;
	}

	if(1 == iSel)
	{
		pstWifiPara->stParam.DhcpEnable = 1;
	}
	else
	{
		pstWifiPara->stParam.DhcpEnable = 0;
	}
	
	if(pstWifiPara->stParam.DhcpEnable == 0)
	{
		iRet = GetIpLocalWifiSettings(&pstWifiPara->stParam);
		if( iRet!=0 )
		{
			return iRet;
		}
	}
	return 0;
}

// Modified by Kim_LinHB 2014-08-19 v1.01.0004
void SyncWifiParam(void *pstDst, const void *pstSrc)
{
	memcpy(&((WIFI_PARA *)pstDst)->stHost1,   &((WIFI_PARA *)pstSrc)->stHost1, sizeof(IP_ADDR));
	memcpy(&((WIFI_PARA *)pstDst)->stHost2,   &((WIFI_PARA *)pstSrc)->stHost2, sizeof(IP_ADDR));

	((WIFI_PARA *)pstDst)->stParam.DhcpEnable = ((WIFI_PARA *)pstSrc)->stParam.DhcpEnable;
	strcpy((char *)(((WIFI_PARA *)pstDst)->stParam.Ip),   (char *)(((WIFI_PARA *)pstSrc)->stParam.Ip));
	strcpy((char *)(((WIFI_PARA *)pstDst)->stParam.Mask),   (char *)(((WIFI_PARA *)pstSrc)->stParam.Mask));
	strcpy((char *)(((WIFI_PARA *)pstDst)->stParam.Gate),   (char *)(((WIFI_PARA *)pstSrc)->stParam.Gate));
	strcpy((char *)(((WIFI_PARA *)pstDst)->stParam.Dns),   (char *)(((WIFI_PARA *)pstSrc)->stParam.Dns));

	memcpy((char *)(&((WIFI_PARA *)pstDst)->stParam), (char *)(&((WIFI_PARA *)pstSrc)->stParam), sizeof(ST_WIFI_PARAM));
	strcpy((char *)(&((WIFI_PARA *)pstDst)->stLastAP),   (char *)(&((WIFI_PARA *)pstSrc)->stLastAP));
}

void DispWifiErrorMsg( int Ret)
{
	unsigned char szBuff[100];
	
	switch(Ret)
	{
		case 0:
			strcpy(szBuff, _T(" CONNECTED"));
			break;	
		case -1:
			strcpy(szBuff, _T(" DEVICE FAILED"));
			break;
		case -2:
			strcpy(szBuff, _T(" WIFI NO RESPOND"));
			break;
		case -3:
			strcpy(szBuff, _T(" WIFI NOT OPEN"));
			break;
		case -4:
			strcpy(szBuff, _T(" NOT CONNECTED"));
			break;
		case -5:
			strcpy(szBuff, _T(" PARAM EMPTY"));
			break;
		case -6:
			strcpy(szBuff, _T(" PWD ERROR"));
			break;
		case -7:
			strcpy(szBuff, _T(" BAN OPERATION"));
			break;
		default:
			strcpy(szBuff, _T(" CANCELED "));
			break;
	}
	WifiClose();
	ScrSetIcon(ICON_WIFI, CLOSEICON);
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	return ;
}


int SetNetworkCommDetails(uchar *pucCommType, uchar ucAllowOptions)
{
	logTrace(__func__);
	uchar	szDispBuff[32];
	int		iRet;

	//sprintf((char *)szDispBuff, "SETUP ");
	//GetCommName(*pucCommType, szDispBuff + strlen((char *)szDispBuff));
	SetCurrTitle("COMM INIT");// (szDispBuff);

	iRet = 0;
	switch (*pucCommType)
	{
	case CT_RS232:
		iRet = SetRs232Param(&glPosParams._comRS232Para);
		break;

	case CT_BLTH:
		if (ucAllowOptions) {
			iRet = SetBTParam(&glPosParams._comBlueToothPara.stConfig);
			if (iRet != 0)
				break;
		}

		CommOnHook(TRUE);
		DispWait();
		iRet = CommInitModule(&glPosParams.commConfig);
		break;

	case CT_WIFI:
		if (ucAllowOptions) {
			iRet = SetWiFiApp(&glPosParams._comWifiPara);
			if (iRet != 0)
			{
				DispWifiErrorMsg(iRet);
				break;
			}
		}
		DispWait();
		iRet = CommInitModule(&glPosParams.commConfig);
		if (iRet != 0) {
			DispWifiErrorMsg(iRet);
			break;
		}
		//SetTcpIpSharedPara(&glPosParams.commConfig);
		break;

	case CT_MODEM:
		SetModemParam();
		break;

	case CT_TCPIP:
		//SetTcpIpSharedPara(&glPosParams.commConfig);
		if (ucAllowOptions) {
			SetTcpIpParam(&glPosParams._comTcpIpPara);
		}
		
		DispWait();
		CommInitModule(&glPosParams.commConfig);
		break;

	case CT_GPRS:
	case CT_CDMA:
	case CT_WCDMA:
		//SetTcpIpSharedPara(&glPosParams.commConfig);
		if (ucAllowOptions) {
			SetWirelessParam(&glPosParams._comWirlessPara);
		}
		CommOnHook(TRUE);
		DispWait();
		iRet = CommInitModule(&glPosParams.commConfig);
		break;

	case CT_DEMO:
	default:
		break;
	}

	return iRet;
}


// end of file

