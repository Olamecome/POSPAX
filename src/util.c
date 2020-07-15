
#include "global.h"

// Modified by Kim_LinHB 2014-7-11		Update GUI to new version

/********************** Internal macros declaration ************************/
#define TIMER_TEMPORARY		4
/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/
static uchar ChkAcqRestrictForCard(const uchar *pszPan);
static int  MatchCardTableForInstalment(uchar acq_index);
static void GetHolderNameFromTrack1(uchar *pszHolderName);
static void ConvertHolderName(const uchar *pszOrgName, uchar *pszNormalName);
static int  GetEmvTrackData(void);
static int  GetPanFromTrack(uchar *pszPAN, uchar *pszExpDate);
static int  MatchTrack2AndPan(const uchar *pszTrack2, const uchar *pszPan);
static int  MatchCardBin(const uchar *pszPAN);
static int  GetSecurityCode(void);
static void DispFallBackPrompt(void);
static int  InputAmount(uchar ucAmtType);
static int  VerifyManualPan(void);
static int  GetTipAmount(void);
static int  ExtractPAN(const uchar *pszPAN, uchar *pszOutPan);
static void DispWelcomeOnPED(void);
static int  GetExpiry(void);


/********************** Internal variables declaration *********************/

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

#ifdef AREA_Arabia
// Modified by Kim_LinHB 2014-8-7 v1.01.0002
// Modified by Kim_LinHB 2014-08-26 v1.01.0005
int CustomizeAppLibForArabiaLang(uchar bSetToArabia)
{
	unsigned char sTermInfo[30];

	if (bSetToArabia)
	{
#if !defined(_Sxx_)
		ST_FONT stFont[3] = {
			{ CHARSET_ARABIA, 8, 16, 0,0 },
			{ CHARSET_ARABIA, 16, 32,0,0 },
			{ CHARSET_ARABIA, 24, 40, 0,0 },
		};
#else
		ST_FONT stFont[3] = {
			{ CHARSET_ARABIA, 6, 8, 0,0 },
			{ CHARSET_ARABIA, 8, 16,0,0 },
			{ CHARSET_ARABIA, 12, 24, 0,0 },
		};
#endif

		gl_AR_FONT_ID = ArFontOpen("PAXARFA.FONT");
		if (gl_AR_FONT_ID > AR_ERROR_CODE_MIN)  //arabic
		{
			Gui_ClearScr();
			Gui_ShowMsgBox(NULL, gl_stTitleAttr, "Error:PAX_ARFA.FONT error\nPls download ParamFile", gl_stCenterAttr, GUI_BUTTON_CANCEL, -1, NULL);
			return -1;
		}
		Gui_Init(_RGB_INT_(255, 255, 238), _RGB_INT_(0, 20, 255), "PAXARFA.FONT");

		GetTermInfo(sTermInfo);
		memcpy(stFont, gl_Font_Def, sizeof(gl_Font_Def));
#ifdef _Sxxx_
		if ((sTermInfo[0] != _TERMINAL_S300_) && (sTermInfo[0] != _TERMINAL_S900_))
		{
			if (_TERMINAL_S800_ == sTermInfo[0]) {
				stFont[1].Width = 12;
				stFont[1].Height = 24;
			}
			else {
				stFont[0].Width = 6;
				stFont[0].Height = 8;

				stFont[1].Width = 8;
				stFont[1].Height = 16;
			}
		}
#endif

		Gui_LoadFont(GUI_FONT_SMALL, &stFont[0], NULL);
		Gui_LoadFont(GUI_FONT_NORMAL, &stFont[1], NULL);
		Gui_LoadFont(GUI_FONT_LARGE, &stFont[2], NULL);

		return 0;
	}
	else
	{
		ST_FONT stFont[3];

		ArFontClose(gl_AR_FONT_ID);
		gl_AR_FONT_ID = AR_OPENFILE_ERROR;

		Gui_Init(_RGB_INT_(255, 255, 238), _RGB_INT_(0, 20, 255), NULL);

		GetTermInfo(sTermInfo);
		memcpy(stFont, gl_Font_Def, sizeof(gl_Font_Def));
#ifdef _Sxxx_
		if ((sTermInfo[0] != _TERMINAL_S300_) && (sTermInfo[0] != _TERMINAL_S900_))
		{
			if (_TERMINAL_S800_ == sTermInfo[0]) {
				stFont[1].Width = 12;
				stFont[1].Height = 24;
			}
			else {
				stFont[0].Width = 6;
				stFont[0].Height = 8;

				stFont[1].Width = 8;
				stFont[1].Height = 16;
			}
		}
#endif

		Gui_LoadFont(GUI_FONT_SMALL, &stFont[0], NULL);
		Gui_LoadFont(GUI_FONT_NORMAL, &stFont[1], NULL);
		Gui_LoadFont(GUI_FONT_LARGE, &stFont[2], NULL);
		return 0;
	}
}
#endif

// 初始化交易参数
// initiate transaction parameters
void InitTransInfo(void)
{
	memset(&glProcInfo, 0, sizeof(SYS_PROC_INFO));
	glProcInfo.uiRecNo = 0xFFFF;
	glProcInfo.bIsFirstGAC = TRUE;
	sprintf((char *)glProcInfo.stTranLog.szOtherAmount, "%012ld", 0L);

	// set initial transaction currency to local currency at first
	glProcInfo.stTranLog.stTranCurrency = glPosParams.currency;		// initial currency
	glProcInfo.stTranLog.uiEntryMode = MODE_NO_INPUT;
	glProcInfo.stTranLog.bPanSeqOK = FALSE;
	glProcInfo.stTranLog.ulSTAN = GetNewTraceNo();
	glProcInfo.stTranLog.ulInvoiceNo = GetNewInvoiceNo();
	GetDateTime(glProcInfo.stTranLog.szDateTime);	// set default txn time

	glProcInfo.uiRecNo = glPosParams.tranRecordCount;

	glCommCfg = glPosParams.commConfig;
}


// 获得新的流水号
// generate a new trace NO.
ulong GetNewTraceNo(void)
{
	
	glSysCtrl.ulSTAN++;
	if (!(glSysCtrl.ulSTAN>0 && glSysCtrl.ulSTAN <= 999999L))
	{
		srand(DEVICE_GetTickCount());
		glSysCtrl.ulSTAN = rand() % 3000;
	}
	SaveSysCtrlBase();

	return (glSysCtrl.ulSTAN);
}

ulong GetNewBatchNo()
{
	glPosParams.batchNo++;
	if (!(glPosParams.batchNo>0 && glPosParams.batchNo <= 999999L))
	{
		glPosParams.batchNo = 1L;
	}

	return glPosParams.batchNo;
}

// 获得新的票据号
// generata a new invoice NO.
ulong GetNewInvoiceNo(void)
{
	glPosParams.sequenceNo++;
	if (!(glPosParams.sequenceNo>0 && glPosParams.sequenceNo <= 999999L))
	{
		glPosParams.sequenceNo = 1L;
	}
	SavePosParams();

	return (glPosParams.sequenceNo);
}

// 提示拔出IC卡
// prompt to remove IC card
void PromptRemoveICC(void)
{
	if (!ChkIfEmvEnable())
	{
		return;
	}

	IccClose(ICC_USER);
	if (IccDetect(ICC_USER) != 0)
	{
		// 如果IC卡已拔出，直接返回。
		// if removed IC card, return directly
		return;
	}

	// 显示并等待IC卡拔出
	// Display Prompt and wait until removed IC card
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS REMOVE CARD"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
	while (IccDetect(ICC_USER) == 0)
	{
		Beep();
		DelayMs(500);
	}
	Gui_ClearScr(); // Added by Kim_LinHB 2014-08-14 v1.01.0003 bug512
}

#ifdef ENABLE_EMV
// 删除过期CAPK
// erase expired CAPK
void EraseExpireCAPK(void)
{
	int			iRet, iCnt;
	EMV_CAPK	stCAPK;

	for (iCnt = 0; iCnt<MAX_KEY_NUM; iCnt++)
	{
		memset(&stCAPK, 0, sizeof(EMV_CAPK));
		iRet = EMVCheckCAPK(&stCAPK.KeyID, stCAPK.RID);
		if (iRet == EMV_OK)
		{
			break;
		}
		EMVDelCAPK(stCAPK.KeyID, stCAPK.RID);
	}
}
#endif

static int DispMainLogo(void)
{
#if defined(_Sxx_) || defined(_SP30_)
	uchar	*psLogoData;
	int		iWidth, iHeigh;

	psLogoData = NULL;
	GetNowDispLogo(&psLogoData);
	if (psLogoData != NULL)
	{
		iWidth = 0;
		iHeigh = 0;
		GetLogoWidthHeigh(psLogoData, &iWidth, &iHeigh);
		Gui_DrawLogo(psLogoData, ((128 - iWidth) / 2), 16);
		return 0;
	}
#elif defined(_Sxxx_) || defined(_Dxxx_)
	int iScrW, iScrH, iLogoW, iLogoH;
	// Modified by Kim_LinHB 2014-08-13 v1.01.0003
	iScrW = Gui_GetScrWidth();
	iScrH = Gui_GetScrHeight();

	if (GUI_OK == Gui_GetImageSize("LOGO.PNG", &iLogoW, &iLogoH) && iScrW >= iLogoW && iScrH >= iLogoH)
	{
		Gui_DrawImage("LOGO.PNG", (iScrW - iLogoW) * 100 / 2 / iScrW, (iScrH - iLogoH) * 100 / 2 / iScrH);
	}
	else
	{
		Gui_DrawImage("LOGO.PNG", 0, 25);
	}

	return 0;
#endif

	return -1;
}



// 显示刷卡/插卡界面
// Display "swipe/insert" prompt
void DispSwipeCard(uchar bShowLogo)
{

	uchar *pszStr = NULL;
	if (bShowLogo)
	{
		if (DispMainLogo() != 0)
		{
			bShowLogo = FALSE;
		}
	}

	if (ChkIfEmvEnable())
	{
		pszStr = _T_NOOP("INSERT CARD");
	}
	else
	{
		pszStr = _T_NOOP("SWIPE CARD");
	}

	if (bShowLogo)
	{
		GUI_TEXT_ATTR stTextAttr = gl_stCenterAttr;
#if !defined(_Sxxx_) && !defined(_Dxxx_)
		stTextAttr.eFontSize = GUI_FONT_SMALL;
		Gui_DrawText(pszStr, stTextAttr, 0, 85);
#else
		Gui_DrawText(pszStr, stTextAttr, 0, 80);
#endif
	}
	else
	{
		Gui_DrawText(pszStr, gl_stCenterAttr, 0, 50);
	}
}

void DispBlockFunc(void)
{
	Gui_ClearScr();
	PubBeepErr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TRANS NOT ALLOW"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
}

void DispProcess(void)
{
	Gui_ClearScr();
	Gui_ShowMsgBox(/*GetCurrTitle()*/NULL, gl_stTitleAttr, _T("PROCESSING...."), gl_stCenterAttrAlt, GUI_BUTTON_NONE, 0, NULL);
}

void DispMessage(char* message)
{
	Gui_ClearScr();
	Gui_ShowMsgBox(/*GetCurrTitle()*/NULL, gl_stTitleAttr, _T(message), gl_stCenterAttrAlt, GUI_BUTTON_NONE, 0, NULL);
}

void DispWait(void)
{
	Gui_ClearScr();
	Gui_ShowMsgBox(/*GetCurrTitle()*/NULL, gl_stTitleAttr, _T("PLEASE WAIT..."), gl_stCenterAttrAlt, GUI_BUTTON_NONE, 0, NULL);
}

void DispDial(void)
{
	//for display CLSS lights added by kevinliu 2016/01/28
	//	SetClssLightStatus(CLSSLIGHTSTATUS_DIALING);
	Gui_ClearScr();
	Gui_ShowMsgBox(/*GetCurrTitle()*/NULL, gl_stTitleAttr, _T("CONNECTING..."), gl_stCenterAttrAlt, GUI_BUTTON_NONE, 0, NULL);
}

void DispSend(void)
{
	//for display CLSS lights added by kevinliu 2016/01/28
	//	SetClssLightStatus(CLSSLIGHTSTATUS_SENDING);
	Gui_ClearScr();
	Gui_ShowMsgBox(/*GetCurrTitle()*/NULL, gl_stTitleAttr, _T("SENDING..."), gl_stCenterAttrAlt, GUI_BUTTON_NONE, 0, NULL);
}

void DispReceive(void)
{
	//for display CLSS lights added by kevinliu 2016/01/28
	static uchar ucClssLight = 0;

	if (ucClssLight) {
		//		SetClssLightStatus(CLSSLIGHTSTATUS_RECEIVING1);
		ucClssLight = 0;
	}
	else {
		//		SetClssLightStatus(CLSSLIGHTSTATUS_RECEIVING2);
		ucClssLight = 1;
	}
	Gui_ClearScr();
	Gui_ShowMsgBox(/*GetCurrTitle()*/NULL, gl_stTitleAttr, _T("RECEIVING..."), gl_stCenterAttrAlt, GUI_BUTTON_NONE, 0, NULL);
}

void DispPrinting(void)
{
	//for display CLSS lights added by kevinliu 2016/01/28
	//	SetClssLightStatus(CLSSLIGHTSTATUS_PRINTING);
	Gui_ClearScr();
	Gui_ShowMsgBox(/*GetCurrTitle()*/NULL, gl_stTitleAttr, _T("PRINTING..."), gl_stCenterAttrAlt, GUI_BUTTON_NONE, 0, NULL);
}

void DispClearOk(void)
{
	Gui_ClearScr();
	DispOperOk(_T("CLEARED"));
}

// 接收等待回调函数
// callback function for receiving
void DispWaitRspStatus(ushort uiLeftTime)
{
	uchar szBuff[10];
	sprintf(szBuff, "%-3u", uiLeftTime);
	Gui_DrawText(szBuff, gl_stCenterAttr, 75, 50); // Modified by Kim_LinHB 2014-8-5 v1.01.0001 bug497
}


// 输入功能号码
// for entering function id
int FunctionInput(void)
{
	logTrace(__func__);
	uchar szFuncNo[2 + 1];

	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 0;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 2;

	memset(szFuncNo, 0, sizeof(szFuncNo));
	Gui_ClearScr();
	if (GUI_OK != Gui_ShowInputBox(NULL, gl_stTitleAttr, _T("FUNCTION ?"), gl_stLeftAttr, szFuncNo,
		gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return -1;
	}

	return  atoi((char *)szFuncNo);
}

void SysHalt(void)
{
	unsigned char szBuff[200];
	sprintf(szBuff, "%s\n%s", _T("HALT FOR SAFETY "), _T("PLS RESTART POS "));
	Gui_ClearScr();
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, -1, NULL);
	while (1);
}

void SysHaltInfo(const void *pszDispInfo, ...)
{
	uchar		szBuff[1024 + 1];
	va_list		pVaList;

	if (pszDispInfo == NULL || *(uchar *)pszDispInfo == 0)
	{
		return;
	}

	sprintf(szBuff, "%s\n", _T("HALT FOR SAFETY "));

	va_start(pVaList, pszDispInfo);
	vsprintf((char*)szBuff + strlen(szBuff), (char*)pszDispInfo, pVaList);
	va_end(pVaList);

	Gui_ClearScr();
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, -1, NULL);
	while (1);
}

void DispMagReadErr(void)
{
	Gui_ClearScr();
	PubBeepErr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("READ CARD ERR."), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
}



void DispAccepted(void)
{
	Gui_ClearScr();
	PubBeepOk();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TXN ACCEPTED"), gl_stCenterAttr, GUI_BUTTON_OK, glSysParam.stEdcInfo.ucAcceptTimeout, NULL);
}

void DispErrMsg(const char *pFirstMsg, const char *pSecondMsg, short sTimeOutSec, ushort usOption)
{
	// DERR_BEEP     : error beep
	unsigned char szBuff[200];
	PubASSERT(pFirstMsg != NULL);

	if (pSecondMsg == NULL)
	{
		strcpy(szBuff, pFirstMsg);
	}
	else
	{
		sprintf(szBuff, "%s\n%s", pFirstMsg, pSecondMsg);
	}

	if (usOption & DERR_BEEP)
	{
		PubBeepErr();
	}

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, sTimeOutSec, NULL);
}

void PrintOne(void)
{
	static uchar sPrinterLogo[137] =
	{
		0x04,
		0x00,0x20,
		0x0,0xf8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,
		0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0xf8,

		0x00,0x20,
		0x0,0xff,0x0,0x0,0x0,0x0,0x80,0x40,0x20,0x70,0x4c,0x44,0x54,0x54,0x54,0x54,
		0x54,0x54,0x44,0x44,0x44,0x74,0x2c,0x20,0xa0,0xe0,0xe0,0x0,0x0,0x0,0x0,0xff,

		0x00,0x20,
		0x0,0xff,0x0,0x0,0x0,0x3f,0x21,0xe1,0x21,0x21,0x21,0x21,0x21,0x21,0x21,0x21,
		0x21,0x21,0x21,0x25,0x25,0xe1,0xe1,0x7f,0x1f,0xf,0x7,0x0,0x0,0x0,0x0,0xff,

		0x00,0x20,
		0x0,0xf,0x8,0x8,0x8,0x8,0x8,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,
		0x9,0x9,0x9,0x9,0x9,0x9,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0xf
	};

	Gui_DrawLogo(sPrinterLogo, 86, 30);
}

int SelectTransCurrency(void)
{
	// TODO: implement currency selection
	// let  glProcInfo.stTranLog.stTranCurrency = selected currency
	// ...

#ifdef ENABLE_EMV
	EMVGetParameter(&glEmvParam);
	memcpy(glEmvParam.TransCurrCode, glProcInfo.stTranLog.stTranCurrency.sCurrencyCode, 2);
	glEmvParam.TransCurrExp = glProcInfo.stTranLog.stTranCurrency.ucDecimal;
	EMVSetParameter(&glEmvParam);
	// Only in this trasaction, so DON'T back up
#endif

	return 0;
}

// 读取磁卡磁道及PAN
int ReadMagCardInfo(void)
{
	logTrace(__func__);
	int		iRet;

	glProcInfo.stTranLog.uiEntryMode = MODE_SWIPE_INPUT;
	MagRead(glProcInfo.szTrack1, glProcInfo.szTrack2, glProcInfo.szTrack3);

	iRet = GetPanFromTrack(glProcInfo.stTranLog.szPan, glProcInfo.stTranLog.szExpDate);
	if (iRet != 0)
	{
		DispMagReadErr();
		return ERR_NO_DISP;
	}
	Beep();

	return 0;
}

// 从磁道信息分析出卡号(PAN)
// get PAN from track
int GetPanFromTrack(uchar *pszPAN, uchar *pszExpDate)
{
	int		iPanLen;
	char	*p, *pszTemp;

	// 从2磁道开始到'＝'
	// read data from the start of track #2 to '-'
	if (strlen((char *)glProcInfo.szTrack2)>0)
	{
		pszTemp = (char *)glProcInfo.szTrack2;
	}
	else if (strlen((char *)glProcInfo.szTrack3)>0)
	{
		pszTemp = (char *)&glProcInfo.szTrack3[2];
	}
	else
	{	// 2、3磁道都没有
		// track #2 and #3 are not existed
		return ERR_SWIPECARD;
	}

	p = strchr((char *)pszTemp, '=');
	if (p == NULL)
	{
		return ERR_SWIPECARD;
	}
	iPanLen = p - pszTemp;
	if (iPanLen<13 || iPanLen>19)
	{
		return ERR_SWIPECARD;
	}

	sprintf((char *)pszPAN, "%.*s", iPanLen, pszTemp);
	if (pszTemp == (char *)glProcInfo.szTrack2)
	{
		sprintf((char *)pszExpDate, "%.4s", p + 1);
	}
	else
	{
		sprintf((char *)pszExpDate, "0000");
	}

	return 0;
}

// 检测磁道信息是否为IC卡磁道信息
// Check service code in track 2, whther it is 2 or 6
uchar IsChipCardSvcCode(const uchar *pszTrack2)
{
	char	*pszSeperator;

	if (*pszTrack2 == 0)
	{
		return FALSE;
	}

	pszSeperator = strchr((char *)pszTrack2, '=');
	if (pszSeperator == NULL)
	{
		return FALSE;
	}
	if ((pszSeperator[5] == '2') || (pszSeperator[5] == '6'))
	{
		return TRUE;
	}

	return FALSE;
}

// 校验卡号
// verify PAN NO.
int ValidPanNo(const uchar *pszPanNo)
{
	uchar	bFlag, ucTemp, ucResult;
	uchar	*pszTemp;

	// (2121 algorithm)
	bFlag = FALSE;
	pszTemp = (uchar *)&pszPanNo[strlen((char *)pszPanNo) - 1];
	ucResult = 0;
	while (pszTemp >= pszPanNo)
	{
		ucTemp = (*pszTemp--) & 0x0F;
		if (bFlag)    ucTemp *= 2;
		if (ucTemp>9) ucTemp -= 9;
		ucResult = (ucTemp + ucResult) % 10;
		bFlag = !bFlag;
	}

	if (ucResult != 0)
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("INVALID CARD"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
		return ERR_NO_DISP;
	}

	return 0;
}

// 检查卡的有效期(YYMM)
// verify the expiry (YYMM)
int ValidCardExpiry(void)
{
	uchar	szDateTime[14 + 1];
	ulong	ulCardYear, ulCardMonth;
	ulong	ulCurYear, ulCurMonth;
	uchar	ucInvalidFormat;

	glProcInfo.bExpiryError = FALSE;
	ucInvalidFormat = FALSE;

	ulCardYear = PubAsc2Long(glProcInfo.stTranLog.szExpDate, 2);
	ulCardYear += (ulCardYear>80) ? 1900 : 2000;
	ulCardMonth = PubAsc2Long(glProcInfo.stTranLog.szExpDate + 2, 2);

	GetDateTime(szDateTime);
	ulCurYear = PubAsc2Long(szDateTime, 4);
	ulCurMonth = PubAsc2Long(szDateTime + 4, 2);

	if (ulCardMonth>12 || ulCardMonth<1)
	{
		ucInvalidFormat = TRUE;
		glProcInfo.bExpiryError = TRUE;
	}
	if (//ulCardYear>ulCurYear+20 ||	// 是否需要判断有效期太长的卡?
		ulCardYear<ulCurYear ||
		(ulCurYear == ulCardYear && ulCurMonth>ulCardMonth))
	{
		glProcInfo.bExpiryError = TRUE;
	}

	if (glProcInfo.bExpiryError)
	{
		if (ChkIssuerOption(ISSUER_EN_EXPIRY) && ChkIssuerOption(ISSUER_CHECK_EXPIRY))
		{
			int nTimeout = 3;

			if (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) {
				nTimeout = 2;
			}
			else {
				nTimeout = 3;
				PubBeepErr();
			}

			Gui_ClearScr();
			if (ucInvalidFormat)
			{
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("ERR EXP. FORMAT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, nTimeout, NULL);
			}
			else
			{
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD EXPIRED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, nTimeout, NULL);
			}

			if (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT)
			{	// if EMV expired card, let core to continue process(based upon TACs/IACs)
				return 0;
			}
			else
			{
				return ERR_NO_DISP;
			}
		}
	}

	return 0;
}

// 获取终端当前时间,格式:YYYYMMDDhhmmss
// format : YYYYMMDDhhmmss
void GetDateTime(uchar *pszDateTime)
{
	uchar	sCurTime[7];

	GetTime(sCurTime);
	sprintf((char *)pszDateTime, "%02X%02X%02X%02X%02X%02X%02X",
		(sCurTime[0]>0x80 ? 0x19 : 0x20), sCurTime[0], sCurTime[1],
		sCurTime[2], sCurTime[3], sCurTime[4], sCurTime[5]);
}

int UpdateLocalTime(const uchar *pszNewYear, const uchar *pszNewDate, const uchar *pszNewTime)
{
	uchar	szLocalTime[14 + 1], sBuffer[16];

	if ((pszNewDate != 0) && (pszNewTime != 0))
	{
		if (pszNewYear == NULL)
		{
			memset(szLocalTime, 0, sizeof(szLocalTime));
			GetDateTime(szLocalTime);

			if ((memcmp(szLocalTime + 4, "12", 2) == 0) &&		// local month is DECEMBER
				(memcmp(pszNewDate, "01", 2) == 0))			// received month is JANUARY. local clock slower
			{
				PubAscInc(szLocalTime, 4);		// increase local year
			}
			if ((memcmp(szLocalTime + 4, "01", 2) == 0) &&		// local month is JANUARY
				(memcmp(pszNewDate, "12", 2) == 0))			// received month is DECEMBER. local clock faster
			{
				PubAscDec(szLocalTime, 4);		// increase local year
			}
		}
		else
		{
			memcpy(szLocalTime, pszNewYear, 4);
		}

		memcpy(szLocalTime + 4, pszNewDate, 4);	// MMDD
		memcpy(szLocalTime + 8, pszNewTime, 6);	// hhmmss

		memset(sBuffer, 0, sizeof(sBuffer));
		PubAsc2Bcd(szLocalTime + 2, 12, sBuffer);
		return SetTime(sBuffer);
	}

	return -1;
}

// 英文习惯的日期时间(16 bytes, eg: "OCT07,2006 11:22")
void GetEngTime(uchar *pszCurTime)
{
	uchar	Month[12][5] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
	uchar	sCurTime[7], ucMonth;

	GetTime(sCurTime);
	ucMonth = (sCurTime[1] >> 4) * 10 + (sCurTime[1] & 0x0F) - 1;
	if (strcmp(LANGCONFIG, "Arabic") == 0) //added by Kim_LinHB 2014-6-7
		sprintf((char *)pszCurTime, "%02X%02X,%s%02X %02X:%02X", (sCurTime[0]>0x80 ? 0x19 : 0x20), sCurTime[0],
			Month[ucMonth], sCurTime[2],
			sCurTime[3], sCurTime[4]);
	else
		sprintf((char *)pszCurTime, "%s%02X,%02X%02X %02X:%02X", Month[ucMonth],
			sCurTime[2], (sCurTime[0]>0x80 ? 0x19 : 0x20),
			sCurTime[0], sCurTime[3], sCurTime[4]);
	//	sprintf((char *)pszCurTime, "%.3s %02X,%02X  %02X:%02X", Month[ucMonth],
	//		sCurTime[2], sCurTime[0], sCurTime[3], sCurTime[4]);

}

// 转换YYYYMMDDhhmmss 到 OCT 07, 2006  11:22
// Convert from ... to ...
void Conv2EngTime(const uchar *pszDateTime, uchar *pszEngTime)
{
	uchar   Month[12][5] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
	uchar	ucMonth;

	ucMonth = (uchar)((PubAsc2Long(&pszDateTime[4], 2) - 1) % 12);
	sprintf((char *)pszEngTime, "%s %2.2s, %4.4s  %2.2s:%2.2s", Month[ucMonth],
		pszDateTime + 6, pszDateTime, pszDateTime + 8, pszDateTime + 10);
}

// 检查卡号,并确定收单行/发卡行(必须在读出卡号后调用)
// Check PAN, and determine Issuer/Acquirer.
int ValidCard(void)
{
	int		iRet;

	/*iRet = MatchCardTable(glProcInfo.stTranLog.szPan);
	if( iRet!=0 )
	{
	Gui_ClearScr();
	PubBeepErr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("UNSUPPORTED\nCARD"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	return ERR_NO_DISP;
	}*/


	iRet = ValidPanNo(glProcInfo.stTranLog.szPan);
	if (iRet != 0)
	{
		return iRet;
	}

	iRet = ValidCardExpiry();
	if (iRet != 0)
	{
		return iRet;
	}

	GetCardHolderName(glProcInfo.stTranLog.szHolderName);
	iRet = ConfirmPanInfo();
	if (iRet != 0)
	{
		CommOnHook(FALSE);
		return iRet;
	}

	/*iRet = GetSecurityCode();
	if( iRet!=0 )
	{
	return iRet;
	}*/

	return 0;
}

// 获得持卡人姓名(已经转换为打印格式)
// Read and convert holder name to printable format.
void GetCardHolderName(uchar *pszHolderName)
{

	int		iRet, iTagLen;
	uchar	szBuff[40];

	uchar	szTempName[40];

	*pszHolderName = 0;


	if (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT)
	{
		memset(szBuff, 0, sizeof(szBuff));
		iRet = EMVGetTLVData(0x5F20, szBuff, &iTagLen);
		if (iRet != EMV_OK)
		{
			return;
		}
		ConvertHolderName(szBuff, szTempName);
	}
	else	// other entry mode, just get it from track 1 data
	{
		GetHolderNameFromTrack1(szTempName);
	}

	sprintf((char *)pszHolderName, "%.26s", szTempName);
}

// get holder name form track 1, which is identified by '^'
void GetHolderNameFromTrack1(uchar *pszHolderName)
{
	char	*p, *q;
	uchar	szOrgName[50];
	int		iLen;

	*pszHolderName = 0;
	if (glProcInfo.szTrack1[0] == 0)
	{
		return;
	}

	p = strchr((char *)glProcInfo.szTrack1, '^');
	if (p == NULL)
	{
		return;
	}
	p++;
	iLen = strlen(p);

	q = strchr(p, '^');
	if (q != NULL)
	{
		iLen = MIN(q - p, iLen);
	}

	sprintf((char *)szOrgName, "%.*s", (int)MIN(sizeof(szOrgName) - 1, iLen), p);
	ConvertHolderName(szOrgName, pszHolderName);
}

// 转换ISO7813格式人名为打印格式
// "Amex/F D.Mr" --> "Mr F D Amex"
void ConvertHolderName(const uchar *pszOrgName, uchar *pszNormalName)
{
	char	*pszTitle, *pszMidName, *pszTemp, szBuff[50];

	sprintf((char *)pszNormalName, "%s", pszOrgName);
	if (*pszOrgName == 0)
	{
		return;
	}
	pszTemp = (char *)pszNormalName;

	pszMidName = strchr((char *)pszOrgName, '/');
	if (pszMidName == NULL)
	{
		return;
	}

	pszTitle = strrchr((char *)pszOrgName, '.');
	if (pszTitle != NULL)
	{
		sprintf(szBuff, "%s ", pszTitle + 1);
		PubTrimStr((uchar *)szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);

		sprintf(szBuff, "%.*s ", (int)(pszTitle - pszMidName - 1), pszMidName + 1);
		PubTrimStr((uchar *)szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);
	}
	else
	{
		sprintf(szBuff, "%s", pszMidName + 1);
		PubTrimStr((uchar *)szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);
	}
	sprintf(pszTemp, "%.*s", (int)(pszMidName - (char *)pszOrgName), pszOrgName);
}

// check whether really match to specific acquirer, due to customized conditions
uchar ChkAcqRestrictForCard(const uchar *pszPan)
{
	//if (ChkIfxxx())

	//...

	return TRUE;
}

// 根据卡号匹配卡表,并最终确定收单行(glCurAca)和发卡行(glCurIssuer)
// determine glCurAcq and glCurIssuer, due to ACQ-ISS-CARD matching table.
int MatchCardTable(const uchar *pszPAN)
{
	int			iRet;
	uchar		ucCnt, ucPanLen, ucAcqNum, ucLastAcqIdx;
	uchar		sPanHeader[5], sCardIndex[MAX_ACQ], sAcqMatchFlag[MAX_ACQ];
	CARD_TABLE	*pstCardTbl;

	memset(sCardIndex, 0, sizeof(sCardIndex));
	memset(sAcqMatchFlag, 0, sizeof(sAcqMatchFlag));

	// 建立收单行列表
	// create a list of matched acquirer.
	ucPanLen = strlen((char *)pszPAN);
	PubAsc2Bcd(pszPAN, 10, sPanHeader);

	return 0;
}

int MatchCardTableForInstalment(uchar ucIndex)
{
	uchar		ucCnt, ucPanLen;
	uchar		sPanHeader[5], sCardIndex[MAX_ACQ], sAcqMatchFlag[MAX_ACQ];
	CARD_TABLE	*pstCardTbl;

	memset(sCardIndex, 0, sizeof(sCardIndex));
	memset(sAcqMatchFlag, 0, sizeof(sAcqMatchFlag));

	// 建立收单行列表
	// create a list of matched acquirer.
	ucPanLen = strlen((char *)glProcInfo.stTranLog.szPan);
	PubAsc2Bcd(glProcInfo.stTranLog.szPan, 10, sPanHeader);


	return 0; // ERR_UNSUPPORT_CARD;
}


/************************************************************************
* 刷卡事件处理函数
* bCheckICC:    TRUE  检查2磁道的service code(对EMV终端有效)
*               FALSE 不检查
************************************************************************/
int SwipeCardProc(uchar bCheckICC)
{
	int		iRet;

	iRet = ReadMagCardInfo();
	if (iRet != 0)
	{
		return iRet;
	}

	// 交易不要求判断卡片类型或者为非EMV终端,直接返回
	// if don't require to check the card type or it is a non-EMV terminal, return directly
	if (!bCheckICC || !ChkIfEmvEnable())
	{
		return 0;
	}

	// EMV终端,继续检查
	// it is an EMV terminal, continue checking
	if (glProcInfo.bIsFallBack == TRUE)
	{
		if (IsChipCardSvcCode(glProcInfo.szTrack2))
		{	// fallback并且是IC卡,则返回成功
			// when it is a IC card and it needs fallback, return success
			glProcInfo.stTranLog.uiEntryMode = MODE_FALLBACK_SWIPE;
			return 0;
		}
		else
		{
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("NON EMV,RE-SWIPE"), gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);
			return ERR_NO_DISP;
		}
	}
	else if (IsChipCardSvcCode(glProcInfo.szTrack2))
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS INSERT CARD"), gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);
		return ERR_NEED_INSERT;
	}

	return 0;
}

#ifdef ENABLE_EMV
// ICC插卡事件处理函数
// Process insertion event.
int InsertCardProc(void)
{
	logTrace(__func__);
	int		iRet;
	uchar	szTotalAmt[12 + 1], sTemp[6];

	// 如果已经FALLBACK,忽略IC卡插卡操作
	// if did fallback, will ignore the coming inserting processing
	if (glProcInfo.bIsFallBack == TRUE)
	{
		return ERR_NEED_FALLBACK;
	}

	//!!!! deleted: it is fixed and not allowed to modify after GPO.
	//ModifyTermCapForPIN();

	glProcInfo.stTranLog.uiEntryMode = MODE_CHIP_INPUT;

	DispProcess();

#ifdef ENABLE_EMV
	InitTransEMVCfg();
#endif

	// 应用选择
	// EMV application selection. This is EMV kernel API
	iRet = EMVAppSelect(ICC_USER, glSysCtrl.ulSTAN);
	if (iRet == EMV_DATA_ERR || iRet == ICC_RESET_ERR || iRet == EMV_NO_APP ||
		iRet == ICC_CMD_ERR || iRet == EMV_RSP_ERR)
	{
		glProcInfo.bIsFallBack = TRUE;
		glProcInfo.iFallbackErrCode = iRet;
		return ERR_NEED_FALLBACK;
	}
	if (iRet == EMV_TIME_OUT || iRet == EMV_USER_CANCEL)
	{
		return ERR_USERCANCEL;
	}
	if (iRet != EMV_OK)
	{
		if (iRet == ICC_BLOCK)
			return iRet;
		return ERR_TRAN_FAIL;
	}

	// Clear log to avoid amount accumulation for floor limit checking
	iRet = EMVClearTransLog();

	// Read Track 2 and/or Pan
	iRet = GetEmvTrackData();
	if (iRet != 0)
	{
		return iRet;
	}

	DispProcess();

	iRet = ValidCard();
	if (iRet != 0)
	{
		return iRet;
	}
	UpdateEMVTranType();

	AppSetMckParam(0);

					  // 输入交易金额
					  // enter transaction amount
	if (
		glProcInfo.stTranLog.ucTranType != BALANCE &&
		glProcInfo.stTranLog.ucTranType != VOID)
	{
		iRet = GetAmount();
		if (iRet != 0)
		{
			return ERR_USERCANCEL;
		}
		PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount, 12, szTotalAmt);
		//PubAddHeadChars(szTotalAmt, 12, '0');  no need: already 12 digits
		PubAsc2Bcd(szTotalAmt, 12, sTemp);
		EMVSetTLVData(0x9F02, sTemp, 6);
		PubLong2Char((ulong)atol((char *)szTotalAmt), 4, sTemp);
		EMVSetTLVData(0x81, sTemp, 4);
	}

	DispProcess();

	// 卡片数据认证
	// Card data authentication
	//if (glProcInfo.stTranLog.ucTranType == SALE ||
	//	glProcInfo.stTranLog.ucTranType == AUTH ||
	//	glProcInfo.stTranLog.ucTranType == PREAUTH)
	//{
		iRet = EMVCardAuth();
		if (iRet != EMV_OK)
		{
			return ERR_TRAN_FAIL;
		}
	//}

	return 0;
}
#endif

#ifdef ENABLE_EMV
// 读取IC卡磁道信息/卡号信息等
// read track information from IC card
int GetEmvTrackData(void)
{
	int		iRet, iLength;
	uchar	sTemp[50], szCardNo[20 + 1];
	int		i, bReadTrack2, bReadPan;

	// 读取应用数据
	// read application data
	DispProcess();
	iRet = EMVReadAppData();
	if (iRet == EMV_TIME_OUT || iRet == EMV_USER_CANCEL)
	{
		return ERR_USERCANCEL;
	}
	if (iRet != EMV_OK)
	{
		return ERR_TRAN_FAIL;
	}

	// Read Track 2 Equivalent Data
	bReadTrack2 = FALSE;
	memset(sTemp, 0, sizeof(sTemp));
	iRet = EMVGetTLVData(0x57, sTemp, &iLength);
	if (iRet == EMV_OK)
	{
		bReadTrack2 = TRUE;
		PubBcd2Asc0(sTemp, iLength, glProcInfo.szTrack2);
		PubTrimTailChars(glProcInfo.szTrack2, 'F');	// erase padded 'F' chars
		for (i = 0; glProcInfo.szTrack2[i] != '\0'; i++)		// convert 'D' to '='
		{
			if (glProcInfo.szTrack2[i] == 'D')
			{
				glProcInfo.szTrack2[i] = '=';
				break;
			}
		}
	}

	// read PAN
	bReadPan = FALSE;
	memset(sTemp, 0, sizeof(sTemp));
	iRet = EMVGetTLVData(0x5A, sTemp, &iLength);
	if (iRet == EMV_OK)
	{
		PubBcd2Asc0(sTemp, iLength, szCardNo);
		PubTrimTailChars(szCardNo, 'F');		// erase padded 'F' chars
		if (bReadTrack2 && !MatchTrack2AndPan(glProcInfo.szTrack2, szCardNo))
		{
			// 如果Track2 & PAN 同时存在,则必须匹配
			// if Track2 & PAN exist at the same time, must match
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
			PromptRemoveICC();
			return ERR_NO_DISP;
			//			return ERR_TRAN_FAIL;
		}
		sprintf((char *)glProcInfo.stTranLog.szPan, "%.19s", szCardNo);
		bReadPan = TRUE;
	}
	else if (!bReadTrack2)
	{
		// 如果Track 2 和 PAN 都没有,则交易失败
		// no track #2, no PAN, no transaction
		return ERR_TRAN_FAIL;
	}
	if (!bReadPan)
	{
		// 没有读取PAN，但是有track 2
		// have not got PAN, but got track #2
		iRet = GetPanFromTrack(glProcInfo.stTranLog.szPan, glProcInfo.stTranLog.szExpDate);
		if (iRet != 0)
		{
			return ERR_TRAN_FAIL;
		}
	}

	// read PAN sequence number
	glProcInfo.stTranLog.bPanSeqOK = FALSE;
	iRet = EMVGetTLVData(0x5F34, &glProcInfo.stTranLog.ucPanSeqNo, &iLength);
	if (iRet == EMV_OK)
	{
		glProcInfo.stTranLog.bPanSeqOK = TRUE;
	}

	// read Application Expiration Date
	if (bReadPan)
	{
		memset(sTemp, 0, sizeof(sTemp));
		iRet = EMVGetTLVData(0x5F24, sTemp, &iLength);
		if (iRet == EMV_OK)
		{
			PubBcd2Asc0(sTemp, 2, glProcInfo.stTranLog.szExpDate);
		}
	}

	// application label
	EMVGetTLVData(0x50, glProcInfo.stTranLog.szAppLabel, &iLength);	// application label
																	// Issuer code table
	iRet = EMVGetTLVData(0x9F11, sTemp, &iLength);
	if ((iRet == 0) && (sTemp[0] == 0x01))
	{
		EMVGetTLVData(0x9F12, glProcInfo.stTranLog.szAppPreferName, &iLength);  // Application prefer name
	}
	// Application ID
	iRet = EMVGetTLVData(0x4F, glProcInfo.stTranLog.sAID, &iLength);	// AID
	if (iRet == EMV_OK)
	{
		glProcInfo.stTranLog.ucAidLen = (uchar)iLength;
	}

	// read cardholder name
	memset(sTemp, 0, sizeof(sTemp));
	iRet = EMVGetTLVData(0x5F20, sTemp, &iLength);
	if (iRet == EMV_OK)
	{
		sprintf((char *)glProcInfo.stTranLog.szHolderName, "%.20s", sTemp);
	}

	return 0;
}
#endif

// 比较2磁道信息和PAN是否一致(For ICC)
// Check whether track2 (from ICC) and PAN (from ICC) are same.
int MatchTrack2AndPan(const uchar *pszTrack2, const uchar *pszPan)
{
	int		i;
	uchar	szTemp[19 + 1];

	for (i = 0; i<19 && pszTrack2[i] != '\0'; i++)
	{
		if (pszTrack2[i] == '=')
		{
			break;
		}
		szTemp[i] = pszTrack2[i];
	}
	szTemp[i] = 0;

	if (strcmp((char *)szTemp, (char *)pszPan) == 0)
	{
		return TRUE;
	}

	return FALSE;
}

// 确认卡号信息
// confirm PAN information
// Modified by Kim_LinHB 2014-08-18 v1.01.0004 from msgbox to infopage
// Modified by Kim_LinHB 2014/9/11 v1.01.0008 add msgbox for Sxx bug517
int ConfirmPanInfo(void)
{
	//return 0;
	GUI_PAGE		stHexMsgPage;
	int		iIndex;
	uchar	szIssuerName[10 + 1];
	GUI_PAGELINE	stBuff[10];
	GUI_TEXT_ATTR stLeftAttr = gl_stLeftAttr;
	uchar			ucLines = 0;

	uchar bSSL = 0;
	uchar szSSL[120];

	if (0 == GetEnv("E_SSL", szSSL))
	{
		bSSL = atoi(szSSL);
	}

#ifdef _Sxx_  //fixed by Kim bug 813 815
	stLeftAttr.eFontSize = GUI_FONT_SMALL;
#endif

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLEASE WAIT..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
	PreDial();

	memset(stBuff, 0, sizeof(stBuff));

	iIndex = MatchCardBin(glProcInfo.stTranLog.szPan);
	if (iIndex >= 0)
	{
		sprintf(stBuff[ucLines].szLine, "%.16s", glSysParam.stIssuerNameList[iIndex].szEnglishName);
	}

	stBuff[ucLines++].stLineAttr = stLeftAttr;

	// Modified by Kim_LinHB 9/9/2014 v1.01.0007
	//	sprintf(stBuff[ucLines].szLine, "%.16s", glProcInfo.stTranLog.szHolderName);

	// Modified by Kevin Liu 20160621 bug813
	sprintf(stBuff[ucLines].szLine, "%.24s", glProcInfo.stTranLog.szHolderName);
	stBuff[ucLines++].stLineAttr = stLeftAttr;

	// Modified by Kim_LinHB 2014-8-5 v1.01.0001 bug489
	if (ChkIfDispMaskPan2())
	{
		MaskPan(glProcInfo.stTranLog.szPan, stBuff[ucLines].szLine);
	}
	else
	{
		strcpy(stBuff[ucLines].szLine, glProcInfo.stTranLog.szPan); // Modified by Kim_LinHB 2014-8-7 v1.01.0001 bug502-504
	}
	stBuff[ucLines++].stLineAttr = stLeftAttr;

	if (ChkIssuerOption(ISSUER_EN_EXPIRY))
	{
		sprintf(stBuff[ucLines].szLine, "EXP DATE:%2.2s/%2.2s", &glProcInfo.stTranLog.szExpDate[2], &glProcInfo.stTranLog.szExpDate[0]);
		stBuff[ucLines++].stLineAttr = stLeftAttr;
	}

	Gui_CreateInfoPage(_T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel), gl_stTitleAttr, stBuff, ucLines, &stHexMsgPage);
	// Display message

	if (bSSL && SxxSSLGetSocket() >= 0)
	{
		SxxSSLSingleProcess();
	}

	/*Gui_ClearScr();
	if (GUI_OK != Gui_ShowInfoPage(&stHexMsgPage, FALSE, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}*/


	// set EMV library parameters
	if (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT)
	{
		EMVGetParameter(&glEmvParam);

		if (strlen(glPosParams.merchantId) == 15) {
			memcpy(glEmvParam.MerchId, glPosParams.merchantId, 15);
		}

		if (strlen(glPosParams.terminalId) == 8) {
			memcpy(glEmvParam.TermId, glPosParams.terminalId, 8);
		}
		EMVSetParameter(&glEmvParam);
		// Only in this trasaction, so DON'T back up
	}

	return 0;
}

// RFU for HK
int MatchCardBin(const uchar *pszPAN)
{
	uchar	szStartNo[20 + 1], szEndNo[20 + 1];
	ushort	i;

	for (i = 0; i<glSysParam.uiCardBinNum; i++)
	{
		PubBcd2Asc(glSysParam.stCardBinTable[i].sStartNo, 10, szStartNo);
		PubBcd2Asc(glSysParam.stCardBinTable[i].sEndNo, 10, szEndNo);
		if (memcmp(pszPAN, szStartNo, glSysParam.stCardBinTable[i].ucMatchLen) >= 0 &&
			memcmp(pszPAN, szEndNo, glSysParam.stCardBinTable[i].ucMatchLen) <= 0)
		{
			return (int)glSysParam.stCardBinTable[i].ucIssuerIndex;
		}
	}

	return -1;
}

void ConvIssuerName(const uchar *pszOrgName, uchar *pszOutName)
{
	char	*p;

	sprintf((char *)pszOutName, "%.10s", pszOrgName);
	p = strchr((char *)pszOutName, '_');
	if (p != NULL)
	{
		*p = 0;
	}
}

// input CVV2 or 4DBC
int GetSecurityCode(void)
{
	GUI_INPUTBOX_ATTR stInputAttr;

	if (!ChkIfNeedSecurityCode())
	{
		return 0;
	}

	if (!ChkIfAmex())
	{
		Gui_ClearScr();
		if (GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, "ENTER\nSECURITY CODE?", gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL))
		{
			return 0;
		}
	}

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = stInputAttr.nMaxLen = ChkIfAmex() ? 4 : 3;

	Gui_ClearScr();
	if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, ChkIfAmex() ? _T("ENTER 4DBC") : _T("SECURITY CODE"),
		gl_stLeftAttr, glProcInfo.szSecurityCode, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	glProcInfo.stTranLog.uiEntryMode |= MODE_SECURITYCODE;
	return 0;
}

// 根据参数进行,刷卡/插卡/输入卡号
// Accept different entry mode due to input:ucMode
// ucMode: bit 8: 1=skipping check track 2 service code, 0=check
//         bit 7: 1=fallback swipe
//         bit 6: 1=skip detect ICC
int GetCard(uchar ucMode)
{
	int		iRet, iEventID;
	uchar	bCheckICC, ucKey;

	if ((glProcInfo.stTranLog.uiEntryMode & 0x0F) != MODE_NO_INPUT)
	{
		return 0;
	}

	if (ucMode & FALLBACK_SWIPE)
	{
		ucMode &= ~(SKIP_CHECK_ICC | CARD_INSERTED);	// clear bit 8, force to check service code
	}

	bCheckICC = !(ucMode & SKIP_CHECK_ICC);
	while (1)
	{
		iEventID = DetectCardEvent(ucMode);
		if (iEventID == CARD_KEYIN)
		{
			ucKey = GetLastKey();
			if (ucKey == KEYCANCEL)
			{
				return ERR_USERCANCEL;
			}
			if ((ucMode & CARD_KEYIN) && ucKey >= '0' && ucKey <= '9')
			{
				return ManualInputPan(ucKey);
			}
		}
		else if (iEventID == CARD_SWIPED)
		{
			iRet = SwipeCardProc(bCheckICC);
			if (iRet == 0)
			{
				return ValidCard();
			}
			else if (iRet == ERR_SWIPECARD)
			{
				DispMagReadErr();
			}
			else if (iRet == ERR_NEED_INSERT)
			{
				// 是芯片卡
				// IC card
				if (!(ucMode & CARD_INSERTED))
				{
					// 本身交易不允许插卡
					// not allowed IC card
					return iRet;
				}
				// 去掉刷卡检查
				// remove flag for check swiping
				ucMode &= ~CARD_SWIPED;
			}
			else
			{
				return iRet;
			}
		}
		else if (iEventID == CARD_INSERTED)
		{
			iRet = InsertCardProc();
			if (iRet == 0)
			{
				return 0;
			}
			else if (iRet == ERR_NEED_FALLBACK)
			{
				DispFallBackPrompt();
				PromptRemoveICC();
				ucMode = CARD_SWIPED | FALLBACK_SWIPE;	// Now we don't support fallback to manual-PAN-entry
			}
			else if (iRet == ERR_TRAN_FAIL)
			{
				Gui_ClearScr();
				PubBeepErr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("NOT ACCEPTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
				PromptRemoveICC();
				return ERR_NO_DISP;
			}
			else if (iRet == ICC_BLOCK)
			{
				Gui_ClearScr();
				PubBeepErr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD BLOCKED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
				PromptRemoveICC();
				return ERR_NO_DISP;
			}
			else
			{
				return iRet;
			}
		}
		else  if (iEventID == CARD_TAPPED) {
			logd(("Card tapped"));
			glProcInfo.stTranLog.uiEntryMode = MODE_CONTACTLESS;
			return 0;
		}
	}

	return 0;
}

static uchar sg_ucMode = NO_SWIPE_INSERT;
static int inputEvent(gui_callbacktype_t type, void *data, int *dataLen) {
	int iRet = -1;
	if (data && *(int *)data != 0)
	{
		// 有按键事件
		// pressed a key
		sg_ucMode = CARD_KEYIN;
		return GUI_RETURNFROMCALLBACK;
	}
	if ((sg_ucMode & CARD_SWIPED) && (MagSwiped() == 0))
	{
		// 有刷卡事件
		// swiped a card
		sg_ucMode = CARD_SWIPED;
		return GUI_RETURNFROMCALLBACK;
	}
	if ((sg_ucMode & CARD_INSERTED) && ChkIfEmvEnable())
	{
		if (sg_ucMode & SKIP_DETECT_ICC)
		{
			// 有插入IC卡事件
			// inserted a card
			sg_ucMode = CARD_INSERTED;
			return GUI_RETURNFROMCALLBACK;
		}
		else if (IccDetect(ICC_USER) == 0)
		{
			// 有插入IC卡事件
			// inserted an IC card
			sg_ucMode = CARD_INSERTED;
			return GUI_RETURNFROMCALLBACK;
		}
	}

	if ((sg_ucMode & CARD_TAPPED) && ChkIfEmvEnable()) {
		
		iRet = PiccDetect(0, NULL, NULL, NULL, NULL);
		//			OsLog(LOG_INFO, "%s - %d PiccDetect RET = %d", __FUNCTION__, __LINE__, iRet);
		if (iRet == 0) {
			sg_ucMode = CARD_TAPPED;
			return GUI_RETURNFROMCALLBACK;
		}
		else if (iRet == 3 || iRet == 5 || iRet == 6)
		{
			DelayMs(100);
		}
		else if (iRet == 4)
		{
			SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TOO MANY CARD"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
			return ERR_NO_DISP;
		}
		else// if(iRet == 1 || iRet==2)
		{
			return ERR_TRAN_FAIL;
		}
	}
	return GUI_OK;
}

// 用户输入事件检测(按键/刷卡/插卡)
// detect event from user(press key/ swipe card/ insert card)
int DetectCardEvent(uchar ucMode)
{
	unsigned char szPrompt[100] = { 0 };
	int key = 0;
	//磁头上电、打开、清缓冲
	// reset magnetic module
	if (ucMode & CARD_SWIPED)
	{
		MagClose();
		MagOpen();
		MagReset();
	}

	if (ucMode && CARD_INSERTED) {
		EMVCoreInit();
	}

	if (ucMode & CARD_TAPPED) {
		ClssTransInit();
		vAppInitSchemeId();

		if (ClssPreProcTxnParam())
		{
			return ERR_TRAN_FAIL;
		}

		if (PiccOpen() != 0)
		{
			SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("OPEN PICC ERR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
			return ERR_NO_DISP;
		}
	}

	Gui_ClearScr(); // Added by Kim_LinHB 2014-8-12 1.01.0003 bug513

	if (ucMode & FALLBACK_SWIPE)
	{
		sprintf(szPrompt, "%s\n%s", _T("PLS SWIPE CARD"), _T("FALL BACK"));
	}
	else if ((ucMode & CARD_TAPPED) && (ucMode & CARD_INSERTED)) {
		if (ChkIfEmvEnable())
		{
			sprintf(szPrompt, "%s", _T("INSERT/TAP CARD"));
		}
	}
	else if ((ucMode & CARD_SWIPED) && (ucMode & CARD_INSERTED))
	{
		//removed by Kevin Liu 20160613 bug848
		if (ChkIfEmvEnable())
		{
			sprintf(szPrompt, "%s", _T("INSERT CARD"));
		}
		else
		{
			sprintf(szPrompt, "%s", _T("SWIPE CARD"));
		}
	}
	else if ((ucMode & CARD_INSERTED))
	{
		if (!(ucMode & SKIP_DETECT_ICC))
		{
			sprintf(szPrompt, "%s", _T("INSERT CARD"));
		}
	}
	else
	{
		sprintf(szPrompt, "%s", _T("SWIPE CARD"));
	}

	sg_ucMode = ucMode;
	Gui_RegCallback(GUI_CALLBACK_LISTEN_EVENT, inputEvent);
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stCenterAttr, GUI_BUTTON_CANCEL, -1, &key);
	Gui_RegCallback(GUI_CALLBACK_LISTEN_EVENT, NULL);
	if (key != NOKEY && key != 0) {
		sg_ucMode = CARD_KEYIN;
		SetLastKey(key);
	}
	return sg_ucMode;
}

// 显示Fallback提示界面
// display fallback prompt
void DispFallBackPrompt(void)
{
	uint	iCnt;
	unsigned char szBuff[100], szBuff2[100];

	sprintf(szBuff, "%s\n   %.*s", _T("PLS SWIPE CARD"), strlen(_T("PULL OUT")), "         ");
	sprintf(szBuff2, "%s\n   %s", _T("PLS SWIPE CARD"), _T("PULL OUT"));

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
	iCnt = 0;
	while (IccDetect(0) == 0)
	{
		iCnt++;
		if (iCnt>4)
		{
			//Gui_ClearScr(); // Removed by Kim_LinHB 2014-08-13 v1.01.0003 bug512
			Beep();
			if (iCnt % 2)
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff2, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
			else
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		}
		DelayMs(500);
	}
}

// 输入金额及小费
// Get amount and tips.
int GetAmount(void)
{
	int		iRet;
	uchar	szTotalAmt[12 + 1];

	uchar bSSL = 0;
	uchar szSSL[120];

	if (0 == GetEnv("E_SSL", szSSL))
	{
		bSSL = atoi(szSSL);
	}

	if (glProcInfo.stTranLog.szAmount[0] != 0)
	{
		return 0;
	}

	while (1)
	{
		if (bSSL && SxxSSLGetSocket() >= 0)
		{
			SxxSSLSingleProcess();
		}

		iRet = InputAmount(AMOUNT);
		if (iRet != 0)
		{
			return iRet;
		}

		if (bSSL && SxxSSLGetSocket() >= 0)
		{
			SxxSSLSingleProcess();
		}

		iRet = GetTipAmount();
		if (iRet != 0)
		{
			return iRet;
		}

		memcpy(glProcInfo.stTranLog.szInitialAmount, glProcInfo.stTranLog.szAmount, sizeof(glProcInfo.stTranLog.szInitialAmount));
		PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount, 12, szTotalAmt);
		if (!ValidBigAmount(szTotalAmt))
		{
			// Modified by Kim_LinHB 2014-8-12 v1.01.0003 bug511
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("EXCEED LIMIT"), gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);
			memset(glProcInfo.stTranLog.szAmount, 0, sizeof(glProcInfo.stTranLog.szAmount));
			memset(glProcInfo.stTranLog.szOtherAmount, 0, sizeof(glProcInfo.stTranLog.szOtherAmount));
			continue;
		}

		if (bSSL && SxxSSLGetSocket() >= 0)
		{
			SxxSSLSingleProcess();
		}

		if (ConfirmAmount(NULL, szTotalAmt))
		{
			break;
		}
	}

	if (!AllowDuplicateTran())
	{
		return ERR_USERCANCEL;
	}

	return 0;
}

// 把不含小数点，不含ignore digit的数字串转换为ISO8583 bit4格式的12位ASCII数字串
// convert the format of transaction amount
void AmtConvToBit4Format(uchar *pszStringInOut, uchar ucIgnoreDigit)
{
	uint	uiLen;

	if (pszStringInOut == NULL)
	{
		return;
	}

	uiLen = (uint)strlen((char *)pszStringInOut);
	if (uiLen >= (uint)(12 - ucIgnoreDigit))
	{
		return;
	}

	// 前补0
	// left pad with 0
	memmove(pszStringInOut + 12 - uiLen - ucIgnoreDigit, pszStringInOut, uiLen + 1);
	memset(pszStringInOut, '0', 12 - uiLen - ucIgnoreDigit);

	// 后补ignore digit个0
	// right pad with 0, number of 0 equals to ucIgnoreDigit
	for (uiLen = 0; uiLen<ucIgnoreDigit; uiLen++)
	{
		strcat((char *)pszStringInOut, "0");
	}
}

// Use local currency to call PubConvAmount
void App_ConvAmountLocal(const uchar *pszIn, uchar *pszOut, uchar ucMisc)
{
	uchar	szBuff[12];

	memset(szBuff, 0, sizeof(szBuff));
	strcpy((char *)szBuff, (char *)glPosParams.currency.szName);
	if ((glSysParam.stEdcInfo.ucCurrencySymbol != ' ')
		&& (glSysParam.stEdcInfo.ucCurrencySymbol != 0))
	{
		szBuff[strlen(szBuff)] = glSysParam.stEdcInfo.ucCurrencySymbol;
	}

	PubConvAmount(szBuff, pszIn,
		glPosParams.currency.ucDecimal,
		glPosParams.currency.ucIgnoreDigit,
		pszOut, ucMisc);
}

// Use transaction currency to call PubConvAmount (MAYBE different form local currency)
void App_ConvAmountTran(const uchar *pszIn, uchar *pszOut, uchar ucMisc)
{
	uchar	szBuff[12];

	memset(szBuff, 0, sizeof(szBuff));
	strcpy((char *)szBuff, (char *)glPosParams.currency.szName);

	PubConvAmount(szBuff, pszIn,
		glPosParams.currency.ucDecimal,
		glPosParams.currency.ucIgnoreDigit,
		pszOut, ucMisc);
}

static uchar sg_ucAmtType = AMOUNT;
int DisplayInputAmount(gui_callbacktype_t type, void *data, int *dataLen)
{
	if (GUI_CALLBACK_UPDATE_TEXT == type && data && dataLen) {
		//modified by KevinLiu 20160811
		unsigned char ucDeciPos = 0;
		unsigned char szPrefix[5 + 1] = { 0 };
		unsigned char isMisc = 0;

		struct _Gui_Callback_Text *amount = (struct _Gui_Callback_Text *)data;

		if (0 == amount->pStr[0] || IsNumStr(amount->pStr))
		{
			ucDeciPos = glPosParams.currency.ucDecimal;

			//added by KevinLiu 20160811
			memset(szPrefix, 0, sizeof(szPrefix));

			if (glProcInfo.stTranLog.ucTranType == REFUND || sg_ucAmtType == RFDAMOUNT)
			{
				isMisc = GA_NEGATIVE;
			}
			PubConvAmount(szPrefix, (unsigned char *)amount->pStr, ucDeciPos, 0, amount->pStr, isMisc);
			*dataLen = strlen(data);
		}
	}
	return GUI_OK;
}

// 输入交易金额
// enter transaction amount
int InputAmount(uchar ucAmtType)
{
	uchar	*pszAmt;
	uchar	szBuff[100];
	int iRet;

	GUI_INPUTBOX_ATTR stInputAttr;

	PubASSERT(ucAmtType == AMOUNT || ucAmtType == RFDAMOUNT ||
		ucAmtType == ORGAMOUNT || ucAmtType == CASHBACKAMOUNT);

	//Modified by KevinLiu 20160720 bug MEDC-12
	memset(szBuff, 0, sizeof(szBuff));
	switch (ucAmtType)
	{
	case AMOUNT:
		if (ChkIfNeedTip())
		{
			strcpy(szBuff, _T("AMOUNT"));
		}
		else
		{
			strcpy(szBuff, _T("AMOUNT"));
		}
		break;

	case RFDAMOUNT:	// RFU
		strcpy(szBuff, _T("AMOUNT"));
		break;

	case ORGAMOUNT:	// RFU
		strcpy(szBuff, _T("AMOUNT"));
		break;

	case CASHBACKAMOUNT:
		strcpy(szBuff, _T("TIP"));
		break;
	}
	strcat(szBuff, _T(" ("));
	strncat(szBuff, glPosParams.currency.szName, 3);
	strcat(szBuff, _T(")"));

	sg_ucAmtType = ucAmtType;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 1;

	// 	sprintf((char *)szCurrName, "%.3s%1.1s", glSysParam.stEdcInfo.szCurrencyName, &glSysParam.stEdcInfo.ucCurrencySymbol);
	stInputAttr.nMinLen = (ucAmtType == CASHBACKAMOUNT) ? 0 : 1;
	stInputAttr.nMaxLen = MIN(glSysParam.stEdcInfo.ucTranAmtLen, 10);

	Gui_RegCallback(GUI_CALLBACK_UPDATE_TEXT, DisplayInputAmount);

	while (1)
	{
		if (ucAmtType == CASHBACKAMOUNT)
		{
			pszAmt = glProcInfo.stTranLog.szOtherAmount;
		}
		else
		{
			pszAmt = glProcInfo.stTranLog.szAmount;
		}

		Gui_ClearScr();
		iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stLeftAttr, pszAmt,
			gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
		if (GUI_OK != iRet)
		{
			sg_ucAmtType = AMOUNT;
			Gui_RegCallback(GUI_CALLBACK_UPDATE_TEXT, NULL);
			return ERR_USERCANCEL;
		}

		if (ucAmtType != CASHBACKAMOUNT) // Added by Kim_LinHB 9/9/2014 v1.01.0007 bug520
		{
			if (0 == atoi(pszAmt))
			{
				Beep();
				continue;
			}
		}
		// Use transaction currency to do conversion
		AmtConvToBit4Format(pszAmt, glProcInfo.stTranLog.stTranCurrency.ucIgnoreDigit);
		sg_ucAmtType = AMOUNT;
		Gui_RegCallback(GUI_CALLBACK_UPDATE_TEXT, NULL);
		return 0;
	}
	return 0;
}

// 输入TIP金额
// enter tip
int GetTipAmount(void)
{
	int		iRet;
	uchar	szTotalAmt[12 + 1];

	if (!ChkIfNeedTip())
	{
		return 0;
	}

	while (1)
	{
		iRet = InputAmount(CASHBACKAMOUNT);
		if (iRet != 0)
		{
			return iRet;
		}

		PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount, 12, szTotalAmt);
		if (ValidAdjustAmount(glProcInfo.stTranLog.szAmount, szTotalAmt))
		{
			break;
		}
	}

	return 0;
}

// 检查调整金额是否合法(TRUE: 合法, FALSE: 非法)
// 金额必须为12数字
// check if the amount adjusted is valid, amount must be a 12 digits string number
uchar ValidAdjustAmount(const uchar *pszBaseAmt, uchar *pszTotalAmt)
{
	uchar	szMaxAmt[15 + 1], szAdjRate[3 + 1];


	if (memcmp(pszTotalAmt, pszBaseAmt, 12)<0)
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(_T("ADJUST"), gl_stTitleAttr, _T("AMOUNT ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
		return FALSE;
	}

	if (memcmp(pszTotalAmt, pszBaseAmt, 12) == 0)
	{
		return TRUE;
	}

	sprintf((char *)szAdjRate, "%ld", (ulong)(100));
	PubAscMul(pszBaseAmt, szAdjRate, szMaxAmt);
	PubAddHeadChars(szMaxAmt, 15, '0');

	// 最后两位为小数,不进行比较
	// ignore the last 2 digits
	if (memcmp(pszTotalAmt, &szMaxAmt[1], 12)>0)
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(_T("ADJUST"), gl_stTitleAttr, _T("OVER LIMIT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
		return FALSE;
	}

	return TRUE;
}

// amount should be less than 4294967296 (max of unsigned long)
uchar ValidBigAmount(const uchar *pszAmount)
{
	int	iLen;

	iLen = strlen(pszAmount);
	if (iLen<10)
	{
		return TRUE;
	}
	if (iLen>12)
	{
		return FALSE;
	}
	if (PubAsc2Long(pszAmount, iLen - 3)>4294966)
	{
		return FALSE;
	}
	return TRUE;
}

// 确认金额界面处理
// confirm transaction amount
// Modified by Kim_LinHB 2014-08-13 v1.01.0003
uchar ConfirmAmount(const char *pszDesignation, uchar *pszAmount)
{
	unsigned char szDispBuff[200];
	GUI_TEXT_ATTR stTextAttr = gl_stCenterAttr;

	GetDispAmount(pszAmount, szDispBuff);
	strcat(szDispBuff, "\n");
	strcat(szDispBuff, _T("CORRECT ?")); // Modified by Kim_LinHB 2014-8-5 v1.01.0001 bug495

	Gui_ClearScr();

#ifndef _Dxxx_
	//stTextAttr.eFontSize = GUI_FONT_SMALL;
#endif
	if (GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szDispBuff, stTextAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL))
	{
		return FALSE;
	}
	return TRUE;
}

// 在指定行显示格式化的金额信息。注意金额是bit4格式，即可能含有ignore digit
// Get formatted amount
void GetDispAmount(uchar *pszAmount, uchar *pszDispBuff)
{
	uchar	ucFlag, szOutAmt[30];

	ucFlag = 0;
	if (glProcInfo.stTranLog.ucTranType == VOID || glProcInfo.stTranLog.ucTranType == REFUND ||
		*pszAmount == 'D')
	{
		ucFlag |= GA_NEGATIVE;
		if (*pszAmount == 'D')
		{
			*pszAmount = '0';
		}
	}

	App_ConvAmountTran(pszAmount, szOutAmt, ucFlag);
	if (pszDispBuff) {
		sprintf(pszDispBuff, "%.21s", szOutAmt);
	}
}

// 从UsrMsg取得卡号
// Format: "CARDNO=4333884001356283"
int GetManualPanFromMsg(const void *pszUsrMsg)
{
	sprintf(glProcInfo.stTranLog.szPan, "%.19s", (char *)pszUsrMsg);
	if (!IsNumStr(glProcInfo.stTranLog.szPan))
	{
		return ERR_NO_DISP;
	}

	return VerifyManualPan();
}

// 手工输入PAN及其相关信息
// enter PAN manually
int ManualInputPan(uchar ucInitChar)
{
	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.nMinLen = 13;
	stInputAttr.nMaxLen = LEN_PAN;

	memset(glProcInfo.stTranLog.szPan, 0, sizeof(glProcInfo.stTranLog.szPan));
	if (ucInitChar >= '0' && ucInitChar <= '9')
	{
		glProcInfo.stTranLog.szPan[0] = ucInitChar;
	}
	else {
		return ERR_NO_DISP;
	}
	Gui_ClearScr();
	if (GUI_OK != Gui_ShowInputBox(NULL, gl_stTitleAttr, _T("ENTER ACCOUNT NO"), gl_stLeftAttr,
		glProcInfo.stTranLog.szPan, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_NO_DISP;
	}

	if (glProcInfo.bIsFallBack == TRUE)
	{
		glProcInfo.stTranLog.uiEntryMode = MODE_FALLBACK_MANUAL;
	}

	return VerifyManualPan();
}

int VerifyManualPan(void)
{
	int		iRet;

	glProcInfo.stTranLog.uiEntryMode = MODE_MANUAL_INPUT;

	iRet = MatchCardTable(glProcInfo.stTranLog.szPan);
	if (iRet != 0)
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("UNSUPPORT CARD"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
		return ERR_NO_DISP;
	}

	if (!ChkIssuerOption(ISSUER_EN_MANUAL))
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("NOT ALLOW MANUL"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
		return ERR_NO_DISP;
	}

	iRet = ValidPanNo(glProcInfo.stTranLog.szPan);
	if (iRet != 0)
	{
		return iRet;
	}

	iRet = GetExpiry();
	if (iRet != 0)
	{
		return iRet;
	}

	if (!ChkEdcOption(EDC_NOT_MANUAL_PWD))
	{
		if (PasswordMerchant() != 0)
		{
			return ERR_USERCANCEL;
		}
	}

	iRet = ConfirmPanInfo();
	if (iRet != 0)
	{
		CommOnHook(FALSE);
		return iRet;
	}

	iRet = GetSecurityCode();
	if (iRet != 0)
	{
		return iRet;
	}

	return 0;
}

// 输入有效期
// enter expiry date
int GetExpiry(void)
{
	int		iRet;
	uchar	szExpDate[4 + 1];

	GUI_INPUTBOX_ATTR stInputAttr;

	if (!ChkIssuerOption(ISSUER_EN_EXPIRY))
	{
		sprintf((char *)glProcInfo.stTranLog.szExpDate, "1111");
		return 0;
	}

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.nMinLen = 4;
	stInputAttr.nMaxLen = 4;

	memset(szExpDate, 0, sizeof(szExpDate)); // Added by Kim_LinHB 2014-8-5 v1.01.0001 bug488

	while (1)
	{
		Gui_ClearScr();
		if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("EXP DATE: MMYY"), gl_stLeftAttr,
			szExpDate, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}

		sprintf((char *)glProcInfo.stTranLog.szExpDate, "%.2s%.2s", &szExpDate[2], szExpDate);
		iRet = ValidCardExpiry();
		if (iRet == 0)
		{
			break;
		}
	}

	return 0;
}

// 输入商品描述信息
int GetDescriptor(void)
{
	uchar	ucCnt, ucTotal = 0, bInputDesc = FALSE;
	GUI_INPUTBOX_ATTR stInputAttr;
	unsigned char szBuff[10];
	unsigned char szDesc[200];

	if (!ChkIssuerOption(ISSUER_EN_DISCRIPTOR))
	{
		return 0;
	}

	if (glSysParam.ucDescNum == 0)
	{
		return 0;
	}
	if (glSysParam.ucDescNum == 1)
	{
		glProcInfo.stTranLog.szDescriptor[0] = '0';
		glProcInfo.stTranLog.ucDescTotal = 1;
		return 0;
	}

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = MAX_GET_DESC;

	memset(szBuff, 0, sizeof(szBuff));
	memset(szDesc, 0, sizeof(szDesc));
	Gui_ClearScr();
	if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("PRODUCT CODE?"), gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	for (ucCnt = 0; ucCnt < strlen(szBuff); ++ucCnt)
	{
		if (strchr((char *)glProcInfo.stTranLog.szDescriptor, szBuff[ucCnt]) == NULL &&
			szBuff[ucCnt]<glSysParam.ucDescNum + '0')
		{
			glProcInfo.stTranLog.szDescriptor[ucTotal] = szBuff[ucCnt];
			ucTotal++;
			bInputDesc = TRUE;
		}
	}

	glProcInfo.stTranLog.ucDescTotal = ucTotal;
	if (bInputDesc)
	{
		GUI_TEXT_ATTR stDescAttr = gl_stCenterAttr;
		uchar ucDesc;
		for (ucCnt = 0; ucCnt<ucTotal; ucCnt++)
		{
			ucDesc = glProcInfo.stTranLog.szDescriptor[ucCnt] - '0';
			sprintf(szDesc + strlen(szDesc), "%.21s\n", glSysParam.stDescList[ucDesc].szText);
		}
		if (strlen(szDesc) > ucTotal)
			szDesc[strlen(szDesc) - 1] = 0; // remove the last '\n'
		else
			szDesc[0] = '\n';

#ifdef _Dxxx_
		stDescAttr.eFontSize = GUI_FONT_NORMAL;
#else
		stDescAttr.eFontSize = GUI_FONT_SMALL;
#endif
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szDesc, stDescAttr, GUI_BUTTON_OK, USER_OPER_TIMEOUT, NULL);
	}
	return 0;
}

// 输入附加信息
// enter additional message.
int GetAddlPrompt(void)
{
	GUI_INPUTBOX_ATTR stInputAttr;

	if (!ChkAcqOption(ACQ_ADDTIONAL_PROMPT) && !ChkAcqOption(ACQ_AIR_TICKET))
	{
		return 0;
	}

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 16;

	Gui_ClearScr();
	if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, glSysParam.stEdcInfo.szAddlPrompt, gl_stLeftAttr,
		glProcInfo.stTranLog.szAddlPrompt, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	return 0;
}

// 输入PIN
// ucFlag: bit 8, ICC online PIN
int GetPIN(uchar ucFlag)
{
	int		iRet;
	uchar	szPAN[16 + 1], szTotalAmt[12 + 1], szDispAmt[100];
	uchar	ucPinKeyID;
	uchar	ucAmtFlag, szBuff[25];
	GUI_TEXT_ATTR st_Small_Left = gl_stCenterAttr;
#ifndef _Dxxx_
	//st_Small_Left.eFontSize = GUI_FONT_SMALL;  // Modified by Kim_LinHB 2014/9/16 v1.01.0009 bug509
#endif

											   // 非EMV PIN的模式下，如果是chip则直接返回，不是chip则检查ISSUER
											   // in non-EMV-PIN mode, if it is chip transaction then return directly
	if (!(ucFlag & GETPIN_EMV))
	{
		if ((glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT))
		{
			return 0;
		}
		else if (!ChkIssuerOption(ISSUER_EN_PIN))
		{
			return 0;
		}
	}

	//!!!! : 预留扩展：ACQUIRER可定义自己使用的ID
	// extended, enable to use acquirer's own ID
	ucPinKeyID = DEF_PIN_KEY_ID;

	iRet = ExtractPAN(glProcInfo.stTranLog.szPan, szPAN);
	if (iRet != 0)
	{
		return iRet;
	}

	PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount, 12, szTotalAmt);

	ucFlag &= (uchar)(0xFF - GETPIN_EMV);
	GetDispAmount(szTotalAmt, szDispAmt);
	strcat(szDispAmt, "\n");

	if (ucFlag == 0)
	{
		strcat(szDispAmt, _T("PLS ENTER PIN"));
	}
	else
	{
		strcat(szDispAmt, _T("PIN ERR, RETRY"));
	}
	//added by Kevin Liu 20160614 bug851 
	strcat(szDispAmt, "\n\n\n\n\n\n\n\n");

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szDispAmt, st_Small_Left, GUI_BUTTON_NONE, 0, NULL);

	// show prompt message on PINPAD
	if (ChkTermPEDMode(PED_EXT_PP))
	{
		ucAmtFlag = 0;
		if (glProcInfo.stTranLog.ucTranType == VOID || glProcInfo.stTranLog.ucTranType == REFUND)
		{
			ucAmtFlag |= GA_NEGATIVE;
		}
		App_ConvAmountTran(szTotalAmt, szBuff, ucFlag);
		// show amount on PINPAD
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, "(PLS USE PINPAD)", gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		PPScrCls();
		PPScrPrint(0, 0, szBuff);
	}

	if (ChkTermPEDMode(PED_INT_PCI))
	{
		ScrGotoxy(32, 6); // TODO: it should base on screen size Kim_LinHB
	}

	memset(glProcInfo.sPinBlock, 0, sizeof(glProcInfo.sPinBlock));

	// !!!! to be check
	//-------------- Internal PCI PED --------------
	if (ChkTermPEDMode(PED_INT_PCI))
	{
		iRet = PedGetPinBlock(ucPinKeyID, "4,5,6,7,8,9,10,11,12", szPAN, glProcInfo.sPinBlock, 0, USER_OPER_TIMEOUT * 1000);
		if (iRet == 0)
		{
			glProcInfo.stTranLog.uiEntryMode |= MODE_PIN_INPUT;
			return 0;
		}
		else if (iRet == PED_RET_ERR_NO_PIN_INPUT)	// !!!! PIN bypass
		{
			return 0;
		}
		else if (iRet == PED_RET_ERR_INPUT_CANCEL)	// !!!! cancel input
		{
			return ERR_USERCANCEL;
		}

		DispPciErrMsg(iRet);
		return ERR_NO_DISP;
	}

	//-------------- External PCI PED --------------
	if (ChkTermPEDMode(PED_EXT_PCI))
	{
		// !!!! to be implemented
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, "EXT PCI PINPAD\nNOT IMPLEMENTED.", gl_stCenterAttr, GUI_BUTTON_NONE, 30, NULL);
		return ERR_NO_DISP;
	}

	//-------------- PINPAD --------------

	if (ChkTermPEDMode(PED_EXT_PP))
	{
		iRet = PPGetPwd_3Des(ucPinKeyID, 0x31, 4, 6, szPAN, glProcInfo.sPinBlock, 0);
		DispWelcomeOnPED();
	}


	if (iRet == 0)
	{
		glProcInfo.stTranLog.uiEntryMode |= MODE_PIN_INPUT;
		return 0;
	}
	else if (iRet == 0x0A)
	{
		return 0;	// PIN by pass
	}
	else if (iRet == 0x06 || iRet == 0x07)
	{
		return ERR_USERCANCEL;
	}

	DispPPPedErrMsg((uchar)iRet);

	return ERR_NO_DISP;
}

void DispWelcomeOnPED(void)
{
	// 	PPScrCls();
	// 	PPScrPrint(0, 0, (uchar *)"   WELCOME!   ");
	// 	PPScrPrint(1, 0, (uchar *)"PAX TECHNOLOGY");
}

// get partial pan for PIN entry
int ExtractPAN(const uchar *pszPAN, uchar *pszOutPan)
{
	int		iLength;

	iLength = strlen((char*)pszPAN);
	if (iLength<13 || iLength>19)
	{
		return ERR_SWIPECARD;
	}

	memset(pszOutPan, '0', 16);
	memcpy(pszOutPan + 4, pszPAN + iLength - 13, 12);
	pszOutPan[16] = 0;

	return 0;
}

// calculate mac
int GetMAC(uchar ucMode, const uchar *psData, ushort uiDataLen, uchar ucMacKeyID, uchar *psOutMAC)
{
	if (!ChkIfNeedMac())
	{
		return 0;
	}

	// 0: ANSI x9.9, 1: fast arithm
	if (ChkTermPEDMode(PED_EXT_PP))
	{
		uchar	ucRet;
		ucRet = PPMac(ucMacKeyID, 0x01, psData, uiDataLen, psOutMAC, 0);	// !!!! mode 改造
		if (ucRet != 0)
		{
			DispPPPedErrMsg(ucRet);
			return ERR_NO_DISP;
		}
	}

	if (ChkTermPEDMode(PED_INT_PCI))
	{
		int		iRet;
		iRet = PedGetMac(ucMacKeyID, (uchar *)psData, uiDataLen, psOutMAC, ucMode);
		if (iRet != 0)
		{
			DispPciErrMsg(iRet);
			return ERR_NO_DISP;
		}
		return 0;
	}

	if (ChkTermPEDMode(PED_EXT_PCI))
	{
		// External PCI
		// !!!! to be implemented
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, "EXTERNAL PCI\nNOT IMPLEMENTED.", gl_stCenterAttr, GUI_BUTTON_NONE, 30, NULL);
		return ERR_NO_DISP;
	}

	return 0;
}

// show PED/PINPAD error message
void DispPPPedErrMsg(uchar ucErrCode)
{
	unsigned char szErrMsg[50];
	switch (ucErrCode)
	{
	case 0x01:
		strcpy(szErrMsg, _T("INV MAC DATA"));
		break;

	case 0x02:
	case 0x0B:
		strcpy(szErrMsg, _T("INVALID KEY ID"));
		break;

	case 0x03:
		strcpy(szErrMsg, _T("INVALID MODE"));
		break;

	default:
		sprintf(szErrMsg, "%s:%02X", _T("PED ERROR"), ucErrCode);
		break;
	}

	Gui_ClearScr();
	PubBeepErr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szErrMsg, gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
}

void DispPciErrMsg(int iErrCode)
{
	char	szDispBuff[64];

	// to be implemented
	switch (iErrCode)
	{
	case PED_RET_OK:
		return;

	case PED_RET_ERR_NO_KEY:
		sprintf(szDispBuff, _T("KEY MISSING"));
		break;

	case PED_RET_ERR_KEYIDX_ERR:
		sprintf(szDispBuff, _T("INVALID KEY INDEX"));
		break;

	case PED_RET_ERR_DERIVE_ERR:
		sprintf(szDispBuff, _T("INVALID KEY LEVEL"));
		break;

	case PED_RET_ERR_CHECK_KEY_FAIL:
	case PED_RET_ERR_KCV_CHECK_FAIL:
		sprintf(szDispBuff, _T("CHECK KEY FAIL"));
		break;

	case PED_RET_ERR_NO_PIN_INPUT:
		sprintf(szDispBuff, _T("PIN BYPASS"));
		break;

	case PED_RET_ERR_INPUT_CANCEL:
	case PED_RET_ERR_INPUT_TIMEOUT:
		sprintf(szDispBuff, _T("INPUT CANCELLED"));
		break;

	case PED_RET_ERR_WAIT_INTERVAL:
		sprintf(szDispBuff, _T("PLS TRY AGAIN LATER"));
		break;

	case PED_RET_ERR_CHECK_MODE_ERR:
		sprintf(szDispBuff, _T("INVALID MODE"));
		break;

	case PED_RET_ERR_NO_RIGHT_USE:
	case PED_RET_ERR_NEED_ADMIN:
		sprintf(szDispBuff, _T("PED ACCESS DENIALED"));
		break;

	case PED_RET_ERR_KEY_TYPE_ERR:
	case PED_RET_ERR_SRCKEY_TYPE_ERR:
		sprintf(szDispBuff, _T("INVALID KEY TYPE"));
		break;

	case PED_RET_ERR_EXPLEN_ERR:
		sprintf(szDispBuff, _T("INVALID PARA"));
		break;

	case PED_RET_ERR_DSTKEY_IDX_ERR:
		sprintf(szDispBuff, _T("INVALID DST INDEX"));
		break;

	case PED_RET_ERR_SRCKEY_IDX_ERR:
		sprintf(szDispBuff, _T("INVALID SRC INDEX"));
		break;

	case PED_RET_ERR_KEY_LEN_ERR:
		sprintf(szDispBuff, _T("INVALID KEY LENGTH"));
		break;

	case PED_RET_ERR_NO_ICC:
		sprintf(szDispBuff, _T("CARD NOT READY"));
		break;

	case PED_RET_ERR_ICC_NO_INIT:
		sprintf(szDispBuff, _T("CARD NOT INIT"));
		break;

	case PED_RET_ERR_GROUP_IDX_ERR:
		sprintf(szDispBuff, _T("INVALID GROUP INDEX"));
		break;

	case PED_RET_ERR_PARAM_PTR_NULL:
		sprintf(szDispBuff, _T("INVALID PARA"));
		break;

	case PED_RET_ERR_LOCKED:
		sprintf(szDispBuff, _T("PED LOCKED"));
		break;

	case PED_RET_ERROR:
		sprintf(szDispBuff, _T("PED GENERAL ERR"));
		break;

	case PED_RET_ERR_UNSPT_CMD:
	case PED_RET_ERR_DUKPT_OVERFLOW:
	case PED_RET_ERR_NOMORE_BUF:
	case PED_RET_ERR_COMM_ERR:
	case PED_RET_ERR_NO_UAPUK:
	case PED_RET_ERR_ADMIN_ERR:
	case PED_RET_ERR_DOWNLOAD_INACTIVE:
	default:
		sprintf(szDispBuff, "%s:%d", _T("PED ERROR"), iErrCode);
		break;
	}

	Gui_ClearScr();
	PubBeepErr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szDispBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
}

// 根据参数设置对PAN进行掩码
// Output a mask PAN
void MaskPan(const uchar *pszInPan, uchar *pszOutPan)
{
	uchar	szBuff[30];
	int		iCnt, iPanLen, iOption;

	memset(szBuff, 0, sizeof(szBuff));
	iPanLen = strlen((char *)pszInPan);
	char sPanMask[3] = "\x00\xFF\xF0";
	if (!ChkOptionExt(sPanMask, 0x0080))	//the first bit of the 24 bits
	{	// right align
		for (iCnt = 0; iCnt<iPanLen; iCnt++)
		{
			iOption = ((2 - iCnt / 8) << 8) + (1 << (iCnt % 8));
			if (!ChkOptionExt(sPanMask, (ushort)iOption))
			{
				szBuff[iPanLen - iCnt - 1] = pszInPan[iPanLen - iCnt - 1];
			}
			else
			{
				szBuff[iPanLen - iCnt - 1] = '*';
			}
		}
	}
	else
	{	// left align
		for (iCnt = 0; iCnt<iPanLen; iCnt++)
		{
			iOption = (((iCnt + 2) / 8) << 8) + (0x80 >> ((iCnt + 2) % 8));
			if (!ChkOptionExt(sPanMask, (ushort)iOption))
			{
				szBuff[iCnt] = pszInPan[iCnt];
			}
			else
			{
				szBuff[iCnt] = '*';
			}
		}
	}

	sprintf((char *)pszOutPan, "%.*s", LEN_PAN, szBuff);
}

// 获取8583打包/解包错误信息
void Get8583ErrMsg(uchar bPackErr, int iErrCode, uchar *pszErrMsg)
{
	PubASSERT(iErrCode<0);
	if (bPackErr)
	{
		if (iErrCode<-2000)
		{
			sprintf((char *)pszErrMsg, "BIT %d DEF ERR", -iErrCode - 2000);
		}
		else if (iErrCode<-1000)
		{
			sprintf((char *)pszErrMsg, "BIT %d PACK ERR", -iErrCode - 1000);
		}
		else
		{
			sprintf((char *)pszErrMsg, "PACK HEADER ERR");
		}
	}
	else
	{
		if (iErrCode<-2000)
		{
			sprintf((char *)pszErrMsg, "BIT %d DEF ERR", -iErrCode - 2000);
		}
		else if (iErrCode<-1000)
		{
			sprintf((char *)pszErrMsg, "UNPACK %d ERR", -iErrCode - 1000);
		}
		else if (iErrCode == -1000)
		{
			sprintf((char *)pszErrMsg, "DATA LENGTH ERR");
		}
		else
		{
			sprintf((char *)pszErrMsg, "UNPACK HEAD ERR");
		}
	}
}


// 获取预授权号码
// get pre-authorization code
int GetPreAuthCode(void)
{
	uchar szBuff[LEN_AUTH_CODE + 1];

	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_MIX;
	stInputAttr.nMinLen = 2;
	stInputAttr.nMaxLen = LEN_AUTH_CODE;

	memset(szBuff, 0, sizeof(szBuff));

	Gui_ClearScr();
	if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("APPV CODE ?"), gl_stLeftAttr, szBuff,
		gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%-*s", LEN_AUTH_CODE, szBuff);

	return 0;
}

// 获取票据号
// get invoice no
int InputInvoiceNo(ulong *pulInvoiceNo)
{
	uchar szBuff[LEN_INVOICE_NO + 1];

	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = LEN_INVOICE_NO;

	memset(szBuff, 0, sizeof(szBuff));

	Gui_ClearScr();
	if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("TRACE NO ?"), gl_stLeftAttr, szBuff,
		gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	*pulInvoiceNo = (ulong)atol((char *)szBuff);

	return 0;
}

// 获取要显示的交易状态信息
// Modified by Kim_LinHB 2014-8-8 v1.01.0002 bug508
// return 0 use current transaction type 
// return 1 use original transaction type
// return -1 error
int  GetStateText(ushort uiStatus, uchar *pszStaText)
{
	if (!pszStaText)
		return -1;
	sprintf((char *)pszStaText, "Normal");
	if (uiStatus & TS_VOID)
	{
		sprintf((char *)pszStaText, "VOIDED");
		return 1;
	}
	else if (uiStatus & TS_ADJ)
	{
		sprintf((char *)pszStaText, "ADJUSTED");
	}
	else if (uiStatus & TS_NOSEND)
	{
		sprintf((char *)pszStaText, "NOT_SEND");
		return 0;
	}
	return 0;
}


// 显示操作成功的动画
// Show animation of "done"
void DispOperOk(const void *pszMsg)
{
	static uchar sLogoOkThree[117] =
	{
		0x03,
		0x00,0x24,
		0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc0,0x40,0x0,0x40,0x0,0x40,0x0,0x40,0x0,
		0x40,0x0,0x40,0x0,0x40,0x0,0x80,0xc0,0xc0,0xc0,0xe0,0x60,0x60,0x30,0x30,0x30,
		0x10,0x10,0x10,0x0,

		0x00,0x24,
		0x0,0x4,0x4,0x4,0xc,0x18,0x18,0xba,0x70,0xe0,0xe0,0xc0,0xc0,0x80,0xc0,0xe0,
		0xf0,0xf8,0x7c,0x3e,0x1f,0xf,0x7,0xab,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0x0,

		0x00,0x24,
		0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,0x4,0x0,0x5,0x3,0x7,0x3,0x7,0x3,
		0x5,0x0,0x4,0x0,0x4,0x0,0x4,0x6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0x0
	};
	static uchar sLogoOkTwo[117] =
	{
		0x03,
		0x00,0x24,
		0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc0,0x40,0x0,0x40,0x0,0x40,0x0,0x40,0x0,
		0x40,0x0,0x40,0x0,0x40,0x0,0x40,0xc0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0x0,

		0x00,0x24,
		0x0,0x4,0x4,0x4,0xc,0x18,0x18,0xba,0x70,0xe0,0xe0,0xc0,0xc0,0x80,0xc0,0xe0,
		0xf0,0xf8,0x7c,0x38,0x10,0x0,0x0,0xaa,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0x0,

		0x00,0x24,
		0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,0x4,0x0,0x5,0x3,0x7,0x3,0x7,0x3,
		0x5,0x0,0x4,0x0,0x4,0x0,0x4,0x6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0x0
	};
	static uchar sLogoOkOne[117] =
	{
		0x03,
		0x00,0x24,
		0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc0,0x40,0x0,0x40,0x0,0x40,0x0,0x40,0x0,
		0x40,0x0,0x40,0x0,0x40,0x0,0x40,0xc0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0x0,

		0x00,0x24,
		0x0,0x4,0x4,0x4,0xc,0x18,0x18,0xba,0x70,0xe0,0xe0,0xc0,0xc0,0x80,0x0,0x0,
		0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xaa,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0x0,

		0x00,0x24,
		0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,0x4,0x0,0x5,0x3,0x7,0x3,0x6,0x0,
		0x4,0x0,0x4,0x0,0x4,0x0,0x4,0x6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0x0
	};

	uchar	ucLogoNo;
	
	Gui_DrawText(pszMsg, gl_stLeftAttr, 0, 40);

	TimerSet(TIMER_TEMPORARY, 2);
	//ScrGotoxy(76, 3);
	for (ucLogoNo = 0; ucLogoNo<3; )
	{
		if (TimerCheck(TIMER_TEMPORARY) == 0)
		{
			if (ucLogoNo == 0)
			{
				Gui_DrawLogo(sLogoOkOne, 76, 24);
			}
			else if (ucLogoNo == 1)
			{
				Gui_DrawLogo(sLogoOkTwo, 76, 24);
			}
			else
			{
				Gui_DrawLogo(sLogoOkThree, 76, 24);
			}
			ucLogoNo++;
			TimerSet(TIMER_TEMPORARY, 2);
		}
	}
	DelayMs(1500);
}

// 选择收单行
// (for settle/reprint ....)
int SelectAcq(uchar bAllowSelAll, const uchar *pszTitle, uchar *pucAcqIndex)
{
	return ERR_USERCANCEL;
}

// 选择发卡行
int SelectIssuer(uchar *pucIssuerIndex)
{
	return ERR_USERCANCEL;
}


// 显示交易汇总信息(glTransTotal)
// Modified by Kim_LinHB 2014/9/16 v1.01.0009 bug509
int DispTransTotal(uchar bShowVoidTrans)
{
	

	return ERR_USERCANCEL;
}

/*
// 调节屏幕对比度
void AdjustLcd(void)
{
uchar	ucKey, szBuff[30];

while( 1 )
{
PubShowTitle(TRUE, (uchar *)"ADJUST CONTRAST");
sprintf((char *)szBuff, _T("STEP = [%d]"), glSysParam.stEdcInfo.ucScrGray);
PubDispString(szBuff, DISP_LINE_CENTER|3);
PubDispString(_T("[CANCEL] - EXIT"), DISP_LINE_CENTER|6);
ScrGray(glSysParam.stEdcInfo.ucScrGray);
ucKey = PubWaitKey(USER_OPER_TIMEOUT);
if( ucKey==KEYCANCEL || ucKey==NOKEY )
{
break;
}
glSysParam.stEdcInfo.ucScrGray = (glSysParam.stEdcInfo.ucScrGray+1) % 8;
}
SaveEdcParam();
}
*/

// 判断是否为数字串
uchar IsNumStr(const char *pszStr)
{
	if (pszStr == NULL || *pszStr == 0)
	{
		return FALSE;
	}

	while (*pszStr)
	{
		if (!isdigit(*pszStr++))
		{
			return FALSE;
		}
	}

	return TRUE;
}

// 获取交易的英文名称
//void GetEngTranLabel(uchar *pszTranTitle, uchar *pszEngLabel)

// 取得金额的符号
uchar GetTranAmountInfo(void *pTranLog)
{
	uchar		ucSignChar;
	TRAN_LOG	*pstLog;

	pstLog = (TRAN_LOG *)pTranLog;
	ucSignChar = 0;
	if (pstLog->ucTranType == REFUND || pstLog->ucTranType == VOID || (pstLog->uiStatus &TS_VOID))
	{
		ucSignChar = GA_NEGATIVE;
	}

	if ((pstLog->ucTranType == VOID) && (pstLog->ucOrgTranType == REFUND))
	{
		ucSignChar = 0;
	}

	if ((pstLog->ucTranType == REFUND) && (pstLog->uiStatus &TS_VOID))
	{
		ucSignChar = 0;
	}

	return ucSignChar;
}

void DispHostRspMsg(uchar *pszRspCode, HOST_ERR_MSG *pstMsgArray)
{
	int		iCnt;
	char	szDispMsg[64];

	for (iCnt = 0; pstMsgArray[iCnt].szRspCode[0] != 0; iCnt++)
	{
		if (memcmp(pszRspCode, pstMsgArray[iCnt].szRspCode, 2) == 0)
		{
			break;
		}
	}

	sprintf(szDispMsg, _T(pstMsgArray[iCnt].szMsg));

	if (ChkIfBea())
	{
		if (memcmp(glProcInfo.stTranLog.szRspCode, "08", 2) == 0 ||
			memcmp(glProcInfo.stTranLog.szRspCode, "43", 2) == 0)
		{
			sprintf(szDispMsg, _T("PLS CALL BANK"));
		}
	}

	sprintf(szDispMsg + strlen(szDispMsg), "\n%.2s", pszRspCode);

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szDispMsg, gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);
}

void DispResult(int iErrCode)
{
	int		iCnt;

	if ((iErrCode == ERR_NO_DISP) ||
		(iErrCode == ERR_EXIT_APP))
	{
		return;
	}

	switch (iErrCode)
	{
	case 0:
		switch (glProcInfo.stTranLog.ucTranType)
		{

		default:
			if (glProcInfo.stTranLog.szAuthCode[0] == 0 ||
				memcmp(glProcInfo.stTranLog.szAuthCode, "       ", 6) == 0)
			{
				DispAccepted();
			}
			else
			{
				unsigned char szBuff[50];
				sprintf(szBuff, "%s\n%.6s", _T("APPV CODE"), glProcInfo.stTranLog.szAuthCode);
				Gui_ClearScr();
				PubBeepOk();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_OK, glSysParam.stEdcInfo.ucAcceptTimeout, NULL);
			}
		}
		break;

	case ERR_HOST_REJ:
		DispHostRspMsg(glProcInfo.stTranLog.szRspCode, glHostErrMsg);
		break;

	default:
		for (iCnt = 0; glTermErrMsg[iCnt].iErrCode != 0; iCnt++)
		{
			if (glTermErrMsg[iCnt].iErrCode == iErrCode)
			{
				Gui_ClearScr();
				PubBeepErr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T(glTermErrMsg[iCnt].szMsg), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
				break;
			}
		}
		if (glTermErrMsg[iCnt].iErrCode == 0)
		{
			unsigned char szBuff[50];
			sprintf(szBuff, "%s\n%04x", _T("SYSTEM ERROR"), iErrCode);
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		}
		break;
	}
	Gui_ClearScr(); // Added by Kim_LinHB 2014-08-18 v1.01.0004
}

// 计算单据数目
// Get number of receipts to pint out in one transaction.
int NumOfReceipt(void)
{
	int		iNum;

	iNum = 1;
	if (ChkEdcOption(EDC_NUM_PRINT_LOW))
	{
		iNum++;
	}
	if (ChkEdcOption(EDC_NUM_PRINT_HIGH))
	{
		iNum += 2;
	}

	return iNum;
}

// if any issuer ticked PIN REQUIRED option, it open the EMV offline PIN
// deleted
//void ModifyTermCapForPIN(void)

#ifdef ENABLE_EMV
// set tag 9C for EMV
void UpdateEMVTranType(void)
{
	// 设置EMV参数
	// set EMV parameters
	EMVGetParameter(&glEmvParam);
	switch (glProcInfo.stTranLog.ucTranType)
	{
	case PURCHASE:
		glEmvParam.TransType = EMV_GOODS;
		EMVSetTLVData(0x9C, (uchar *)"\x00", 1);
		break;

	case POS_PRE_AUTHORIZATION:
		EMVSetTLVData(0x9C, (uchar *)"\x03", 1); //(uchar *)"\x30", 1);
		glEmvParam.TransType = EMV_GOODS;
		break;

	case BALANCE:
		EMVSetTLVData(0x9C, (uchar *)"\x31", 1);
		glEmvParam.TransType = EMV_GOODS;
		break;
	default:
		return;
	}
	EMVSetParameter(&glEmvParam);
	// Only in this trasaction, so DON'T back up
}
#endif

int FindCurrency(const uchar *pszCurrencyNameCode, CURRENCY_CONFIG *pstCurrency)
{
	int	iCnt;
	uchar	sBCD[8], sBuff[8];

	for (iCnt = 0; glCurrency[iCnt].szName[0] != 0; iCnt++)
	{
		if (IsNumStr(pszCurrencyNameCode))
		{
			sprintf((char *)sBuff, "0%.3s", pszCurrencyNameCode);
			PubAsc2Bcd(sBuff, 3, sBCD);
			if (memcmp(sBCD, glCurrency[iCnt].sCurrencyCode, 2) == 0)
			{
				memcpy(pstCurrency, &glCurrency[iCnt], sizeof(CURRENCY_CONFIG));
				return 0;
			}
		}
		else
		{
			if (strcmp((char *)pszCurrencyNameCode, (char *)glCurrency[iCnt].szName) == 0)
			{
				memcpy(pstCurrency, &glCurrency[iCnt], sizeof(CURRENCY_CONFIG));
				return 0;
			}
		}
	}

	return -1;
}

#ifdef ENABLE_EMV
// 根据EDC参数设定EMV库的国家代码和货币特性
// Setup EMV core parameter due to EDC para
void SyncEmvCurrency(const uchar *psCountryCode, const uchar *psCurrencyCode, uchar ucDecimal)
{
	EMVGetParameter(&glEmvParam);
	if ((memcmp(psCountryCode, glEmvParam.CountryCode, 2) != 0) ||
		(memcmp(psCurrencyCode, glEmvParam.TransCurrCode, 2) != 0) ||
		(glEmvParam.TransCurrExp != ucDecimal))
	{
		memcpy(glEmvParam.CountryCode, psCountryCode, 2);
		memcpy(glEmvParam.TransCurrCode, psCurrencyCode, 2);
		memcpy(glEmvParam.ReferCurrCode, psCurrencyCode, 2);
		glEmvParam.TransCurrExp = ucDecimal;
		glEmvParam.ReferCurrExp = ucDecimal;
		EMVSetParameter(&glEmvParam);
	}
}
#endif

// Read monitor config info, by API: GetTermInfo()
// return: 0--No need save; 1--Need save
// 读取monitor保存的系统配置信息
// 返回：0－－不需要保存更新；1－－需要保存
int UpdateTermInfo(void)
{
	int		iRet;
	uchar	ucNeedUpdate, sBuff[sizeof(glSysParam.sTermInfo)];

	ucNeedUpdate = 0;

	while (1)
	{
		memset(sBuff, 0, sizeof(sBuff));
		iRet = GetTermInfo(sBuff);
		if (iRet<0)
		{
#ifdef _WIN32
			Gui_ClearScr();
			Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("CONNECT SIMULTR."), gl_stCenterAttr, GUI_BUTTON_NONE, 1000, NULL);
			continue;
#else
			SysHaltInfo(_T("FAIL GET SYSINFO"));
#endif
		}

		break;
	}

	if (memcmp(sBuff, glSysParam.sTermInfo, sizeof(glSysParam.sTermInfo)) != 0)
	{
		memcpy(glSysParam.sTermInfo, sBuff, sizeof(glSysParam.sTermInfo));
		ucNeedUpdate = 1;
	}

	return ucNeedUpdate;
}

static void ShowMsgFontMissing(uchar bIsPrnFont, ST_FONT *psingle_code_font, ST_FONT *pmulti_code_font, int iErrCode)
{
	uchar	szBuff[64 + 30];
	sprintf((char *)szBuff, "%02d:%d*%d %s%s\n%02d:%d*%d %s%s %d",
		psingle_code_font->CharSet,
		psingle_code_font->Width, psingle_code_font->Height,
		(psingle_code_font->Bold ? "B" : ""), (psingle_code_font->Italic ? "I" : ""),
		pmulti_code_font->CharSet,
		pmulti_code_font->Width, pmulti_code_font->Height,
		(pmulti_code_font->Bold ? "B" : ""), (pmulti_code_font->Italic ? "I" : ""), iErrCode);

	if (bIsPrnFont)
	{
		strcat(szBuff, "\nPRN FONT MISS");
	}
	else
	{
		strcat(szBuff, "\nDISP FONT MISS");
	}

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, -1, NULL);
}

// Check whether system has fonts(for print and display) required under selected language.
// 在多语言动态切换环境下，检查系统是否有已选择语言所对应的打印和显示字库
int CheckSysFont(void)
{
	int		iRet, iRet1, iRet2, ii;

	ST_FONT	stPrnFonts[] = {
		{ CHARSET_WEST, 8,  16, 0, 0 },{ -1, 16, 16, 0, 0 },
		{ CHARSET_WEST, 12, 24, 0, 0 },{ -1, 24, 24, 0, 0 },
	};

	ST_FONT	stDispFonts[] = {
		{ CHARSET_WEST, 8, 16, 0, 0 },{ -1, 16, 16, 0, 0 },
	};

#ifdef AREA_Arabia
	if (strcmp(LANGCONFIG, "Arabia") == 0)
	{
		iRet = CustomizeAppLibForArabiaLang(TRUE);
		return iRet;
	}
	else
	{
		CustomizeAppLibForArabiaLang(FALSE);
		// then continue
	}
#endif	

	iRet = 0;
	// 检查是否有打印/显示字库 Check printer/display used fonts
	// for non-S60, display and print share the same fonts
	for (ii = 0; ii<sizeof(stPrnFonts) / sizeof(ST_FONT); ii += 2)
	{
		if (stPrnFonts[ii + 1].CharSet == -1)// 换成系统当前已选择的语言的编码
		{
			stPrnFonts[ii + 1].CharSet = glSysParam.stEdcInfo.stLangCfg.ucCharSet;
		}

		iRet2 = PrnSelectFont(&stPrnFonts[ii], &stPrnFonts[ii + 1]);
		if (iRet2)
		{
			ShowMsgFontMissing(TRUE, &stPrnFonts[ii], &stPrnFonts[ii + 1], iRet2);
			iRet = -1;
		}
	}
	// 如果是S60，还要检查手机显示用字库 Check display-used fonts on handset of S60
	if (ChkTerm(_TERMINAL_S60_))
	{
		for (ii = 0; ii<sizeof(stDispFonts) / sizeof(ST_FONT); ii += 2)
		{
			if (stDispFonts[ii + 1].CharSet == -1)// 换成系统当前已选择的语言的编码
			{
				stDispFonts[ii + 1].CharSet = glSysParam.stEdcInfo.stLangCfg.ucCharSet;
			}

			iRet1 = ScrSelectFont(&stDispFonts[ii], &stDispFonts[ii + 1]);
			if (iRet1)
			{
				ShowMsgFontMissing(FALSE, &stDispFonts[ii], &stDispFonts[ii + 1], iRet1);
				iRet = -1;
			}
		}
	}

	return iRet;
}

// Enumerates all supported fonts in POS.
// 列举系统字库
int EnumSysFonts(void)
{
	int			iRet, ii;
	ST_FONT		stFontsList[16];

	GUI_MENU	stInfoList;
	GUI_MENUITEM	stInfoListItem[16 + 1];

	iRet = EnumFont(stFontsList, 16);
	if (iRet<1)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(_T("ENUM SYS FONTS"), gl_stTitleAttr, _T("ENUM FAILED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
		return ERR_NO_DISP;
	}

	memset(stInfoListItem, 0, sizeof(stInfoListItem));
	for (ii = 0; ii<iRet; ii++)
	{
		sprintf((char *)stInfoListItem[ii].szText, "%02d-%02dx%02d,%d,%d",
			stFontsList[ii].CharSet, stFontsList[ii].Width, stFontsList[ii].Height,
			stFontsList[ii].Bold, stFontsList[ii].Italic); // Modified by Kim_LinHB 2014-8-6 v1.01.0001 bug494
		stInfoListItem[ii].bVisible = TRUE;
		stInfoListItem[ii].nValue = ii;
		stInfoListItem[ii].vFunc = NULL;
	}

	strcpy(stInfoListItem[ii].szText, "");
	stInfoListItem[ii].bVisible = FALSE;
	stInfoListItem[ii].nValue = -1;
	stInfoListItem[ii].vFunc = NULL;

	Gui_BindMenu(_T("ENUM SYS FONTS"), gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stInfoListItem, &stInfoList);
	Gui_ClearScr();
	Gui_ShowMenuList(&stInfoList, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, NULL);
	return 0;
}

int SxxWriteKey(uchar ucSrcKeyIdx, const uchar *psKeyBCD, uchar ucKeyLen, uchar ucDstKeyId, uchar ucDstKeyType, const uchar *psKCV)
{
	int			iRet;
	ST_KEY_INFO	stKeyInfoIn;
	ST_KCV_INFO	stKcvInfoIn;

	memset(&stKeyInfoIn, 0, sizeof(stKeyInfoIn));
	memset(&stKcvInfoIn, 0, sizeof(stKcvInfoIn));

	memcpy(stKeyInfoIn.aucDstKeyValue, psKeyBCD, ucKeyLen);
	stKeyInfoIn.iDstKeyLen = ucKeyLen;
	stKeyInfoIn.ucDstKeyIdx = ucDstKeyId;
	stKeyInfoIn.ucDstKeyType = ucDstKeyType;
	stKeyInfoIn.ucSrcKeyIdx = ucSrcKeyIdx;
	stKeyInfoIn.ucSrcKeyType = PED_TMK;

	if (psKCV == NULL)
	{
		stKcvInfoIn.iCheckMode = 0;
	}
	else
	{
		stKcvInfoIn.iCheckMode = 1;
		stKcvInfoIn.aucCheckBuf[0] = 4;
		memcpy(stKcvInfoIn.aucCheckBuf + 1, psKCV, 4);
	}

	iRet = PedWriteKey(&stKeyInfoIn, &stKcvInfoIn);
	return iRet;
}


uchar ChkOnBase(void)
{
	if (!ChkTerm(_TERMINAL_S60_))
	{
		return 0;
	}
	return OnBase();
}

void SetOffBase(unsigned char(*Handle)())
{
	if (!ChkTerm(_TERMINAL_S60_))
	{
		return;
	}
	if (Handle == NULL)
	{
		//ScrPrint(0,0,0," SET NULL ");DelayMs(500);ScrPrint(0,0,0,"          ");

		ScrSetEcho(0);
		return;
	}
	//ScrPrint(0,0,0," SET ECHO ");DelayMs(500);ScrPrint(0,0,0,"          ");
	ScrSetOutput(2);
	(*Handle)();

	ScrSetEcho(2);
	ScrSetOutput(1);
	ScrSetEcho(1);
}

uchar OffBaseDisplay(void)
{
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("RETURN TO BASE"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
	return 0;
}

uchar OffBaseCheckPrint(void)
{
	if (OnBase() == 0)
	{
		return 0;
	}
	return OffBaseDisplay();
}

uchar OffBaseCheckEcr(void)
{
	if (OnBase() == 0)
	{
		return 0;
	}
	return OffBaseDisplay();
}

// Added by Kim_LinHB 2014-6-21
void SetCurrTitle(const unsigned char *pszTitle)
{
	if (pszTitle)
	{
		sprintf(gl_szCurrTitle, "%.*s", sizeof(gl_szCurrTitle), pszTitle);
	}
}

const unsigned char *GetCurrTitle(void)
{
	if (0 == gl_szCurrTitle[0])
		return NULL;
	return (const unsigned char *)&gl_szCurrTitle[0];
}
// Add End

// Added by Kim_LinHB 2014-08-22 v1.01.0004
void SplitIpAddress(const unsigned char *Ip, unsigned char sub[4])
{
	unsigned char *p;
	int iCnt = 0;

	p = (unsigned char *)Ip;
	while (p)
	{
		sub[iCnt++] = atoi(p);
		p = strchr(p + 1, '.');
		if (p) ++p;
	}
}

void MergeIpAddress(const unsigned char sub[4], unsigned char *Ip)
{
	if (Ip)
	{
		sprintf(Ip, "%d.%d.%d.%d", sub[0], sub[1], sub[2], sub[3]);
	}
}
static int sgLastKey = 0;
int GetLastKey() {
	return sgLastKey;
}
void SetLastKey(int key) {
	sgLastKey = key;
}
int DelFilesbyPrefix(const char *prefix)
{
	int i = 0, total = 0;
	static FILE_INFO files[256];
	total = GetFileInfo(files);
	while (i < total) {
		if (strstr(files[i].name, prefix)) {
			remove(files[i].name);
		}
		++i;
	}
	return 0;
}

const char *GetCurSignPrefix(uchar ucAcqKey)
{
	static char szPath[255];
	if (ACQ_ALL == ucAcqKey)
		sprintf(szPath, "SIG-");
	else
		sprintf(szPath, "SIG-%d-%d", ucAcqKey, glPosParams.batchNo);
	return szPath;
}
char HasE_Signature()
{
	return glProcInfo.stTranLog.szSignPath[0] != 0 && fexist(glProcInfo.stTranLog.szSignPath) >= 0;
}
void DoE_Signature()
{
	if ((glSysParam.sTermInfo[19] & 0x01) == 1)
	{ // touch screen, focus to get the E signature
		int iRet;
		sprintf(glProcInfo.stTranLog.szSignPath, "%s-%d", GetCurSignPrefix(ACQ_ALL), glProcInfo.stTranLog.ulInvoiceNo);
		Gui_ClearScr();
		iRet = Gui_ShowSignatureBoard(_T("Please Sign"), gl_stTitleAttr, glProcInfo.stTranLog.szSignPath, 0, USER_OPER_TIMEOUT);
	}
}



int InputTransactionAmount(char* title, uchar ucAmtType, uchar amount[12 + 1])
{
	uchar	*pszAmt;
	uchar	szBuff[100];
	int iRet;
	GUI_INPUTBOX_ATTR stInputAttr;

	pszAmt = amount;

	PubASSERT(ucAmtType == AMOUNT || ucAmtType == RFDAMOUNT || ucAmtType == ORGAMOUNT || ucAmtType == CASHBACKAMOUNT);

	memset(szBuff, 0, sizeof(szBuff));
	switch (ucAmtType)
	{
	case AMOUNT:
		strcpy(szBuff, _T("AMOUNT"));
		break;

	case RFDAMOUNT:	// RFU
		strcpy(szBuff, _T("REFUND AMOUNT"));
		break;

	case ORGAMOUNT:	// RFU
		strcpy(szBuff, _T("ORIGINAL AMOUNT"));
		break;

	case CASHBACKAMOUNT:
		strcpy(szBuff, _T("CASHBACK AMOUNT"));
		break;
	}

	strcat(szBuff, _T(" ("));
	strncat(szBuff, glPosParams.currency.szName, 3);
	strcat(szBuff, _T(")"));

	sg_ucAmtType = ucAmtType;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 1;

	//sprintf((char *)szCurrName, "%.3s%1.1s", glSysParam.stEdcInfo.szCurrencyName, &glSysParam.stEdcInfo.ucCurrencySymbol);
	stInputAttr.nMinLen =  1;
	stInputAttr.nMaxLen = 10;

	Gui_RegCallback(GUI_CALLBACK_UPDATE_TEXT, DisplayInputAmount);

	while (1)
	{

		Gui_ClearScr();
		iRet = Gui_ShowInputBox(title, gl_stTitleAttr, szBuff, gl_stLeftAttr, pszAmt,
			gl_stCenterAttr, &stInputAttr, USER_OPER_TIMEOUT);
		if (GUI_OK != iRet)
		{
			sg_ucAmtType = AMOUNT;
			Gui_RegCallback(GUI_CALLBACK_UPDATE_TEXT, NULL);
			return APP_CANCEL;
		}

		if (ucAmtType != CASHBACKAMOUNT) // Added by Kim_LinHB 9/9/2014 v1.01.0007 bug520
		{
			if (0 == atoi(pszAmt))
			{
				Beep();
				continue;
			}
		}

		// Use transaction currency to do conversion
		AmtConvToBit4Format(pszAmt, glPosParams.currency.ucIgnoreDigit);
		sg_ucAmtType = AMOUNT;
		Gui_RegCallback(GUI_CALLBACK_UPDATE_TEXT, NULL);
		return 0;
	}

	return 0;
}

const char* GetTerminalModel() {
	char termInfo[32] = { 0 };
	GetTermInfo(termInfo);

	switch (termInfo[0]) {
	case _TERMINAL_S80_:
		return "S80";
	case _TERMINAL_SP30_:
		return "SP30";
	case _TERMINAL_S90_:
		return "S90";
	case _TERMINAL_D200_:
		return "D200";
	case _TERMINAL_D210_:
		return "D210";
	case _TERMINAL_S900_:
		return "S900";
	case 23:
		return "D180";
	case 26:
		return "S910";
	default:
		return "Unknown";
	}
}

double absolute(double number)
{
	//printf("&#37;f ", number);
	unsigned long long* x = (unsigned long long*)&number;
	*x &= 0x7fffffffffffffff;

	//printf("%f \n", number);
	return number;
}


uchar checkTerminalPrepStatus() {
	if (!glPosParams.ucIsPrepped) {
		DispErrMsg("Terminal not prepped", "Please prep terminal!!!", USB_ERR_TIMEOUT, DERR_BEEP);
	}
	return glPosParams.ucIsPrepped;
}

int get_wl_info(WlInfo_T *wl_info)
{
	int ret = -1;
	int key = 0;

	memset(wl_info, 0, sizeof(WlInfo_T));
	WlInit(NULL);

	ret = WlOpenPort();
	//if (ret != 0) return -1; 
	//WlUsePaxBaseSo(NONUSE_PAX_BASESO);

	key = /*RSSI_KEY | ISP_KEY |*/ CELLINFO_KEY;
	ret = WlGetInfo(key, wl_info);
	//wl_info_ok = ret;

	logTrace("WlGetInfo============================>ret=%d", ret);
	WlClosePort();

	return ret;
}

int GetSignal_Status(char * status)
{
	int Sig;
	char dSignal[6];

	//ScrCls();
	if (WlGetSignal((char *)dSignal) == 0)
	{
		if (dSignal[0] == 0x00)//Very good
		{
			strcpy(status, "VERY GOOD SIGNAL");
			Sig = 100;
		}
		else if (dSignal[0] == 0x01) {//Good
			strcpy(status, "GOOD SIGNAL");
			Sig = 80;
		}
		else if (dSignal[0] == 0x02) {//signal ok
			strcpy(status, "SIGNAL OK");
			Sig = 60;
		}
		else if (dSignal[0] == 0x03) {//Signal weak
			strcpy(status, "SIGNAL WEAK");
			Sig = 40;	
		}
		else if (dSignal[0] == 0x04) {//Signal very weak
			Sig = 20;
			strcpy(status, "SIGNAL VERY WEAK");
		}
		else if (dSignal[0] == 0x05) {//No signal
			Sig = 0;
			strcpy(status, "NO SIGNAL");
		}
		else {
			Sig = 0;
			strcpy(status, "NO NETWORK");
		}
	}
	else
	{
		strcpy(status, "NO NETWORK");
		Sig = 0;
	}

	return Sig;
}


// end of file

