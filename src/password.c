
#include "global.h"

/********************** Internal macros declaration ************************/

// PED function for P8010 not implement [11/20/2006 Tommy]

/********************** Internal structure declaration *********************/
typedef struct _tagPASSWD_INFO
{
	uchar	szMsg[16+1];
	uchar	ucLen;
}PASSWD_INFO;

/********************** Internal functions declaration *********************/
static uchar PasswordSub(uchar ucPwdID);
static uchar PasswordNew(uchar *psOutPwd, uchar ucLen);
static void  ModifyPasswordSub(uchar ucPwdID);

/********************** Internal variables declaration *********************/
static PASSWD_INFO sgPwdInfo[] =
{
	// the order must be the same as the orders of PWD_xxx
	{_T_NOOP("BANK PASSWORD   "), 6},
	{_T_NOOP("ENT TERMINAL PWD"), 4},
	{_T_NOOP("ENT MERCHANT PWD"), 4},
	{_T_NOOP("VOID PASSWORD   "), 4},
	{_T_NOOP("REFUND PASSWORD "), 4},
	{_T_NOOP("ADJUST PASSWORD "), 4},
	{_T_NOOP("SETTLEMENT PWD  "), 4},
};

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

void ResetPwdAll(void)
{
	int	iCnt;
	
	for (iCnt=0; iCnt<PWD_MAX; iCnt++)
	{
		sprintf((char *)&glSysParam.sPassword[iCnt][0], "%0*d", sgPwdInfo[iCnt].ucLen, 0);
	}
}

uchar PasswordBank(void)
{
	return PasswordSub(PWD_BANK);
}

uchar PasswordTerm(void)
{
	return PasswordSub(PWD_TERM);
}

uchar PasswordMerchant(void)
{
	return PasswordSub(PWD_MERCHANT);
}

uchar PasswordVoid(void)
{
	return PasswordSub(PWD_VOID);
}

uchar PasswordRefund(void)
{
	return PasswordSub(PWD_REFUND);
}

uchar PasswordAdjust(void)
{
	return PasswordSub(PWD_ADJUST);
}

uchar PasswordSettle(void)
{
	return PasswordSub(PWD_SETTLE);
}

int ModifyPasswordBank(void)
{
	SetCurrTitle(_T("BANK   PWD")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	ModifyPasswordSub(PWD_BANK);
	return 0;
}

int ModifyPasswordTerm(void)
{
	SetCurrTitle(_T("TERMINAL   PWD")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	ModifyPasswordSub(PWD_TERM);
	return 0;
}

int ModifyPasswordMerchant(void)
{
	SetCurrTitle(_T("MERCHANT   PWD")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	ModifyPasswordSub(PWD_MERCHANT);
	return 0;
}

int ModifyPasswordVoid(void)
{
	SetCurrTitle(_T("VOID   PWD")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	ModifyPasswordSub(PWD_VOID);
	return 0;
}

int ModifyPasswordRefund(void)
{
	SetCurrTitle(_T("REFUND   PWD")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	ModifyPasswordSub(PWD_REFUND);
	return 0;
}

int ModifyPasswordAdjust(void)
{
	SetCurrTitle(_T("ADJUST   PWD")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	ModifyPasswordSub(PWD_ADJUST);
	return 0;
}

int ModifyPasswordSettle(void)
{
	SetCurrTitle(_T("SETTLE   PWD")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	ModifyPasswordSub(PWD_SETTLE);
	return 0;
}

// Modified by Kim_LinHB 2014-6-20
uchar PasswordSub(uchar ucPwdID)
{
	uchar szBuff[20];
	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 0;
	stInputAttr.nMinLen = sgPwdInfo[ucPwdID].ucLen;
	stInputAttr.nMaxLen = 6;
	stInputAttr.bSensitive = 1;

	if( ucPwdID>sizeof(sgPwdInfo)/sizeof(sgPwdInfo[0]) )
	{
		return 1;
	}

	while( 1 )
	{
		memset(szBuff, 0, sizeof(szBuff));
		Gui_ClearScr();
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, 
			_T(sgPwdInfo[ucPwdID].szMsg), gl_stLeftAttr, szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT)){
			Gui_ClearScr();
			return 1;
		}

		// give up using PED for the local password
		if( memcmp(szBuff, glSysParam.sPassword[ucPwdID], sgPwdInfo[ucPwdID].ucLen)==0 &&
			strlen((char *)szBuff)==(int)sgPwdInfo[ucPwdID].ucLen )
		{
			return 0;
		}

		if( memcmp(szBuff, glSysParam.sPassword[PWD_BANK], sgPwdInfo[PWD_BANK].ucLen)==0 &&
			strlen((char *)szBuff)==(int)sgPwdInfo[PWD_BANK].ucLen )
		{
			return 0;
		}

		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PWD ERROR!"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	}

	return 0;
}

void ModifyPasswordSub(uchar ucPwdID)
{
	if( PasswordSub(ucPwdID)!=0 )
	{
		return;
	}

	if( PasswordNew(glSysParam.sPassword[ucPwdID], sgPwdInfo[ucPwdID].ucLen)!=0 )
	{
		return;
	}

	// Modified by Kim_LinHB 2014-6-21
	SavePassword();
	Gui_ClearScr();
	PubBeepOk();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PWD CHANGED!"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	
}

// Modified by Kim_LinHB 2014-6-21
uchar PasswordNew(uchar *psOutPwd, uchar ucLen)
{
	uchar sBuff1[20], sBuff2[20];
	int iRet;

	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.eType = GUI_INPUT_NUM;
	stInputAttr.bEchoMode = 0;
	stInputAttr.nMinLen = ucLen;
	stInputAttr.nMaxLen = ucLen;
	stInputAttr.bSensitive = 1;

	while( 1 )
	{
		memset(sBuff1, 0, sizeof(sBuff1));
		Gui_ClearScr();
		iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("ENTER NEW PWD"), gl_stLeftAttr,
			sBuff1, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
		if( iRet!=GUI_OK ){
			return 1;
		}

		memset(sBuff2, 0, sizeof(sBuff2));
		Gui_ClearScr();
		iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("RE-ENTER NEW PWD"), gl_stLeftAttr,
			sBuff2, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
		if( iRet!=GUI_OK ){
			return 1;
		}

		// give up using PED for the local password
		if( memcmp(sBuff1, sBuff2, ucLen)==0 )
		{
			memcpy(psOutPwd, sBuff1, ucLen);
			return 0;
		}
       
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("NOT CONSISTENT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	}

	return 0;
}

// uchar PasswordCheck(uchar *password6, uchar *password4, uchar *password_chk)

// end of file
