
#include "global.h"




extern int processNibssTransaction();
extern int rollbackNibssTransaction(int reason);


/********************** Internal macros declaration ************************/
// macros for analyze EMV TLV string
#define TAGMASK_CLASS			0xC0	// tag mask of tag class
#define TAGMASK_CONSTRUCTED		0x20	// tag mask of constructed/primitive data
#define TAGMASK_FIRSTBYTE		0x1F	// tag mask of first byte
#define TAGMASK_NEXTBYTE		0x80	// tag mask of next byte

#define LENMASK_NEXTBYTE		0x80	// length mask
#define LENMASK_LENBYTES		0x7F	// mask of bytes of lenght

#define TAG_NULL_1				0x00	// null tag
#define TAG_NULL_2				0xFF	// null tag

#define DE55_LEN_FIXED		0x01	// for amex
#define DE55_LEN_VAR1		0x02
#define DE55_LEN_VAR2		0x03

#define DE55_MUST_SET		0x10	// 必须存在
#define DE55_OPT_SET		0x20	// 可选择存在
#define DE55_COND_SET		0x30	// 根据条件存在
/********************** Internal structure declaration *********************/
// callback function for GetTLVItem() to save TLV value
typedef void(*SaveTLVData)(uint uiTag, const uchar *psData, int iDataLen);

typedef struct _tagDE55Tag
{
	ushort	uiEmvTag;
	uchar	ucOption;
	uchar	ucLen;		// for process AMEX bit 55, no used for ADVT/TIP
}DE55Tag;

typedef struct _tagScriptInfo
{
	ushort	uiTag;
	int		iIDLen;
	uchar	sScriptID[4];
	int		iCmdLen[20];
	uchar	sScriptCmd[20][300];
}ScriptInfo;

/********************** Internal functions declaration *********************/
static int  SetAmexDE55(const DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  AppendStdTagList(DE55Tag *pstList, ushort uiTag, uchar ucOption, uchar ucMaxLen);
static int  RemoveFromTagList(DE55Tag *pstList, ushort uiTag);
static int  SetStdDE55(uchar bForUpLoad, const DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  SetStdDE56(const DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  GetTLVItem(uchar **ppsTLVString, int iMaxLen, SaveTLVData pfSaveData, uchar bExpandAll);
static int  GetSpecTLVItem(const uchar *psTLVString, int iMaxLen, uint uiSearchTag, uchar *psOutTLV, ushort *puiOutLen);
static int  GetDE55Amex(const uchar *psSendHeader, const uchar *psRecvDE55, int iLen);
static int  IsConstructedTag(uint uiTag);
static void SaveRspICCData(uint uiTag, const uchar *psData, int iDataLen);
static void BuildTLVString(ushort uiEmvTag, const uchar *psData, int iLength, uchar **ppsOutData);
static void SaveEmvData(void);
static void AdjustIssuerScript(void);
static void SaveScriptData(uint uiTag, const uchar *psData, int iDataLen);
static void PackTLVData(uint uiTag, const uchar *psData, uint uiDataLen, uchar *psOutData, int *piOutLen);
static void PackTLVHead(uint uiTag, uint uiDataLen, uchar *psOutData, int *piOutLen);
static int  CalcTLVTotalLen(uint uiTag, uint uiDataLen);
static void PackScriptData(void);
static void SaveTVRTSI(uchar bBeforeOnline);
static void UpdateEntryModeForOfflinePIN(void);
static int FinishOffLine(void);

/********************** Internal variables declaration *********************/
static uchar sAuthData[16];			// authentication data from issuer
static uchar sIssuerScript[300];	// issuer script
// { // for test only
// 	0x71, 0x12+0x0F, 0x9F, 0x18, 0x00, 0x86, 0x0D, 0x84, 0x1E, 0x00, 0x00, 0x08,
// 	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
// 	0x86, 0x0D, 0x84, 0x1E, 0x00, 0x00, 0x08,
// 	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
// 	0x72, 0x12+4, 0x9F, 0x18, 0x04, 0,1,2,3, 0x86, 0x0D, 0x84, 0x1E, 0x00, 0x00, 0x08,
// 	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
// };
//static int iScriptLen=40+15+4;

static int			sgAuthDataLen, sgScriptLen;
static ScriptInfo	sgScriptInfo;
static uchar		sgScriptBak[300];
static int			sgCurScript, bHaveScript, sgScriptBakLen;


// AMEX format field 55
static DE55Tag sgAmexTagList[] =
{
	{0x9F26, DE55_LEN_FIXED, 8},
	{0x9F10, DE55_LEN_VAR1,  32},
	{0x9F37, DE55_LEN_FIXED, 4},
	{0x9F36, DE55_LEN_FIXED, 2},
	{0x95,   DE55_LEN_FIXED, 5},
	{0x9A,   DE55_LEN_FIXED, 3},
	{0x9C,   DE55_LEN_FIXED, 1},
	{0x9F02, DE55_LEN_FIXED, 6},
	{0x5F2A, DE55_LEN_FIXED, 2},
	{0x9F1A, DE55_LEN_FIXED, 2},
	{0x82,   DE55_LEN_FIXED, 2},
	{0x9F03, DE55_LEN_FIXED, 6},
	{0x5F34, DE55_LEN_FIXED, 1},
	{0x9F27, DE55_LEN_FIXED, 1},
	{0x9F06, DE55_LEN_VAR1,  16},
	{0x9F09, DE55_LEN_FIXED, 2},
	{0x9F34, DE55_LEN_FIXED, 3},
	{0x9F0E, DE55_LEN_FIXED, 5},
	{0x9F0F, DE55_LEN_FIXED, 5},
	{0x9F0D, DE55_LEN_FIXED, 5},
	{0},
};

// 消费/(预)授权,55域EMV标签, TLV format
// tags data in field 55 of transaction sale/(pre-)authorization (TLV format)
static DE55Tag sgStdEmvTagList[] =
{
	{0x82,   DE55_MUST_SET, 0},
	{0x84,   DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9A,   DE55_MUST_SET, 0},
	{0x9C,   DE55_MUST_SET, 0},
	{0x5F2A, DE55_MUST_SET, 0},
	{0x5F30, DE55_OPT_SET,	0},
	{0x5F34, DE55_MUST_SET, 1}, // notice it's limited to L=1
	{0x9F02, DE55_MUST_SET, 0},
	{0x9F03, DE55_MUST_SET, 0},
	{0x9F06, DE55_MUST_SET, 0},
	{0x9F09, DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F1A, DE55_MUST_SET, 0},
	{0x9F1E, DE55_MUST_SET, 0},
	{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0x9F33, DE55_MUST_SET, 0},
	{0x9F34, DE55_MUST_SET, 0},
	{0x9F35, DE55_MUST_SET, 0},
	{0x9F36, DE55_MUST_SET, 0},
	{0x9F37, DE55_MUST_SET, 0},
	{0x9F41, DE55_MUST_SET, 0},
	//{0x9F53, DE55_MUST_SET, 0},
	{0x8E,	 DE55_MUST_SET, 0},
	{0},
};

// 消费/(预)授权,56域EMV标签, TLV format
static DE55Tag sgStdEmvField56TagList[] =
{
	{0x5A,   DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9B,   DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0},
};

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// This is NOT a callback of EMV lib
void AppSetMckParam(uchar bEnablePinBypass)
{
	//在交易处理前调用接口函数  EMVSetMCKParam 设置是否使用批数据采集
	EMV_MCKPARAM	stMckParam;
	EMV_EXTMPARAM	stExtMckParam;

	stMckParam.ucBypassPin = (bEnablePinBypass ? 1 : 0);
	stMckParam.ucBatchCapture = 1;
	memset(&stExtMckParam, 0, sizeof(stExtMckParam));
	stExtMckParam.ucUseTermAIPFlg = 1;
	stExtMckParam.aucTermAIP[0] = 0x08;	// always perform term risk management
	stExtMckParam.ucBypassAllFlg = 1;
	stMckParam.pvoid = &stExtMckParam;
	EMVSetMCKParam(&stMckParam);
}

// Set to default EMV parameter, since it may be changed during last transaction.
void InitTransEMVCfg(void)
{
	//在交易处理前调用接口函数  EMVSetMCKParam 设置是否使用批数据采集
	AppSetMckParam(TRUE);

	EMVGetParameter(&glEmvParam);
	glEmvParam.ForceOnline = 0;
	memcpy(glEmvParam.CountryCode, glPosParams.currency.sCountryCode, 2);
	memcpy(glEmvParam.TransCurrCode, glPosParams.currency.sCurrencyCode, 2);
	memcpy(glEmvParam.ReferCurrCode, glPosParams.currency.sCurrencyCode, 2);
	glEmvParam.TransCurrExp = glPosParams.currency.ucDecimal;
	glEmvParam.ReferCurrExp = glPosParams.currency.ucDecimal;
	EMVSetParameter(&glEmvParam);
}

// Modified by Kim_LinHB 2014-5-31
// for displaying a application list to card holder to select
// if there is only one application in the chip, then EMV kernel will not call this callback function
int cEMVWaitAppSel(int TryCnt, EMV_APPLIST List[], int AppNum)
{
	logTrace("==%s==", __func__);
	int			iRet, iCnt, iAppCnt;
	GUI_MENU		stAppMenu;
	GUI_MENUITEM	stAppMenuItem[MAX_APP_NUM + 1];
	APPLABEL_LIST	stAppList[MAX_APP_NUM];
	int iSelected = 0;

	if (TryCnt != 0)
	{
		unsigned char szBuff[200];
		sprintf(szBuff, "%s\n%s", _T("NOT ACCEPT"), _T("PLS TRY AGAIN"));
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);
	}

	EMVGetLabelList(stAppList, &iAppCnt);

	PubASSERT(AppNum <= MAX_APP_NUM);
	memset(stAppMenuItem, 0, sizeof(stAppMenuItem));
	for (iCnt = 0; iCnt < iAppCnt && iCnt < MAX_APP_NUM; iCnt++)
	{
		stAppMenuItem[iCnt].bVisible = TRUE;
		stAppMenuItem[iCnt].nValue = iCnt;
		stAppMenuItem[iCnt].vFunc = NULL;
		sprintf((char *)stAppMenuItem[iCnt].szText, "%.16s", stAppList[iCnt].aucAppLabel);
	}
	strcpy(stAppMenuItem[iCnt].szText, "");

	Gui_BindMenu(_T("Select App"), gl_stCenterAttr, gl_stLeftAttr, (GUI_MENUITEM *)stAppMenuItem, &stAppMenu);
	Gui_ClearScr();
	iSelected = 0;
	iRet = Gui_ShowMenuList(&stAppMenu, 0, USER_OPER_TIMEOUT, &iSelected);
	if (iRet != GUI_OK)
	{
		return EMV_USER_CANCEL;
	}

	return iSelected;
}

// it is acallback function for EMV kernel, 
// for displaying a amount input box,  
// developer customize
int cEMVInputAmount(ulong *AuthAmt, ulong *CashBackAmt)
{
	uchar	szTotalAmt[20];
	uchar   szBuff[32];

	if (glProcInfo.stTranLog.szAmount[0] != 0)
	{
		PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount, 12, szTotalAmt);
		logTrace("szAmount: %s", glProcInfo.stTranLog.szAmount);

		*AuthAmt = atol((char *)szTotalAmt);
		if (CashBackAmt != NULL)
		{
			*CashBackAmt = atol(glProcInfo.stTranLog.szOtherAmount);
		}
	}
	else
	{
		*AuthAmt = 0L;
		if (CashBackAmt != NULL)
		{
			*CashBackAmt = atol(glProcInfo.stTranLog.szOtherAmount);
		}
	}

	if (glProcInfo.stTranLog.ucTranType == CASH)
	{
		if (CashBackAmt == NULL)
		{
			if ((EMVReadVerInfo(szBuff) == EMV_OK) && (memcmp(szBuff, "v2", 2) == 0))
			{
				// For EMV2x, "v28_7" etc. Not for EMV4xx
				// Set cash back amount
				EMVSetTLVData(0x9F03, (uchar *)"\x00\x00\x00\x00\x00\x00", 6);
				EMVSetTLVData(0x9F04, (uchar *)"\x00\x00\x00\x00", 4);
			}
		}
	}

	return EMV_OK;
}

// 处理DOL的过程中，EMV库遇到不识别的TAG时会调用该回调函数，要求应用程序处理
// 如果应用程序无法处理，则直接返回-1，提供该函数只为解决一些不符合EMV的特殊
// 应用程序的要求，一般情况下返回-1即可
// Callback function required by EMV core.
// When processing DOL, if there is a tag that EMV core doesn't know about, core will call this function.
// developer should offer processing for proprietary tag.
// if really unable to, just return -1
int cEMVUnknowTLVData(ushort iTag, uchar *psDat, int iDataLen)
{
	switch (iTag)
	{
		/*
		'C' = CASH DESBUS
		'Z' = ATM CASH
		'O' = COLLEGE SCHOOL
		'H' = HOTEL/SHIP
		'X' = TRANSFER
		'A' = AUTO MOBILE/RENTAL
		'F' = RESTAURANT
		'T' = TELEPHONE ORDER PREAUTH
		'U' = UNIQUE TRANS
		'R' = RENTAL/OTHER TRANS
		*/
	case 0x9F53:		// Transaction Category Code (TCC) - Master Card
		*psDat = 'R';	// 0x52;
		break;

	default:
		return -1;
	}

	return EMV_OK;
}

// Callback function required by EMV core.
// Wait holder enter PIN.
// developer customized.
// Modified by Kim_LinHB 2014-6-8 v1.01.0000
int cEMVGetHolderPwd(int iTryFlag, int iRemainCnt, uchar *pszPlainPin)
{
	logTrace("==%s==", __func__);
	int		iResult;
	uchar	ucRet, szBuff[30], szAmount[15];
	uchar	sPinBlock[8];

	// online PIN
	if (pszPlainPin == NULL)
	{
		iResult = GetPIN(GETPIN_EMV);
		if (iResult == 0)
		{
			if (glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT)
			{
				logd(("Online pin entered"));
				return EMV_OK;
			}
			else
			{
				return EMV_NO_PASSWORD;
			}
		}
		else if (iResult == ERR_USERCANCEL)
		{
			return EMV_USER_CANCEL;
		}
		else
		{
			return EMV_NO_PINPAD;
		}
	}

	// Offline plain/enciphered PIN processing below
	Gui_ClearScr();
	if (iRemainCnt == 1)
	{
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("LAST PIN TRY"), gl_stCenterAttr, GUI_BUTTON_NONE, 2, NULL);
	}

	PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount, 12, szAmount);
	Gui_ClearScr();
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	if (iTryFlag == 0)
	{
		GetDispAmount(szAmount, szAmount);
		Gui_DrawText(szAmount, gl_stCenterAttr, 0, 25);
	}
	else
	{
		Gui_DrawText(_T("INCORRECT PIN"), gl_stCenterAttr, 0, 25);
	}

ENTERPIN:
	Gui_DrawText(_T("ENTER PIN"), gl_stCenterAttr, 0, 50);

	if (ChkTermPEDMode(PED_INT_PCI))
	{

		// Offline PIN, done by core itself since EMV core V25_T1. Application only needs to display prompt message.
		// In this mode, cEMVGetHolderPwd() will be called twice. the first time is to display message to user,
		// then back to kernel and wait PIN. afterwards kernel call this again and inform the process result.
		if (pszPlainPin[0] == EMV_PED_TIMEOUT)
		{
			// EMV core has processed PIN entry and it's timeout
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PED ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
			return EMV_TIME_OUT;
		}
		else if (pszPlainPin[0] == EMV_PED_WAIT)
		{
			// API is called too frequently
			DelayMs(1000);
			Gui_ClearScr();
			// Modified by Kim_LinHB 2014-8-11 v1.01.0003
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

			ScrGotoxy(32, 6);
			return EMV_OK;
		}
		else if (pszPlainPin[0] == EMV_PED_FAIL)
		{
			// EMV core has processed PIN entry and PED failed.
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PED ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);

			return EMV_NO_PINPAD;
		}
		else
		{
			// EMV PIN not processed yet. So just display.
			ScrGotoxy(32, 6);
			return EMV_OK;
		}
	}
	else if (ChkTermPEDMode(PED_EXT_PP))
	{
		Gui_DrawText(_T("PLS USE PINPAD"), gl_stCenterAttr, 0, 75);
		App_ConvAmountTran(szAmount, szBuff, 0);
		// show amount on PINPAD
		ucRet = PPScrCls();
		if (ucRet)
		{
			return EMV_NO_PINPAD;
		}
		PPScrPrint(0, 0, szBuff);
		PPScrClrLine(1);

		memset(sPinBlock, 0, sizeof(sPinBlock));
		ucRet = PPEmvGetPwd(4, 12, sPinBlock);
		switch (ucRet)
		{
		case 0x00:
			// PinBlock Format: C L P P P P P/F P/F P/F P/F P/F P/F P/F P/F F F
			// C = 0x02, L = length of PIN, P = PIN digits, F = 0x0F
			PubBcd2Asc0(sPinBlock, 8, szBuff);
			PubTrimTailChars(szBuff, 'F');
			sprintf((char *)pszPlainPin, "%.12s", &szBuff[2]);
			glProcInfo.stTranLog.uiEntryMode |= MODE_OFF_PIN;
			return EMV_OK;

		case 0x06:
		case 0x07:
		case 0xC6:
			return EMV_USER_CANCEL;

		case 0x0A:
			if (!ChkIssuerOption(ISSUER_EN_EMVPIN_BYPASS) && ChkIfAmex())
			{
				PPScrCls();
				PPScrPrint(1, 0, " NOT PERMITTED");
				PPBeep();

				Gui_ClearScr();
				Beef(6, 200);
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("NOT PERMITTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);

				goto ENTERPIN;
			}
			else
			{
				return EMV_NO_PASSWORD;
			}

		default:
			return EMV_NO_PINPAD;
		}
	}
	else	// PED_EXT_PCI
	{
		// !!!! extern PCI, to be implemented.
		unsigned char szBuff[200];
		sprintf(szBuff, "%s\n%s", _T("EXT PCI PINPAD"), _T("NOT IMPLEMENTED."));
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
		return ERR_NO_DISP;
	}
}

// 语音参考处理
// 如果不支持，如果是卡片发起的参考，
// 则可根据发卡行要求直接返回REFER_DENIAL或REFER_ONLINE,
// 一般情况下不应该直接返回REFER_APPROVE(除非发卡行要求这么做)

// 如果不支持，如果是发卡行(主机)发起的参考，
// 则可根据发卡行要求直接返回REFER_DENIAL
// 一般情况下不应该直接返回REFER_APPROVE(除非发卡行要求这么做)

// 下边是参考的处理代码，供参考
// Callback function required by EMV core.
// Voice referral.
// If not support, return REFER_DENIAL.
int cEMVReferProc(void)
{
	logTrace("==%s==", __func__);
	return REFER_DENIAL;
}

// Callback function required by EMV core.
// TC advise after EMV transaction. If not support, immediately return.
void cEMVAdviceProc(void)
{
	logTrace("==%s==", __func__);
	//	脱机通知的处理：
	//	通过函数EMVGetTLVData()获得通知数据包需要的数据，存贮到交易日志中，
	//	然后在交易结算时，再联机发送到主机。
	//	需要注意的是：通知数据包的任何数据(比如金额等)不可以用于交易对帐。

	//	联机通知的处理：
	//	(1)拨号连接主机。
	//	(2)通过函数EMVGetTLVData()获得通知数据包需要的数据，再发送到主机。
	//	需要注意的是：联机通知方式在我们的POS机中应该不会使用。
}

//联机处理
/*
	处理步骤：
	(1)拨号连接主机,如果所有交易都要联机，那么可以在插入IC卡时预拨号,
	   如果拨号失败返回ONLINE_FAILED
	(2)通过函数EMVGetTLVData()获得交易数据包需要的数据，并打包。
	(3)保存冲正数据及标志,然后发送交易数据包到主机(冲正处理完全由应用完成)
	(4)接收主机的回应数据包,根据主机的回应，做如下返回：
	   A.如果主机返回批准，则根据返回数据填写RspCode、AuthCode、AuthCodeLen等
		 参数的值，并返回ONLINE_APPROVE
	   B.如果主机拒绝交易,则根据返回数据填写RspCode,如果其他参数也有数据值，
		 同样需要填写，返回ONLINE_DENIAL
	   C.如果主机请求语音参考,则根据返回数据填写RspCode,如果其他参数也有数据值，
		 同样需要填写，返回ONLINE_REFER。需要说明的是：很多情况可能没有参考处理，
		 在这种情况下，应用程序就不需要返回ONLINE_REFER了

	等交易处理成功后，应用程序才可以清除冲正标志。
*/
/* Online processing.
	steps:
	(1) Dial. If dial failed, return ONLINE_FAILED
	(2) Use EMVGetTLVData() to retrieve data from core, pack to ISO8583.
	(3) Save reversal data and flag, then send request to host
	(4) Receive from host, then do accordingly:
	   A. If host approved, copy RspCode,AuthCode,AuthCodeLen or so, and return ONLINE_APPROVE
	   B. If host denial, copy RspCode or so, return ONLINE_DENIAL
	   C. If host require voice referral, copy RspCode or so.,return ONLINE_REFER.
		   Note that if not support, needn't return ONLINE_REFER but directly ONLINE_DENIAL

	Reversal flag can only be cleared after all EMV processing, NOT immediately after online.
*/
int  cEMVOnlineProc(uchar *psRspCode, uchar *psAuthCode, int *piAuthCodeLen,
	uchar *psAuthData, int *piAuthDataLen,
	uchar *psScript, int *piScriptLen)
{
	logTrace("==%s==", __func__);
	int		iRet, iLength, iRetryPIN;
	ulong	ulICCDataLen;
	uchar	*psICCData, *psTemp;

	// initialize output parameters
	*psRspCode = 0;
	*piAuthCodeLen = 0;
	*piAuthDataLen = 0;
	*piScriptLen = 0;
	SaveTVRTSI(TRUE);
	glProcInfo.bIsFirstGAC = FALSE;

	UpdateEntryModeForOfflinePIN();

	// prepare online DE55 data
	memset(glProcInfo.stTranLog.sIccData, 0, lengthOf(glProcInfo.stTranLog.sIccData));
	iRet = SetDE55(DE55_SALE, glProcInfo.stTranLog.sIccData, &iLength);
	if (iRet != 0)
	{
		glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
		return ONLINE_FAILED;
	}
	glProcInfo.stTranLog.uiIccDataLen = iLength;
	logHexString("ICC Data ", glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);

	iRet = processNibssTransaction();
	if (iRet != 0)
	{
		if (iRet < 0) {
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			return ONLINE_FAILED;
		}

		iRet = rollbackNibssTransaction(REASON_TIME_OUT);
		if (iRet < 0) {
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			return ONLINE_FAILED;
		}
	}

	// set response code
	memcpy(psRspCode, glProcInfo.stTranLog.szRspCode, LEN_RSP_CODE);
	glProcInfo.ucOnlineStatus = ST_ONLINE_APPV;

	// get response issuer data
	sgAuthDataLen = sgScriptLen = 0;

	ulICCDataLen = glProcInfo.uiResponseIccLen;
	psICCData = glProcInfo.sResponseIcc;
	for (psTemp = psICCData; psTemp < psICCData + ulICCDataLen; ) {
		iRet = GetTLVItem(&psTemp, psICCData + ulICCDataLen - psTemp, SaveRspICCData, FALSE);
	}

	memcpy(psAuthData, sAuthData, sgAuthDataLen);
	*piAuthDataLen = sgAuthDataLen;

	// version 1.00.0016 change by Jolie Yang at 2013-08-16
	// due to the application need not extract the sub-tag of 71/72, just get contents of 71\72, and transfer to EMV kernal
	// AdjustIssuerScript();
	LOG_HEX_PRINTF("Issuer script", sIssuerScript, sgScriptLen);
	logd(("Issuer Scripts: %s, length: %d", sIssuerScript, sgScriptLen));

	memcpy(psScript, sIssuerScript, sgScriptLen);
	*piScriptLen = sgScriptLen;

	if (memcmp(glProcInfo.stTranLog.szRspCode, "00", LEN_RSP_CODE) != 0)
	{
		return ONLINE_DENIAL;
	}
	// set authorize code only if txn approved
	memcpy(psAuthCode, glProcInfo.stTranLog.szAuthCode, LEN_AUTH_CODE);
	*piAuthCodeLen = strlen((char *)glProcInfo.stTranLog.szAuthCode);

	return ONLINE_APPROVE;
}

// 如果不需要提示密码验证成功，则直接返回就可以了
// Callback function required by EMV core.
// Display "EMV PIN OK" info. (plaintext/enciphered PIN)
// Modified by Kim_LinHB 2014-6-8 v1.01.0000
void cEMVVerifyPINOK(void)
{
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PIN OK"), gl_stCenterAttr, GUI_BUTTON_NONE, 1, NULL);
}

// 持卡人认证例程
// Callback function required by EMV core.
// Don't need to care about this function
int cCertVerify(void)
{
	//	AppSetMckParam(!ChkIssuerOption(ISSUER_EN_EMVPIN_BYPASS));
	return -1;
}

// Callback function required by EMV core.
// in EMV ver 2.1+, this function is called before GPO
int cEMVSetParam(void)
{
	return 0;
}

int FinishEmvTran(void)
{
	logTrace(__func__);
	int		iRet, iLength;
	uchar	ucSW1, ucSW2;

	logd(("Tran Type: %d", glProcInfo.stTranLog.ucTranType));
	// 根据需要设置是否强制联机
	// decide whether need force online
	EMVGetParameter(&glEmvParam);
	//glEmvParam.ForceOnline = 1;// ((glProcInfo.stTranLog.ucTranType != SALE) ? 1 : 0);
	//EMVSetParameter(&glEmvParam);
	// Only in this transaction, so DON'T back up

	// clear last EMV status
	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();
	if (ChkTermPEDMode(PED_INT_PCI))
	{
		iRet = EMVSetPCIModeParam(1, (uchar *)"4,5,6,7,8,9,10,11,12", 120000);
	}

	// Process EMV transaction.
	iRet = EMVProcTrans();
	if (iRet != EMV_OK) {
		logd(("EMV process failed:: %d", iRet));
		int debug_info = -1;
		EMVGetDebugInfo(0, NULL, &debug_info);
		logd(("Debug Info: %d", debug_info));
	}
	SaveTVRTSI(FALSE);
	UpdateEntryModeForOfflinePIN();
	if (iRet == EMV_TIME_OUT || iRet == EMV_USER_CANCEL)
	{
		logd(("EMV user cancelled"));
		return ERR_USERCANCEL;
	}

	if ((glProcInfo.ucOnlineStatus == ST_ONLINE_APPV)) {
		Gui_ClearScr();
		if (isSuccessResponse(glProcInfo.stTranLog.szRspCode)) {
			PubBeepOk();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, "APPROVED", gl_stCenterAttrAlt, GUI_BUTTON_NONE, 2, NULL);
		}
		else {
			PubBeepErr();
			DispErrMsg("DECLINED", responseCodeToString(glProcInfo.stTranLog.szRspCode), 10, 0);
		}

	}

	if ((glProcInfo.ucOnlineStatus == ST_ONLINE_APPV) && memcmp(glProcInfo.stTranLog.szRspCode, "00", 2) == 0)
	{
		SetDE55(DE55_SALE, glProcInfo.stTranLog.sIccData, &iLength);
		glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
	}

	EMVGetTLVData(0x9F26, glProcInfo.stTranLog.sAppCrypto, &iLength);
	EMVGetTLVData(0x95, glProcInfo.stTranLog.sTVR, &iLength);
	EMVGetTLVData(0x9B, glProcInfo.stTranLog.sTSI, &iLength);

	if (iRet != EMV_OK)
	{
		if (glProcInfo.ucOnlineStatus != ST_OFFLINE && isSuccessResponse(glProcInfo.stTranLog.szRspCode)) { //host approved, but declined offline, rollback transaction
			rollbackNibssTransaction(REASON_TIME_OUT);
		}

		SaveEmvErrLog();
		EMVGetVerifyICCStatus(&ucSW1, &ucSW2);
		if (glProcInfo.bIsFirstGAC && ucSW1 == 0x69 && ucSW2 == 0x85 &&
			glProcInfo.stTranLog.szPan[0] == '5')
		{	// for TIP fallback when 1st GAC return 6985
			return ERR_NEED_FALLBACK;
		}

		SetDE55(DE55_SALE, glProcInfo.stTranLog.sIccData, &iLength);
		glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;

		if (glProcInfo.stTranLog.szRspCode[0] != 0 &&
			memcmp(glProcInfo.stTranLog.szRspCode, "00", 2) != 0)
		{	// show reject code from host
			return 0; //AfterTranProc();
		}
		return ERR_TRAN_FAIL;
	}

	// transaction approved. save EMV data
	SaveEmvData();
	if (glProcInfo.ucOnlineStatus != ST_ONLINE_APPV)
	{
		return FinishOffLine();
	}


	return 0; //AfterTranProc();
}

int FinishOffLine(void)
{
	SetOffBase(OffBaseDisplay);

	DispProcess();

	sprintf((char *)glProcInfo.stTranLog.szRspCode, "00");

	DoE_Signature();


	DispResult(0);
	// PubWaitKey(glSysParam.stEdcInfo.ucAcceptTimeout); // Hidden by Kim_LinHB 2014/9/11 v1.01.0008 bug523

	return 0;
}



int FinishSwipeTran(void)
{
	logTrace(__func__);
	int		iRet, iLength;

	iRet = GetPIN(GETPIN_EMV);
	if (iRet != 0) {
		return iRet;
	}

	iRet = processNibssTransaction();
	if (iRet != 0)
	{
		if (iRet < 0) {
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			return ERR_TRAN_FAIL;
		}

		iRet = rollbackNibssTransaction(REASON_TIME_OUT);
		if (iRet < 0) {
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			return ONLINE_FAILED;
		}
	}

	Gui_ClearScr();
	if (isSuccessResponse(glProcInfo.stTranLog.szRspCode)) {
		PubBeepOk();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, "APPROVED", gl_stCenterAttrAlt, GUI_BUTTON_NONE, 2, NULL);
	}
	else {
		PubBeepErr();
		DispErrMsg("DECLINED", responseCodeToString(glProcInfo.stTranLog.szRspCode), 10, 0);
	}


	return 0; //AfterTranProc();
}

// Set bit 55 data for online transaction.
int SetDE55(DE55_TYPE ucType, uchar *psOutData, int *piOutLen)
{
	/*if( ChkIfAmex() )
	{
		return SetAmexDE55(sgAmexTagList, psOutData, piOutLen);
	}
	else
	{*/
	return SetStdDE55((uchar)ucType, sgStdEmvTagList, psOutData, piOutLen);
	//}
}

// set AMEX bit 55, structure of TLV items
int SetAmexDE55(const DE55Tag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt, iLength;
	uchar	*psTemp, sBuff[128];

	*piOutLen = 0;
	memcpy(psOutData, "\xC1\xC7\xD5\xE2\x00\x01", 6);	// AMEX header
	psTemp = psOutData + 6;

	for (iCnt = 0; pstList[iCnt].uiEmvTag != 0; iCnt++)
	{
		iLength = 0;
		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMVGetTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
		if ((iRet != EMV_OK) && (iRet != EMV_NO_DATA))
		{
			return ERR_TRAN_FAIL;
		}

		if (iRet == EMV_NO_DATA)
		{
			iLength = pstList[iCnt].ucLen;
			memset(sBuff, 0, sizeof(sBuff));
		}

		if (pstList[iCnt].ucOption == DE55_LEN_VAR1)
		{
			*psTemp++ = (uchar)iLength;
		}
		else if (pstList[iCnt].ucOption == DE55_LEN_VAR2)
		{
			*psTemp++ = (uchar)(iLength >> 8);
			*psTemp++ = (uchar)iLength;
		}
		memcpy(psTemp, sBuff, iLength);
		psTemp += iLength;
	}
	*piOutLen = (psTemp - psOutData);

	return 0;
}

// this function will not check the overflow risk of array pointed by pstList.
int AppendStdTagList(DE55Tag *pstList, ushort uiTag, uchar ucOption, uchar ucMaxLen)
{
	int	iCnt;

	iCnt = 0;
	while (pstList[iCnt].uiEmvTag != 0)
	{
		iCnt++;
	}
	pstList[iCnt].uiEmvTag = uiTag;
	pstList[iCnt].ucOption = ucOption;
	pstList[iCnt].ucLen = ucMaxLen;
	pstList[iCnt + 1].uiEmvTag = 0;
	pstList[iCnt + 1].ucOption = 0;
	pstList[iCnt + 1].ucLen = 0;
	return 0;
}

int RemoveFromTagList(DE55Tag *pstList, ushort uiTag)
{
	int	iCnt;

	for (iCnt = 0; pstList[iCnt].uiEmvTag != 0; iCnt++)
	{
		if (pstList[iCnt].uiEmvTag == uiTag)
		{
			break;
		}
	}
	if (pstList[iCnt].uiEmvTag == 0)
	{
		return -1;
	}

	for (; pstList[iCnt].uiEmvTag != 0; iCnt++)
	{
		pstList[iCnt] = pstList[iCnt + 1];
	}

	return 0;
}

// set ADVT/TIP bit 55
int SetStdDE55(uchar bForUpLoad, const DE55Tag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt, iLength;
	uchar	*psTemp, sBuff[200];
	DE55Tag	astLocalTaglist[64];

	*piOutLen = 0;
	psTemp = psOutData;

	if (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT)
	{
		// Copy from std tag list
		//-----------------------------------------------------------
		memset(astLocalTaglist, 0, sizeof(astLocalTaglist));
		for (iCnt = 0; pstList[iCnt].uiEmvTag != 0; iCnt++)
		{
			astLocalTaglist[iCnt] = pstList[iCnt];
		}
		//-----------------------------------------------------------
		// Generate data by tag list
		for (iCnt = 0; pstList[iCnt].uiEmvTag != 0; iCnt++)
		{
			memset(sBuff, 0, sizeof(sBuff));

			if (pstList[iCnt].uiEmvTag == 0x9F03) {
				memset(sBuff, 0, sizeof(sBuff));
				PubAsc2Bcd(glProcInfo.stTranLog.szOtherAmount, 12, sBuff);
				iLength = 6;
				BuildTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
				continue;
			}
			iRet = EMVGetTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
			if (iRet == EMV_OK)
			{
				if ((pstList[iCnt].ucLen > 0) && (iLength > pstList[iCnt].ucLen))
				{
					iLength = pstList[iCnt].ucLen;
				}
				BuildTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
			}
			else if (pstList[iCnt].ucOption == DE55_MUST_SET)
			{
				BuildTLVString(pstList[iCnt].uiEmvTag, NULL, 0, &psTemp);
			}
		}

		//-----------------------------------------------------------
		// Generate custom tag content
		if (glProcInfo.stTranLog.szPan[0] == '5')
		{	// for master card TCC = "R" -- retail
			BuildTLVString(0x9F53, (uchar *)"R", 1, &psTemp);
		}

		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMVGetScriptResult(sBuff, &iLength);
		if (iRet == EMV_OK)
		{
			BuildTLVString(0xDF5B, sBuff, iLength, &psTemp);
		}
	}
	else
	{
		return 0;
	}

	*piOutLen = (psTemp - psOutData);

	return 0;
}

int SetTCDE55(void *pstTranLog, uchar *psOutData, int *piOutLen)
{
	char    sBuff[LEN_ICC_DATA];
	ushort  uiLen;
	int     iRet;

	if (ChkIfICBC_MACAU())
	{
		// ICBC-Macau only need 9F26 in TC DE55
		*piOutLen = 0;
		iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9F26, sBuff, &uiLen);
		if (iRet == 0)
		{
			memcpy(psOutData, sBuff, uiLen);
			psOutData += uiLen;
			*piOutLen += uiLen;
		}
		return 0;
	}
	else if (ChkIfDah() || ChkIfWingHang())
	{
		*piOutLen = 0;
		iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9F26, sBuff, &uiLen);
		if (iRet == 0)
		{
			memcpy(psOutData, sBuff, uiLen);
			psOutData += uiLen;
			*piOutLen += uiLen;
		}
		iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9F27, sBuff, &uiLen);
		if (iRet == 0)
		{
			memcpy(psOutData, sBuff, uiLen);
			psOutData += uiLen;
			*piOutLen += uiLen;
		}
		return 0;
	}

	*piOutLen = ((TRAN_LOG *)pstTranLog)->uiIccDataLen;
	memcpy(psOutData, ((TRAN_LOG *)pstTranLog)->sIccData, *piOutLen);
	return 0;
}

//Set 56 field
int SetDE56(uchar *psOutData, int *piOutLen)
{
	*piOutLen = 0;
	if (ChkIfAmex())
	{
		return 0;
	}

	return SetStdDE56(sgStdEmvField56TagList, psOutData, piOutLen);
}

int SetStdEmptyDE56(uchar *psOutData, int *piOutLen)
{
	if (ChkIfAmex())
	{
		*piOutLen = 0;
		return 0;
	}

	if (ChkIfBea())
	{
		memcpy(psOutData, "\xDF\xF0\x0D\x00\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20", 16);
		*piOutLen = 16;
	}
	else if (ChkIfBoc() || ChkIfFubon())
	{
		memcpy(psOutData, "\xDF\xF0\x07\x00\x20\x20\x20\x20\x20\x20", 10);
		*piOutLen = 10;
	}
	else
	{
		memcpy(psOutData, "\xDF\x5C\x07\x00\x20\x20\x20\x20\x20\x20", 10);
		*piOutLen = 10;
	}

	return 0;
}

int SetStdDE56(const DE55Tag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt, iLength;
	uchar	*psTemp, sBuff[110];

	// build header of bit 56
	*piOutLen = 0;
	psTemp = psOutData;
	if (ChkIfBea())
	{
		memcpy(psTemp, "\xDF\xF0\x0D\x01", 4);
		psTemp += 4;
		PubLong2Bcd(glProcInfo.stTranLog.ulInvoiceNo, 3, psTemp);
		psTemp += 3;
		PubLong2Bcd(glProcInfo.stTranLog.ulSTAN, 3, psTemp);
		psTemp += 3;
		PubAsc2Bcd(glProcInfo.stTranLog.szRRN, 12, psTemp);
		psTemp += 6;
	}
	else
	{
		if (ChkIfBoc() || ChkIfFubon())
		{
			memcpy(psTemp, "\xDF\xF0\x07\x01", 4);
		}
		else
		{
			memcpy(psTemp, "\xDF\x5C\x07\x01", 4);
		}
		psTemp += 4;

		PubLong2Bcd(glProcInfo.stTranLog.ulInvoiceNo, 3, psTemp);
		psTemp += 3;
		PubLong2Bcd(glProcInfo.stTranLog.ulSTAN, 3, psTemp);
		psTemp += 3;
	}

	// build common EMV core tags for all HK banks
	for (iCnt = 0; pstList[iCnt].uiEmvTag != 0; iCnt++)
	{
		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMVGetTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
		if (iRet == EMV_OK)
		{
			BuildTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
		}
		else
		{
			BuildTLVString(pstList[iCnt].uiEmvTag, NULL, 0, &psTemp);
		}
	}

	// process special EMC core tags for different banks
	if (ChkIfFubon())
	{
		memset(sBuff, 0, sizeof(sBuff));
		EMVGetTLVData(0x9A, sBuff, &iLength);
		BuildTLVString(0x9A, sBuff, iLength, &psTemp);
	}
	if (ChkIfHSBC())
	{
		memset(sBuff, 0, sizeof(sBuff));
		EMVGetTLVData(0x9F36, sBuff, &iLength);
		BuildTLVString(0x9F36, sBuff, iLength, &psTemp);
	}

	memset(sBuff, 0, sizeof(sBuff));
	iRet = EMVGetScriptResult(sBuff, &iLength);
	if (iRet != EMV_OK)
	{
		*piOutLen = (psTemp - psOutData);
		return 0;
	}

	// continue issuer script result process
	if (ChkIfBoc() || ChkIfFubon() || ChkIfBea())
	{
		memcpy(psTemp, "\xDF\x91", 2);
	}
	else if (ChkIfDah() || ChkIfScb() || ChkIfCiti())
	{
		memcpy(psTemp, "\x9F\x5B", 2);
	}
	else
	{
		memcpy(psTemp, "\xDF\x5B", 2);
	}
	psTemp += 2;
	*psTemp++ = (uchar)iLength;
	memcpy(psTemp, sBuff, iLength);
	psTemp += iLength;

	*piOutLen = (psTemp - psOutData);

	return 0;
}

// bExpandAll:       TRUE: expand constructed item, FALSE: not
int GetTLVItem(uchar **ppsTLVString, int iMaxLen, SaveTLVData pfSaveData, uchar bExpandAll)
{
	int			iRet;
	uchar		*psTag, *psSubTag;
	uint		uiTag, uiLenBytes;
	ulong		lTemp;

	// skip null tags
	for (psTag = *ppsTLVString; psTag < *ppsTLVString + iMaxLen; psTag++)
	{
		if ((*psTag != TAG_NULL_1) && (*psTag != TAG_NULL_2))
		{
			break;
		}
	}
	if (psTag >= *ppsTLVString + iMaxLen)
	{
		*ppsTLVString = psTag;
		return 0;	// no tag available
	}

	// process tag bytes
	uiTag = *psTag++;
	if ((uiTag & TAGMASK_FIRSTBYTE) == TAGMASK_FIRSTBYTE)
	{	// have another byte
		uiTag = (uiTag << 8) + *psTag++;
	}
	if (psTag >= *ppsTLVString + iMaxLen)
	{
		return -1;
	}

	// process length bytes
	if ((*psTag & LENMASK_NEXTBYTE) == LENMASK_NEXTBYTE)
	{
		uiLenBytes = *psTag & LENMASK_LENBYTES;
		lTemp = PubChar2Long(psTag + 1, uiLenBytes);
	}
	else
	{
		uiLenBytes = 0;
		lTemp = *psTag & LENMASK_LENBYTES;
	}
	psTag += uiLenBytes + 1;
	if (psTag + lTemp > *ppsTLVString + iMaxLen)
	{
		return -2;
	}
	*ppsTLVString = psTag + lTemp;	// advance pointer of TLV string

	// save data
	(*pfSaveData)(uiTag, psTag, (int)lTemp);
	if (!IsConstructedTag(uiTag) || !bExpandAll)
	{
		return 0;
	}

	// constructed data
	for (psSubTag = psTag; psSubTag < psTag + lTemp; )
	{
		iRet = GetTLVItem(&psSubTag, psTag + lTemp - psSubTag, pfSaveData, TRUE);
		if (iRet < 0)
		{
			return iRet;
		}
	}

	return 0;
}

int GetSpecTLVItem(const uchar *psTLVString, int iMaxLen, uint uiSearchTag, uchar *psOutTLV, ushort *puiOutLen)
{
	uchar		*psTag, *psTagStr;
	uint		uiTag, uiLenBytes;
	ulong		lTemp;

	// skip null tags
	for (psTag = (uchar *)psTLVString; psTag < psTLVString + iMaxLen; psTag++)
	{
		if ((*psTag != TAG_NULL_1) && (*psTag != TAG_NULL_2))
		{
			break;
		}
	}
	if (psTag >= psTLVString + iMaxLen)
	{
		return -1;	// no tag available
	}

	while (1)
	{
		psTagStr = psTag;
		// process tag bytes
		uiTag = *psTag++;
		if ((uiTag & TAGMASK_FIRSTBYTE) == TAGMASK_FIRSTBYTE)
		{	// have another byte
			uiTag = (uiTag << 8) + *psTag++;
		}
		if (psTag >= psTLVString + iMaxLen)
		{
			return -2;	// specific tag not found
		}

		// process length bytes
		if ((*psTag & LENMASK_NEXTBYTE) == LENMASK_NEXTBYTE)
		{
			uiLenBytes = *psTag & LENMASK_LENBYTES;
			lTemp = PubChar2Long(psTag + 1, uiLenBytes);
		}
		else
		{
			uiLenBytes = 0;
			lTemp = *psTag & LENMASK_LENBYTES;
		}
		psTag += uiLenBytes + 1;
		if (psTag + lTemp > psTLVString + iMaxLen)
		{
			return -2;	// specific tag not found also
		}

		// Check if tag needed
		if (uiTag == uiSearchTag)
		{
			*puiOutLen = (ushort)(psTag - psTagStr + lTemp);
			memcpy(psOutTLV, psTagStr, *puiOutLen);
			return 0;
		}

		if (IsConstructedTag(uiTag))
		{
			if (GetSpecTLVItem(psTag, (int)lTemp, uiSearchTag, psOutTLV, puiOutLen) == 0)
			{
				return 0;
			}
		}

		psTag += lTemp;	// advance pointer of TLV string
		if (psTag >= psTLVString + iMaxLen)
		{
			return -2;
		}
	}

	return 0;
}


int GetDE55Amex(const uchar *psSendHeader, const uchar *psRecvDE55, int iLen)
{
	uchar	*psTmp;
	uint	uiLenBytes;

	// invalid length
	if (iLen < 6)
	{
		return -1;
	}
	// header mismatch
	if (memcmp(psSendHeader, psRecvDE55, 6) != 0)
	{
		return -1;
	}

	psTmp = (uchar *)psRecvDE55 + 6;

	// Data Sub Field 1 : Issuer Authentication Data EMV (Tag 91)
	uiLenBytes = *psTmp++;
	if (uiLenBytes > 16)
	{
		return -2;
	}
	memcpy(sAuthData, psTmp, uiLenBytes);
	sgAuthDataLen = uiLenBytes;
	psTmp += uiLenBytes;
	if (psTmp - psRecvDE55 > iLen)
	{
		return -3;
	}
	if (psTmp - psRecvDE55 == iLen)	// end up
	{
		return 0;
	}

	// Data Sub Field 2 : Issuer Script Data
	uiLenBytes = *psTmp++;
	if (uiLenBytes > 128)
	{
		return -2;
	}
	sgScriptLen = 0;
	memcpy(&sIssuerScript[sgScriptLen], psTmp, uiLenBytes);
	sgScriptLen += uiLenBytes;
	psTmp += uiLenBytes;
	if (psTmp - psRecvDE55 > iLen)
	{
		return -3;
	}
	if (psTmp - psRecvDE55 == iLen)	// end up
	{
		return 0;
	}

	return 0;
}

int IsConstructedTag(uint uiTag)
{
	int		i;

	for (i = 0; (uiTag & 0xFF00) && i < 2; i++)
	{
		uiTag >>= 8;
	}

	return ((uiTag & TAGMASK_CONSTRUCTED) == TAGMASK_CONSTRUCTED);
}

// Save Iuuser Authentication Data, Issuer Script.
void SaveRspICCData(uint uiTag, const uchar *psData, int iDataLen)
{
	switch (uiTag)
	{
	case 0x91:
		memcpy(sAuthData, psData, MIN(iDataLen, 16));
		sgAuthDataLen = MIN(iDataLen, 16);
		break;

	case 0x71:
	case 0x72:
		sIssuerScript[sgScriptLen++] = (uchar)uiTag;
		if (iDataLen > 127)
		{
			sIssuerScript[sgScriptLen++] = 0x81;
		}
		sIssuerScript[sgScriptLen++] = (uchar)iDataLen;
		memcpy(&sIssuerScript[sgScriptLen], psData, iDataLen);
		sgScriptLen += iDataLen;
		break;

	case 0x9F36:
		//		memcpy(sATC, psData, MIN(iDataLen, 2));	// ignore
		break;

	default:
		break;
	}
}

// 只处理基本数据元素Tag,不包括结构/模板类的Tag
// Build basic TLV data, exclude structure/template.
void BuildTLVString(ushort uiEmvTag, const uchar *psData, int iLength, uchar **ppsOutData)
{
	uchar	*psTemp;

	if (iLength < 0)
	{
		return;
	}

	// set tags
	psTemp = *ppsOutData;
	if (uiEmvTag & 0xFF00)
	{
		*psTemp++ = (uchar)(uiEmvTag >> 8);
	}
	*psTemp++ = (uchar)uiEmvTag;

	// set length
	// for now, lengths of all data are less then 127 bytes, but still extend the rest part based on standard
	if (iLength <= 127)
	{
		*psTemp++ = (uchar)iLength;
	}
	else
	{
		// the upper limit is 255 bytes data defined by EMV standard
		*psTemp++ = 0x81;
		*psTemp++ = (uchar)iLength;
	}

	// set value
	if (iLength > 0)
	{
		memcpy(psTemp, psData, iLength);
		psTemp += iLength;
	}

	*ppsOutData = psTemp;
}

// Retrieve EMV data from core, for saving record or upload use.
void SaveEmvData(void)
{
	int		iLength;

	EMVGetTLVData(0x9F26, glProcInfo.stTranLog.sAppCrypto, &iLength);
	EMVGetTLVData(0x8A, glProcInfo.stTranLog.szRspCode, &iLength);
	EMVGetTLVData(0x95, glProcInfo.stTranLog.sTVR, &iLength);
	EMVGetTLVData(0x9B, glProcInfo.stTranLog.sTSI, &iLength);
	EMVGetTLVData(0x9F36, glProcInfo.stTranLog.sATC, &iLength);

	// save for upload
	SetDE55(DE55_UPLOAD, glProcInfo.stTranLog.sIccData, &iLength);
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;

	if (glProcInfo.ucOnlineStatus != ST_ONLINE_APPV)
	{
		// ICC脱机, offline approved
		SaveTVRTSI(TRUE);
		GetNewTraceNo();
		//		sprintf((char *)glProcInfo.stTranLog.szRspCode, "00");
		//		sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
		sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%06lu", glSysCtrl.ulSTAN);
		if (ChkIfAmex())
		{
			if (glProcInfo.ucOnlineStatus == ST_ONLINE_FAIL)
			{
				sprintf((char *)glProcInfo.stTranLog.szAuthCode, "Y3");
			}
			else
			{
				// for AMEX, approval code = Y1 while chip off line apporved.
				sprintf((char *)glProcInfo.stTranLog.szAuthCode, "Y1");
			}
		}
		else
		{
			sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%06lu", glSysCtrl.ulSTAN);
		}

		if (ChkIfAcqNeedDE56())
		{
			SetDE56(glProcInfo.stTranLog.sField56, &iLength);
			glProcInfo.stTranLog.uiField56Len = (ushort)iLength;
		}
	}
}

// remove at version 1.00.0016 core already fixed this bug
// core cannot process correctly if length of 9F18 is zero
// eg, 71 12 9F 18 00 86 0D 84 1E 00 00 08 11 22 33 44 55 66 77 88
void AdjustIssuerScript(void)
{
	int		iRet;
	uchar	*psTemp;

	memset(sgScriptBak, 0, sizeof(sgScriptBak));
	memset(&sgScriptInfo, 0, sizeof(sgScriptInfo));
	sgCurScript = sgScriptBakLen = 0;
	bHaveScript = FALSE;
	for (psTemp = sIssuerScript; psTemp < sIssuerScript + sgScriptLen; )
	{
		iRet = GetTLVItem(&psTemp, sIssuerScript + sgScriptLen - psTemp, SaveScriptData, TRUE);
		if (iRet < 0)
		{
			return;
		}
	}
	if (bHaveScript && sgCurScript > 0)
	{
		PackScriptData();
	}

	memcpy(sIssuerScript, sgScriptBak, sgScriptBakLen);
	sgScriptLen = sgScriptBakLen;
}

// callback function for process issuer script
void  SaveScriptData(uint uiTag, const uchar *psData, int iDataLen)
{
	switch (uiTag)
	{
	case 0x71:
	case 0x72:
		if (bHaveScript && sgCurScript > 0)
		{
			PackScriptData();
		}
		sgScriptInfo.uiTag = uiTag;
		bHaveScript = TRUE;
		break;

	case 0x9F18:
		sgScriptInfo.iIDLen = MIN(4, iDataLen);
		memcpy(sgScriptInfo.sScriptID, psData, MIN(4, iDataLen));
		break;

	case 0x86:
		sgScriptInfo.iCmdLen[sgCurScript] = iDataLen;
		memcpy(sgScriptInfo.sScriptCmd[sgCurScript], psData, iDataLen);
		sgCurScript++;
		break;

	default:
		break;
	}
}

void PackTLVData(uint uiTag, const uchar *psData, uint uiDataLen, uchar *psOutData, int *piOutLen)
{
	int		iHeadLen;

	PackTLVHead(uiTag, uiDataLen, psOutData, &iHeadLen);
	memcpy(psOutData + iHeadLen, psData, uiDataLen);
	*piOutLen = (uiDataLen + iHeadLen);
}

void PackTLVHead(uint uiTag, uint uiDataLen, uchar *psOutData, int *piOutLen)
{
	uchar	*psTemp;

	// pack tag bytes
	psTemp = psOutData;
	if (uiTag & 0xFF00)
	{
		*psTemp++ = uiTag >> 8;
	}
	*psTemp++ = uiTag;

	// pack length bytes
	if (uiDataLen <= 127)
	{
		*psTemp++ = (uchar)uiDataLen;
	}
	else
	{
		*psTemp++ = LENMASK_NEXTBYTE | 0x01;	// one byte length
		*psTemp++ = (uchar)uiDataLen;
	}

	*piOutLen = (psTemp - psOutData);
}

int CalcTLVTotalLen(uint uiTag, uint uiDataLen)
{
	int		iLen;

	// get length of TLV tag bytes
	iLen = 1;
	if (uiTag & 0xFF00)
	{
		iLen++;
	}

	// get length of TLV length bytes
	iLen++;
	if (uiDataLen > 127)
	{
		iLen++;
	}

	return (iLen + uiDataLen);
}

// re-generate issuer script(remove issuer script ID, if the length is zero)
void PackScriptData(void)
{
	int		iCnt, iTotalLen, iTempLen;

	iTotalLen = 0;
	if (sgScriptInfo.iIDLen > 0)
	{
		iTotalLen += CalcTLVTotalLen(0x9F18, 4);
	}
	for (iCnt = 0; iCnt < sgCurScript; iCnt++)
	{
		iTotalLen += CalcTLVTotalLen(0x86, sgScriptInfo.iCmdLen[iCnt]);
	}
	PackTLVHead(sgScriptInfo.uiTag, iTotalLen, &sgScriptBak[sgScriptBakLen], &iTempLen);
	sgScriptBakLen += iTempLen;

	if (sgScriptInfo.iIDLen > 0)
	{
		PackTLVData(0x9F18, sgScriptInfo.sScriptID, 4, &sgScriptBak[sgScriptBakLen], &iTempLen);
		sgScriptBakLen += iTempLen;
	}
	for (iCnt = 0; iCnt < sgCurScript; iCnt++)
	{
		PackTLVData(0x86, sgScriptInfo.sScriptCmd[iCnt], sgScriptInfo.iCmdLen[iCnt], &sgScriptBak[sgScriptBakLen], &iTempLen);
		sgScriptBakLen += iTempLen;
	}

	memset(&sgScriptInfo, 0, sizeof(sgScriptInfo));
	sgCurScript = 0;
}

// save EMV status for FUNC 9
void SaveTVRTSI(uchar bBeforeOnline)
{
	int				iRet, iLength, iCnt;
	uchar			sTermAID[16], sBuff[512];
	uchar			*psTLVData;
	EMV_APPLIST		stEmvApp;
	DE55Tag stList[] =
	{
		{0x5A,   DE55_MUST_SET, 0},
		{0x5F2A, DE55_MUST_SET, 0},
		{0x5F34, DE55_MUST_SET, 0},
		{0x82,   DE55_MUST_SET, 0},
		{0x84,   DE55_MUST_SET, 0},
		{0x8A,   DE55_MUST_SET, 0},
		{0x95,   DE55_MUST_SET, 0},
		{0x9A,   DE55_MUST_SET, 0},
		{0x9C,   DE55_MUST_SET, 0},
		{0x9F02, DE55_MUST_SET, 0},
		{0x9F03, DE55_MUST_SET, 0},
		{0x9F09, DE55_MUST_SET, 0},
		{0x9F10, DE55_MUST_SET, 0},
		{0x9F1A, DE55_MUST_SET, 0},
		{0x9F1E, DE55_MUST_SET, 0},
		{0x9F33, DE55_MUST_SET, 0},
		{0x9F34, DE55_MUST_SET, 0},
		{0x9F35, DE55_MUST_SET, 0},
		{0x9F36, DE55_MUST_SET, 0},
		{0x9F37, DE55_MUST_SET, 0},
		{0x9F41, DE55_MUST_SET, 0},
		{0},
	};

	SetStdDE55(FALSE, stList, glEmvStatus.sTLV + 2, &iLength);
	glEmvStatus.sTLV[0] = iLength / 256;
	glEmvStatus.sTLV[1] = iLength % 256;

	if (glProcInfo.bIsFirstGAC)
	{
		psTLVData = glEmvStatus.sAppCryptoFirst + 2;

		EMVGetTLVData(0x9F26, sBuff, &iLength);
		BuildTLVString(0x9F26, sBuff, iLength, &psTLVData);

		EMVGetTLVData(0x9F27, sBuff, &iLength);
		BuildTLVString(0x9F27, sBuff, iLength, &psTLVData);

		iLength = psTLVData - glEmvStatus.sAppCryptoFirst - 2;
		glEmvStatus.sAppCryptoFirst[0] = iLength / 256;
		glEmvStatus.sAppCryptoFirst[1] = iLength % 256;
	}
	else
	{
		psTLVData = glEmvStatus.sAppCryptoSecond + 2;

		EMVGetTLVData(0x9F26, sBuff, &iLength);
		BuildTLVString(0x9F26, sBuff, iLength, &psTLVData);

		EMVGetTLVData(0x9F27, sBuff, &iLength);
		BuildTLVString(0x9F27, sBuff, iLength, &psTLVData);

		iLength = psTLVData - glEmvStatus.sAppCryptoSecond - 2;
		glEmvStatus.sAppCryptoSecond[0] = iLength / 256;
		glEmvStatus.sAppCryptoSecond[1] = iLength % 256;
	}

	if (bBeforeOnline)
	{
		EMVGetTLVData(0x95, glEmvStatus.sgTVROld, &iLength);
		EMVGetTLVData(0x9B, glEmvStatus.sgTSIOld, &iLength);
		glEmvStatus.sgARQCLenOld = 0;
		EMVGetTLVData(0x9F10, glEmvStatus.sgARQCOld, &glEmvStatus.sgARQCLenOld);

		EMVGetTLVData(0x9F0E, glEmvStatus.sgIACDeinal, &iLength);
		EMVGetTLVData(0x9F0F, glEmvStatus.sgIACOnline, &iLength);
		EMVGetTLVData(0x9F0D, glEmvStatus.sgIACDefault, &iLength);

		// search TAC from terminal parameter
		memset(sTermAID, 0, sizeof(sTermAID));
		EMVGetTLVData(0x9F06, sTermAID, &iLength);
		for (iCnt = 0; iCnt < MAX_APP_NUM; iCnt++)
		{
			memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
			iRet = EMVGetApp(iCnt, &stEmvApp);
			if (iRet != EMV_OK)
			{
				continue;
			}
			if (memcmp(sTermAID, stEmvApp.AID, stEmvApp.AidLen) == 0)
			{
				memcpy(glEmvStatus.sgTACDeinal, stEmvApp.TACDenial, 5);
				memcpy(glEmvStatus.sgTACOnline, stEmvApp.TACOnline, 5);
				memcpy(glEmvStatus.sgTACDefault, stEmvApp.TACDefault, 5);
				break;
			}
		}
	}
	else
	{
		EMVGetTLVData(0x95, glEmvStatus.sgTVRNew, &iLength);
		EMVGetTLVData(0x9B, glEmvStatus.sgTSINew, &iLength);
	}
	SaveEmvStatus();
}

void UpdateEntryModeForOfflinePIN(void)
{
	int		iRet, iLength;
	uchar	sTemp[64];

	if (!(glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT))
	{
		return;
	}

	memset(sTemp, 0, sizeof(sTemp));
	iRet = EMVGetTLVData(0x9F34, sTemp, &iLength);
	if (iRet == EMV_OK)
	{
		sTemp[0] &= 0x3F;
		if (sTemp[2] == 0x02)		// last CVM succeed
		{
			if (sTemp[0] == 0x01 ||	// plaintext PIN
				sTemp[0] == 0x03 ||	// plaintext PIN and signature
				sTemp[0] == 0x04 ||	// enciphered PIN
				sTemp[0] == 0x05)	// enciphered PIN and signature
			{
				glProcInfo.stTranLog.uiEntryMode |= MODE_OFF_PIN;
			}
		}
	}
}

// show last EMV status
// Modified by Kim_LinHB 2014-6-8
int ViewTVR_TSI(void)
{
	int		iTemp;
	GUI_PAGELINE stBuff[100];
	unsigned char szHex[1 + 1];
	int		nLine = 0;
	GUI_PAGE stPage;
	GUI_TEXT_ATTR stLeftAttr_Small = gl_stLeftAttr;

	SetCurrTitle(_T("View TVR TSI")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	if (/*PasswordBank() != 0*/TRUE)
	{
		return ERR_NO_DISP;
	}

	LoadEmvStatus();

	memset(stBuff, 0, sizeof(stBuff));
	stLeftAttr_Small.eFontSize = GUI_FONT_SMALL;

	sprintf(stBuff[nLine].szLine, "Before TSI=%02X %02X", glEmvStatus.sgTSIOld[0], glEmvStatus.sgTSIOld[1]);
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[nLine].szLine, "TVR=%02X %02X %02X %02X %02X",
		glEmvStatus.sgTVROld[0], glEmvStatus.sgTVROld[1], glEmvStatus.sgTVROld[2],
		glEmvStatus.sgTVROld[3], glEmvStatus.sgTVROld[4]);
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	sprintf(stBuff[nLine].szLine, "IssuAppData=");
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	for (iTemp = 0; iTemp < glEmvStatus.sgARQCLenOld; iTemp++)
	{
		sprintf(szHex, "%02X", glEmvStatus.sgARQCOld[iTemp]);
		strcat(stBuff[nLine].szLine, szHex);
		if (0 == iTemp % 5)
			++nLine;
	}
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[nLine].szLine, "After TSI=%02X %02X", glEmvStatus.sgTSINew[0], glEmvStatus.sgTSINew[1]);
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[nLine].szLine, "TVR=%02X %02X %02X %02X %02X",
		glEmvStatus.sgTVRNew[0], glEmvStatus.sgTVRNew[1], glEmvStatus.sgTVRNew[2],
		glEmvStatus.sgTVRNew[3], glEmvStatus.sgTVRNew[4]);
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	strcpy(stBuff[nLine].szLine, "TACDenial =");
	PubBcd2Asc0(glEmvStatus.sgTACDeinal, 5, stBuff[nLine].szLine + strlen("TACDenial ="));
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	strcpy(stBuff[nLine].szLine, "TACOnline =");
	PubBcd2Asc0(glEmvStatus.sgTACOnline, 5, stBuff[nLine].szLine + strlen("TACOnline ="));
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	strcpy(stBuff[nLine].szLine, "IACDenial =");
	PubBcd2Asc0(glEmvStatus.sgIACDeinal, 5, stBuff[nLine].szLine + strlen("IACDenial ="));
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	strcpy(stBuff[nLine].szLine, "IACOnline =");
	PubBcd2Asc0(glEmvStatus.sgIACOnline, 5, stBuff[nLine].szLine + strlen("IACOnline ="));
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	strcpy(stBuff[nLine].szLine, "IACDefault =");
	PubBcd2Asc0(glEmvStatus.sgIACDefault, 5, stBuff[nLine].szLine + strlen("IACDefault ="));
	stBuff[nLine++].stLineAttr = stLeftAttr_Small;

	Gui_CreateInfoPage(GetCurrTitle(), gl_stTitleAttr, stBuff, nLine, &stPage);

	Gui_ClearScr();
	Gui_ShowInfoPage(&stPage, FALSE, USER_OPER_TIMEOUT);

	if (glEmvStatus.sAppCryptoFirst[1] > 0 ||
		glEmvStatus.sAppCryptoSecond[1] > 0 ||
		glEmvStatus.sTLV[1] > 0) // Added by Kim 20150116 bug612
	{
		Gui_ClearScr();
		if (GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PRINT DETAIL?"), gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL)) {
			return 0;
		}

		PrnInit();
		PrnSetNormal();
		PubDebugOutput("FIRST GAC", glEmvStatus.sAppCryptoFirst + 2,
			glEmvStatus.sAppCryptoFirst[1],
			DEVICE_PRN, TLV_MODE);
		PubDebugOutput("SECOND GAC", glEmvStatus.sAppCryptoSecond + 2,
			glEmvStatus.sAppCryptoSecond[1],
			DEVICE_PRN, TLV_MODE);
		PubDebugOutput("TRANS TLV", glEmvStatus.sTLV + 2,
			glEmvStatus.sTLV[1],
			DEVICE_PRN, TLV_MODE);
	}
	return 0;
}

unsigned char  cEMVPiccIsoCommand(unsigned char cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return 0;
}



unsigned char cPiccIsoCommand_Entry(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

unsigned char  cPiccIsoCommand_Pboc(unsigned char cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

unsigned char cPiccIsoCommand_Wave(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

unsigned char cPiccIsoCommand_MC(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

//added by kevinliu 2015/10/19
unsigned char cPiccIsoCommand_AE(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

int cClssCheckExceptionFile_Pboc(uchar *pucPAN, int nPANLen, uchar *pucPANSeq)
{
	return EMV_OK;
}

unsigned char cEMVSM2Verify(unsigned char *paucPubkeyIn, unsigned char *paucMsgIn, int nMsglenIn, unsigned char *paucSignIn, int nSignlenIn)
{
	return EMV_OK;
}

unsigned char cEMVSM3(unsigned char *paucMsgIn, int nMsglenIn, unsigned char *paucResultOut)
{
	return EMV_OK;
}

// end of file

