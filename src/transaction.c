#include "global.h"
#include "xui.h"
#include "util.h"
#include "Logger.h"
#include "utils.h"

static bool hasOriginalAmount(uchar tranType) {
	switch (tranType) {
	case VOID:
	case REVERSAL:
	case REFUND:
	case POS_PRE_AUTH_COMPLETION:
		return true;
	default: 
		return false;
	}
}

static bool requireAmount(uchar tranType) {
	switch (tranType) {
	case VOID:
	case REVERSAL:
	case BALANCE:
	case MINI_STATEMENT:
		return false;
	default:
		return true;
	}
}

static bool requireOtherAmount(uchar tranType) {
	switch (tranType) {
	case PURCHASE_WITH_CASH_BACK:
		return true;
	default:
		return false;
	}
}

static int getOriginalTransaction() {
	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	GUI_INPUTBOX_ATTR inputAttr = { 0 };
	inputAttr.bEchoMode = true;
	inputAttr.nMinLen = 12;
	inputAttr.nMaxLen = 12;
	inputAttr.eType = GUI_INPUT_NUM;

	char RRN[12 + 1] = "\0";
	clearScreen();
	if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "Enter RRN", gl_stLeftAttr, RRN, gl_stCenterAttr, &inputAttr, 60)) {
		return -1;
	}


	TRAN_LOG trans = { 0 };
	int i = 0;
	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);

		if (stringEqual(RRN, trans.szRRN)) {
			strmcpy(glProcInfo.stTranLog.szOrgAmount, trans.szAmount, lengthOf(glProcInfo.stTranLog.szOrgAmount));
			strmcpy(glProcInfo.stTranLog.szOrgDateTime, trans.szDateTime, lengthOf(glProcInfo.stTranLog.szOrgDateTime));
			strmcpy(glProcInfo.stTranLog.szOrgRRN, trans.szRRN, lengthOf(glProcInfo.stTranLog.szOrgRRN));
			strmcpy(glProcInfo.stTranLog.szOriginalForwdInstCode, trans.szOriginalForwdInstCode, lengthOf(glProcInfo.stTranLog.szOriginalForwdInstCode));
			glProcInfo.stTranLog.ucOrgTranType = trans.ucTranType;
			glProcInfo.stTranLog.ulOrgSTAN = trans.ulSTAN;

			if (glProcInfo.stTranLog.ucTranType == REVERSAL) {
				strmcpy(glProcInfo.stTranLog.szRRN, trans.szRRN, lengthOf(glProcInfo.stTranLog.szRRN));
			}
			return 0;
		}
	}

	showErrorDialog("Transaction not found", USER_OPER_TIMEOUT);
	return -1;
}

int transaction(uchar tranType) {

	clearScreen();
	SetCurrTitle(getTransactionTitle(tranType));
	
	if (!checkTerminalPrepStatus())// || !checkPrinter()) 
	{
		return -1;
	}

	resetTransactionData();
	glProcInfo.stTranLog.ucTranType = tranType;

	if (glPosParams.tranRecordCount >= MAX_TRANLOG) {
		DispErrMsg("Memory Full", "Run Close Batch!!!", USER_OPER_TIMEOUT, DERR_BEEP);
		return -1;
	}

	if (hasOriginalAmount(tranType)) {
		if (getOriginalTransaction() != 0) {
			return -1;
		}
	}

ENTER_AMT:
	if (requireAmount(tranType)) {
		if (GUI_OK != GetAmountNew(GetCurrTitle(), glProcInfo.stTranLog.szAmount, AMOUNT)) {
			return GUI_ERR_USERCANCELLED;
		}

		logTrace("Amount: %s", glProcInfo.stTranLog.szAmount);
		if (hasOriginalAmount(tranType)) {
			if (atol(glProcInfo.stTranLog.szAmount) > atol(glProcInfo.stTranLog.szOrgAmount)) {
				char dispAmount[50] = "\0";
				GetDispAmount(glProcInfo.stTranLog.szOrgAmount, dispAmount);
				showInfo(GetCurrTitle(), 10, 3, "Amount entered > \n%s", dispAmount);
				goto ENTER_AMT;
			}
		}
	}

	if (requireOtherAmount(tranType)) {
		if (GUI_OK != GetAmountNew(GetCurrTitle(), glProcInfo.stTranLog.szOtherAmount, CASHBACKAMOUNT)) {
			return GUI_ERR_USERCANCELLED;
		}

		logTrace("Other Amount: %s", glProcInfo.stTranLog.szOtherAmount);
	}
	else {
		sprintf((char *)glProcInfo.stTranLog.szOtherAmount, "%012ld", 0L);
	}

#ifdef ENABLE_ACCT_SEL
	if (GUI_OK != GetAccountType("Account Type", &glProcInfo.stTranLog.ucAccountType)) {
		goto ENTER_AMT;
	}
#endif // ENABLE_ACCT_SEL


	//Start Transaction
	startEmvTransaction(/*CARD_SWIPED |*/ CARD_INSERTED | CARD_TAPPED, tranType, glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount);

	return 0;
}