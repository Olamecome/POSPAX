#include "global.h"
#include "xui.h"
#include "utils.h"


void resetTransactionData() {
	InitTransInfo();
	getRRN(glProcInfo.stTranLog.szRRN);;
}


static void resetFallbackInfo() {
	SYS_PROC_INFO temp = glProcInfo;
	resetTransactionData();
	glProcInfo.stTranLog.ucTranType = temp.stTranLog.ucTranType;
	glProcInfo.stTranLog.ucAccountType = temp.stTranLog.ucAccountType;
	memcpy(glProcInfo.stTranLog.szAmount, temp.stTranLog.szAmount, sizeof(glProcInfo.stTranLog.szAmount));
	memcpy(glProcInfo.stTranLog.szOtherAmount, temp.stTranLog.szOtherAmount, sizeof(glProcInfo.stTranLog.szOtherAmount));

	strmcpy(glProcInfo.stTranLog.szOrgAmount, temp.stTranLog.szOrgAmount, lengthOf(glProcInfo.stTranLog.szOrgAmount));
	strmcpy(glProcInfo.stTranLog.szOrgDateTime, temp.stTranLog.szOrgDateTime, lengthOf(glProcInfo.stTranLog.szOrgDateTime));
	strmcpy(glProcInfo.stTranLog.szOrgRRN, temp.stTranLog.szOrgRRN, lengthOf(glProcInfo.stTranLog.szOrgRRN));
	strmcpy(glProcInfo.stTranLog.szOriginalForwdInstCode, temp.stTranLog.szOriginalForwdInstCode, lengthOf(glProcInfo.stTranLog.szOriginalForwdInstCode));
	glProcInfo.stTranLog.ucOrgTranType = temp.stTranLog.ucOrgTranType;
	glProcInfo.stTranLog.ulOrgSTAN = temp.stTranLog.ulOrgSTAN;

}


static void notifyTransaction() {
	DispMessage("Sending Notification");
	RunnerData runnerData = { 0 };

	runnerData.data = &glProcInfo.stTranLog;
	runnerData.recordIdx = glProcInfo.uiRecNo;
	logd(("Current Record: %d", glProcInfo.uiRecNo));
	
	notificationHandler((void*)&runnerData);
}

static int statusReceiptAndNotification() {
	DispPrinting();
	CommDial(DM_PREDIAL);

	if (glProcInfo.stTranLog.ucTranType != REVERSAL) {
		SaveTranLog(&glProcInfo.stTranLog);
	}
	else {
		UpdateTranLog(&glProcInfo.stTranLog, glProcInfo.uiRecNo);
	}
	
	if (!ChkHardware(HWCFG_PRINTER, 0) &&  checkPrinter())
	{
		printTransactionReceipt(&glProcInfo.stTranLog, CUSTOMER_COPY, 0);
		PubWaitKey(1);
		printTransactionReceipt(&glProcInfo.stTranLog, MERCHANT_COPY, 0);
	}

	notifyTransaction();

	return  isSuccessResponse(glProcInfo.stTranLog.szRspCode) ? 0 : -1;
}

static int finishSwipeTransaction() {
	logTrace(__func__);
	logTrace("Track1: %s", glProcInfo.szTrack1);
	logTrace("Track2: %s", glProcInfo.szTrack2);
	logTrace("Track3: %s", glProcInfo.szTrack3);
	logTrace("security code: %s", glProcInfo.szSecurityCode);
	logTrace("PAN: %s", glProcInfo.stTranLog.szPan);
	logTrace("Card Expirity: %d", glProcInfo.stTranLog.szExpDate);

	int ret = FinishSwipeTran();

	if (0 == ret) {//Normal processing completed (approved or declined)
		return statusReceiptAndNotification();
	}

	showErrorDialog("Transaction Failed", USER_OPER_TIMEOUT);
	return -1;

}


static int finishContactTransaction() {
	logTrace(__func__);
	int ret =  FinishEmvTran();

	if (ret == ERR_NEED_FALLBACK) {
		logd(("Fallback required"));
		resetFallbackInfo();
		return startEmvTransaction(FALLBACK_SWIPE, glProcInfo.stTranLog.ucTranType, glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount);
	} 

	if (0 == ret) {//Normal processing completed (approved or declined)
		return statusReceiptAndNotification();
	}

	showErrorDialog("Transaction Failed", USER_OPER_TIMEOUT);
	return -1;
}

int finishContactlessTransaction() {
	logTrace(__func__);
	CommDial(DM_PREDIAL);
	int ret = TransClssSale(TRUE);

	if (ret == ERR_NEED_FALLBACK) {
		logd(("Fallback required"));
		resetFallbackInfo();

		return startEmvTransaction(CARD_INSERTED, glProcInfo.stTranLog.ucTranType, glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount);
	} else if (0 == ret) {//Normal processing completed (approved or declined)
		return statusReceiptAndNotification();
	}
	else {
		disp_clss_err(ret);
	}

	return -1;
}


int startEmvTransaction(unsigned short uiEntryMode, int ucTranType, char amount[12 + 1], char otherAmount[12 + 1]) {
	logTrace(__func__);
	glProcInfo.stTranLog.ucTranType = ucTranType;
	logd(("Tran type: %d", glProcInfo.stTranLog.ucTranType));

	int ret = -1;
	ret = GetCard(uiEntryMode);
	logTrace("GetCard ret::%d, entryMode: %02x", ret, glProcInfo.stTranLog.uiEntryMode);
	logd(("Tran type: %d", glProcInfo.stTranLog.ucTranType));

	if (ret == 0) {
		if ((glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) || 
				(glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT)) {
			ret = finishSwipeTransaction();
		}
		else if (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) {
			ret = finishContactTransaction();
		}
		else if (glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS) {
			ret = finishContactlessTransaction();
		}
	}


	PromptRemoveICC();
	return ret;
}