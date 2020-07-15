
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static void ClearReversalSub(void);
static int  ViewTranSub(int iStartRecNo);

/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

void UnLockTerminal(void)
{
	if( ChkEdcOption(EDC_NOT_KEYBOARD_LOCKED) )
	{
		return;
	}

	while( !ChkEdcOption(EDC_NOT_KEYBOARD_LOCKED) )
	{
		SetCurrTitle(_T("TERMINAL  LOCKED")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("MERCHANT PWD"), gl_stCenterAttr, GUI_BUTTON_NONE, -1, NULL);
		if( PasswordMerchant()==0 )
		{
			glSysParam.stEdcInfo.sOption[EDC_NOT_KEYBOARD_LOCKED>>8] |= (EDC_NOT_KEYBOARD_LOCKED & 0xFF);
			SaveEdcParam();
			PubBeepOk();
		}
	}
}

// Modified by Kim_LinHB 2014-6-8
int LockTerm(void)
{
	int iRet;
	SetCurrTitle(_T("LOCK TERM")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	Gui_ClearScr();
	iRet = Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TERMINAL LOCK? "), gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL);

	if( GUI_OK == iRet)
	{
		glSysParam.stEdcInfo.sOption[EDC_NOT_KEYBOARD_LOCKED>>8] &= ~(EDC_NOT_KEYBOARD_LOCKED & 0xFF);
		SaveEdcParam();
		PubBeepOk();
		UnLockTerminal();
	}
	return 0;
}

// Modified by Kim_LinHB 2014-6-8
int ClearAllRecord(void)
{
	int iRet;
	SetCurrTitle(_T("CLEAR BATCH")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	Gui_ClearScr();
	iRet = Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CONFIRM CLEAR"), gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL);

	if( iRet != GUI_OK)
	{
		return ERR_NO_DISP;
	}

	DispProcess();

	//ClearRecord(ACQ_ALL);
	DispClearOk();
	return 0;
}

// Modified by Kim_LinHB 2014-6-8
int ClearConfig(void)
{
	int iRet;
	SetCurrTitle(_T("CLEAR CONFIG")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	Gui_ClearScr();
	iRet = Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CONFIRM CLEAR"), gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL);

	if( iRet != GUI_OK)
	{
		return ERR_NO_DISP;
	}
	
	DispProcess();

	LoadEdcDefault();

#ifdef ENABLE_EMV
	LoadEmvDefault();
#endif
  
	DispClearOk();
	return 0;
}

// Modified by Kim_LinHB 2014-6-8
int ClearPassword(void)
{
	int iRet;
	SetCurrTitle(_T("CLEAR PASSWORD")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	Gui_ClearScr();
	iRet = Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CONFIRM CLEAR"), gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL);

	if( iRet != GUI_OK)
	{
		return ERR_NO_DISP;
	}

	DispProcess();

	ResetPwdAll();
	SavePassword();
	DispClearOk();
	return 0;
}

// Modified by Kim_LinHB 2014-6-8
int ClearReversal(void)
{
	int iRet;
	SetCurrTitle(_T("CLEAR REVERSAL")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	Gui_ClearScr();
	iRet = Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CONFIRM CLEAR"), gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL);

	if( iRet != GUI_OK)
	{
		return ERR_NO_DISP;
	}

	DispProcess();

	ClearReversalSub();
	DispClearOk();
	return 0;
}

void ClearReversalSub(void)
{

	SaveSysCtrlNormal();

	// glSysCtrl.stField56
}


// 清除终端数据界面
// Interface of "Clear". (FUNC99) 
// Modified by Kim_LinHB 2014-6-8
int DoClear(void)
{
	GUI_MENU	stClearMenu;
	GUI_MENUITEM stClearMenuItems[] =
	{
		{ "CLEAR CONFIG",	1,TRUE,  ClearConfig},
		{ "CLEAR BATCH",	2,TRUE,  ClearAllRecord},
		{ "CLEAR REVERSAL",	3,TRUE,  ClearReversal},
		{ "CLEAR PWD",	4,TRUE,  ClearPassword},
		{ "",	-1,FALSE, NULL},
	};

	SetCurrTitle(_T("CLEAR"));
	if( PasswordBank()!=0 )
	{
		return ERR_NO_DISP;
	}

	Gui_BindMenu(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stClearMenuItems, &stClearMenu);
	Gui_ClearScr();
	Gui_ShowMenuList(&stClearMenu, 0, USER_OPER_TIMEOUT, NULL);
	return 0;
}

// 查看交易汇总
// View total. (glTransTotal)
int ViewTotal(void)
{
	
	return 0;
}

// 查看所有交易记录
// View all transaction record
int ViewTranList(void)
{
	SetCurrTitle("TRANS REVIEW"); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	if( GetTranLogNum(ACQ_ALL)==0 )
	{
		// Modified by Kim_LinHB 2014-6-8
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("EMPTY BATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}

	// 浏览所以交易
	// view all transaction records
	ViewTranSub(-1);
	return 0;
}

// 查看指定交易记录
// View specific record
// Modified by Kim_LinHB 2014-6-8
int ViewSpecList(void)
{
	int			iRet;
	TRAN_LOG	stLog;

	SetCurrTitle("TRANS REVIEW");

	while (1)
	{
		if( GetTranLogNum(ACQ_ALL)==0 )
		{
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("EMPTY BATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
			return ERR_NO_DISP;
		}

		iRet = GetRecord(TS_ALL_LOG, &stLog);
		if( iRet!=0 )
		{
			return ERR_NO_DISP;
		}

		if (ViewTranSub((int)glProcInfo.uiRecNo)!=0)
		{
			break;
		}
	}
	return 0;
}

// 交易记录浏览控制
// View transaction records
// 返回：ERR_USERCANCEL--取消或超时退出；0--其它按键（或原因）退出
// return ERR_USERCANCEL--timeout or cancel
// Modified by Kim_LinHB 2014-6-8
int ViewTranSub(int iStartRecNo)
{
	int			iRecNo, iStep, iCnt, iActRecNo;
	TRAN_LOG	stLog;

	iRecNo = iStartRecNo;
	iStep  = iStartRecNo<0 ? 1 : 0;
	while( 1 )
	{
		iRecNo = iRecNo + iStep;
		if( iRecNo>=MAX_TRANLOG )
		{
			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("END OF BATCH"), gl_stCenterAttr, GUI_BUTTON_NONE, 1, NULL);
			iRecNo = 0;
		}
		else if( iRecNo<0 )
		{
			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("START OF BATCH"), gl_stCenterAttr, GUI_BUTTON_NONE, 1, NULL);
			iRecNo = MAX_TRANLOG-1;
		}
		if( glSysCtrl.sAcqKeyList[iRecNo]==INV_ACQ_KEY )
		{
			continue;
		}
		memset(&stLog, 0, sizeof(TRAN_LOG));
		LoadTranLog(&stLog, (ushort)iRecNo);
		for(iActRecNo=iCnt=0; iCnt<=iRecNo; iCnt++)
		{
			if( glSysCtrl.sAcqKeyList[iCnt]!=INV_ACQ_KEY )
			{
				iActRecNo++;
			}
		}

		{
			int		iRet;
			uchar	*pszTitle, szBuff[25], szTotalAmt[12+1], ucCnt = 0;
			GUI_PAGELINE stBuff[20];
			GUI_TEXT_ATTR stLeftAttr_Small = gl_stLeftAttr;
			GUI_TEXT_ATTR stLeftAttr = gl_stLeftAttr;
			GUI_PAGE	 stPage;

			TRAN_LOG	*pstLog = (TRAN_LOG *)&stLog;
			stLeftAttr_Small.eFontSize = GUI_FONT_SMALL;

#ifdef _Sxx_
			stLeftAttr.eFontSize = GUI_FONT_SMALL;
#endif

			sprintf(stBuff[ucCnt].szLine, "%03d/%03d", iActRecNo, GetTranLogNum(ACQ_ALL));
			stBuff[ucCnt++].stLineAttr = stLeftAttr;

			// Modified by Kim_LinHB 2014-8-8 v1.01.0002 bug508
			iRet = GetStateText(pstLog->uiStatus, szBuff);
			if(0 == iRet)
			{
				pszTitle = glTranConfig[pstLog->ucTranType].szLabel;
			}
			else if(1 == iRet)
			{
				pszTitle = glTranConfig[pstLog->ucOrgTranType].szLabel;
			}

			sprintf(stBuff[ucCnt].szLine, "Status:%s", szBuff);
			stBuff[ucCnt++].stLineAttr = stLeftAttr;

			sprintf(stBuff[ucCnt].szLine, "TRACE:%06lu", pstLog->ulInvoiceNo);
			stBuff[ucCnt++].stLineAttr = stLeftAttr;

			if( !ChkIfDispMaskPan2() )
			{
				strcpy(stBuff[ucCnt].szLine, pstLog->szPan);
			}
			else
			{
				MaskPan(pstLog->szPan, szBuff);
				strcpy(stBuff[ucCnt].szLine, szBuff);
			}
			stBuff[ucCnt++].stLineAttr = stLeftAttr_Small;


			PubAscAdd(pstLog->szAmount, pstLog->szOtherAmount, 12, szTotalAmt);
			App_ConvAmountTran(szTotalAmt,	szBuff, GetTranAmountInfo(pstLog));
			strcpy(stBuff[ucCnt].szLine, szBuff);
			stBuff[ucCnt++].stLineAttr = stLeftAttr_Small;
		
			strcpy(stBuff[ucCnt].szLine, "APPROVAL CODE:");
			stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

			sprintf(stBuff[ucCnt].szLine, "%6.6s", pstLog->szAuthCode);
			stBuff[ucCnt++].stLineAttr = gl_stRightAttr;
			
			Conv2EngTime(pstLog->szDateTime, szBuff);
			sprintf(stBuff[ucCnt].szLine, "%s", szBuff);
			stBuff[ucCnt++].stLineAttr = stLeftAttr_Small;

			sprintf(stBuff[ucCnt].szLine, "RRN:%12.12s", pstLog->szRRN);
			stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

			Gui_CreateInfoPage(_T(pszTitle), gl_stTitleAttr, stBuff, ucCnt, &stPage);
			Gui_ClearScr();

			iRet = Gui_ShowInfoPage(&stPage, iStartRecNo < 0 ? TRUE : FALSE, USER_OPER_TIMEOUT);

			// 查阅指定记录,不上下翻页
			// if viewing a specific record, then return directly
			if (iStartRecNo>=0)
			{
				return 0;
			}

			if(GUI_OK_NEXT == iRet)
			{
				iStep = 1;
			}
			else if(GUI_OK_PREVIOUS == iRet)
			{
				iStep = -1;
			}
			else
			{
				return ERR_USERCANCEL;
			}
		}
	}
	return 0;
}

// Modified by Kim_LinHB 2014-6-8
int PrnLastTrans(void)
{
	int			iRet;

	SetCurrTitle(_T("REPRINT"));

	
	if( glSysCtrl.uiLastRecNo>=MAX_TRANLOG )
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("EMPTY BATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}

	InitTransInfo();
	iRet = LoadTranLog(&glProcInfo.stTranLog, glSysCtrl.uiLastRecNo);
	if( iRet!=0 )
	{
		return ERR_NO_DISP;
	}

	Gui_ClearScr();
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	PrintReceipt(PRN_REPRINT);
	return 0;
}

// Modified by Kim_LinHB 2014-6-8
int RePrnSpecTrans(void)
{
	int			iRet;

	SetCurrTitle(_T("REPRINT"));

	if( GetTranLogNum(ACQ_ALL)==0 )
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("EMPTY BATCH"),gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}

	InitTransInfo();
	iRet = GetRecord(TS_ALL_LOG, &glProcInfo.stTranLog);
	if( iRet!=0 )
	{
		return ERR_NO_DISP;
	}

	Gui_ClearScr();
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	PrintReceipt(PRN_REPRINT);
	return 0;
}

// Modified by Kim_LinHB 2014-6-8
int PrnTotal(void)
{

	return 0;
}

// Modified by Kim_LinHB 2014-6-8
int RePrnSettle(void)
{
	
}

#ifdef ENABLE_EMV
// Print EMV error log message
int PrintEmvErrLog(void)
{
	// Modified by Kim_LinHB 2014-6-8
	SetCurrTitle(_T("PRINT ERROR LOG"));
	if( PasswordBank()!=0 )
	{
		return ERR_NO_DISP;
	}

	DispProcess();

	PrintEmvErrLogSub();
	return 0;
}
#endif

// end of file

