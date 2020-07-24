
#include "global.h"

/********************** Internal macros declaration ************************/
#define DOWNPARA_FILE	"SYS_DOWN_EDC"

/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static void RemoveEmvAppCapk(void);
static void TransformIP(const uchar * ip_in, uchar * ip_out);
static void TransformPort(const uchar * port_in, uchar * port_out);

static int  SaveEmvMisc(const uchar *psPara);
static int  SaveEmvApp(const uchar *psPara);
static int  SaveEmvCapk(const uchar *psPara);
static void GetNextAutoDayTime(uchar *pszDateTimeInOut, ushort uiInterval);

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

	DelFilesbyPrefix(GetCurSignPrefix(ACQ_ALL));

	memset(&glSysCtrl, 0, sizeof(SYS_CONTROL));
	glSysCtrl.ulInvoiceNo = 1L;
	
	srand(DEVICE_GetTickCount());
	glSysCtrl.ulSTAN = rand() % 3000;

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

#ifdef APP_DEBUG
	InitTestApps();
	InitTestKeys();
#endif // APP_DEBUG

	InitLiveApps();
	InitLiveKeys();

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

// end of file

