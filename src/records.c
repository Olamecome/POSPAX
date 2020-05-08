#include "records.h"
#include "xui.h"
#include "utils.h"
#include "printHelper.h"


extern int sendCardNotification(TRAN_LOG* data);

int reprintRRN() {

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
	if (GUI_OK != Gui_ShowInputBox("Reprint RRN", gl_stTitleAttr, "Enter RRN", gl_stLeftAttr, RRN, gl_stCenterAttr, &inputAttr, 60)) {
		return -1;
	}


	TRAN_LOG trans = { 0 };
	int i = 0;
	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);

		if (stringEqual(RRN, trans.szRRN)) {
			DispPrinting();
			printTransactionReceipt(&trans, MERCHANT_COPY, TRUE);
			return 0;
		}
	}

	showErrorDialog("Transaction not found", USER_OPER_TIMEOUT);
	return -1;
}


int reprintSTAN() {

	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	GUI_INPUTBOX_ATTR inputAttr = { 0 };
	inputAttr.bEchoMode = true;
	inputAttr.nMinLen = 6;
	inputAttr.nMaxLen = 6;
	inputAttr.eType = GUI_INPUT_NUM;

	char STAN[6 + 1] = "\0";
	clearScreen();
	if (GUI_OK != Gui_ShowInputBox("Reprint STAN", gl_stTitleAttr, "Enter STAN", gl_stLeftAttr, STAN, gl_stCenterAttr, &inputAttr, 60)) {
		return -1;
	}


	TRAN_LOG trans = { 0 };
	int i = 0;
	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);

		if (atol(STAN) == trans.ulSTAN) {
			DispPrinting();
			printTransactionReceipt(&trans, MERCHANT_COPY, TRUE);
			return 0;
		}
	}

	showErrorDialog("Transaction not found", USER_OPER_TIMEOUT);
	return -1;
}


int reprintSequenceNumber() {

	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	GUI_INPUTBOX_ATTR inputAttr = { 0 };
	inputAttr.bEchoMode = true;
	inputAttr.nMinLen = 6;
	inputAttr.nMaxLen = 6;
	inputAttr.eType = GUI_INPUT_NUM;

	char seqNo[6 + 1] = "\0";
	clearScreen();
	if (GUI_OK != Gui_ShowInputBox("Reprint SEQ NO", gl_stTitleAttr, "Enter Sequence Number", gl_stLeftAttr, seqNo, gl_stCenterAttr, &inputAttr, 60)) {
		return -1;
	}


	TRAN_LOG trans = { 0 };
	int i = 0;
	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);

		if (atol(seqNo) == trans.ulInvoiceNo) {
			DispPrinting();
			printTransactionReceipt(&trans, MERCHANT_COPY, TRUE);
			return 0;
		}
	}

	showErrorDialog("Transaction not found", USER_OPER_TIMEOUT);
	return -1;
}


int reprintLast() {
	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	if (glPosParams.tranRecordCount > MAX_TRANLOG) {
		showErrorDialog("Record Overflow.\nPlease run EOD!!!", USER_OPER_TIMEOUT);
		return -1;
	}

	int lastIdx = glPosParams.tranRecordCount - 1;
	TRAN_LOG trans = { 0 };
	if (0 != LoadTranLog(&trans, lastIdx)) {
		showErrorDialog("Error loading transaction", USER_OPER_TIMEOUT);
		return -1;
	}


	DispPrinting();
	printTransactionReceipt(&trans, MERCHANT_COPY, TRUE);
	return 0;
}


int reprintAll() {

	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	DispPrinting();

	TRAN_LOG trans = { 0 };
	int i = 0;
	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);
		DispPrinting();
		printTransactionReceipt(&trans, MERCHANT_COPY, TRUE);
	}

	return 0;
}


int repushTransactions(char silent) {
	if (glPosParams.tranRecordCount <= 0) {
		//showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return 0;
	}

	if (!silent) {
		DispMessage("Checking transactions");
	}
	
	int count = 0;
	TRAN_LOG trans = { 0 };
	int i = 0;
	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);
		if (!trans.notified) {
			count++;
			do {
				if (!silent) {
					DispMessage("Re-pushing notification");
				}
				
			} while (sendCardNotification(&trans) != 0);
			trans.notified = TRUE;
			UpdateTranLog(&trans, i);
		}
	}

	if (!silent) {
		showInfo(GetCurrTitle(), 10, 1, "All notifications pushed");
	}

	return 0;
}

static int tranLogToReportData(TRAN_LOG* trans, SummaryReport* summary, int indx) {
	ReportData* current = summary->data + indx;

	strmcpy(current->responseCode, trans->szRspCode, lengthOf(current->responseCode));
	strmcpy(current->cardExpiry, trans->szExpDate, lengthOf(current->cardExpiry));
	strmcpy(current->date, trans->szDateTime, lengthOf(current->date));
	strmcpy(current->RRN, trans->szRRN, lengthOf(current->RRN));
	strmcpy(current->amount, trans->szAmount, lengthOf(current->amount));
	strmcpy(current->otherAmount, trans->szOtherAmount, lengthOf(current->otherAmount));
	maskPan( current->maskedPan,trans->szPan, '*');
	strmcpy(current->time, trans->szDateTime + 8, lengthOf(current->time));
	current->tranType = trans->ucTranType;
	current->batchNumber = glPosParams.batchNo;
	current->sequenceNumber = trans->ulInvoiceNo;

	if (isSuccessResponse(current->responseCode) && (current->tranType != REVERSAL && current->tranType != REFUND)) {
		summary->totalApproved += atol(current->amount) + atol(current->otherAmount);
	} else {
		summary->totalDeclined += atol(current->amount) + atol(current->otherAmount);
	}
}

int dailySummary() {
	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	char today[15 + 1] = { 0 };
	GetDateTime(today);

	DispMessage("Checking transactions");

	SummaryReport summary = { 0 };
	TRAN_LOG trans = { 0 };
	int i = 0;
	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);
		if (isEqual(trans.szDateTime, today, 8)) {
			summary.data = realloc(summary.data, sizeof(ReportData) * (summary.reportCount+1));
			memset(summary.data + summary.reportCount, 0, sizeof(ReportData));

			ReportData* current = summary.data + summary.reportCount;
			tranLogToReportData(&trans, &summary, summary.reportCount);
			
			summary.reportCount++;
		}
	}

	if (summary.reportCount <= 0) {
		showErrorDialog("No transaction found", 30);
		return -1;
	}
	
	DispPrinting();
	printSummaryReport(&summary, "DAILY SUMMARY");


	if (summary.data) {
		free(summary.data);
	}
	
	return 0;
}
int weeklySummary() {
	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	char today[15 + 1] = "\0", weekStart[15+1] = "\0",weekEnd[15+1] = "\0";
	GetDateTime(today);

	char szYear[4 + 1] = "\0", szMonth[2+1] = "\0", szDay[2+1] = "\0";
	strncpy(szYear, today, 4);
	strncpy(szMonth, today + 4, 2);
	strncpy(szDay, today + 6, 2);

	int dayIndx = dayOfWeek(atoi(szYear), atoi(szMonth), atoi(szDay));
	logd(("Index Of day in week: %d", dayIndx));

	PubCalDateTime(today, weekStart, -dayIndx, "DD");
	logd(("Week started: %s, %ld", weekStart, PubDay2Long(weekStart)));

	PubCalDateTime(today, weekEnd, 6 - dayIndx, "DD");
	logd(("Week ends: %s, %ld", weekEnd, PubDay2Long(weekEnd)));

	DispMessage("Checking transactions");
	
	SummaryReport summary = { 0 };
	TRAN_LOG trans = { 0 };
	int i = 0;
	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);
		int ulTranDate = PubDay2Long(trans.szDateTime);
		logd(("ulDate: %ld", ulTranDate));
		if (PubDay2Long(weekStart) <=  ulTranDate && ulTranDate <= PubDay2Long(weekEnd)){
			summary.data = realloc(summary.data, sizeof(ReportData) * (summary.reportCount + 1));
			memset(summary.data + summary.reportCount, 0, sizeof(ReportData));

			ReportData* current = summary.data + summary.reportCount;
			tranLogToReportData(&trans, &summary, summary.reportCount);

			summary.reportCount++;
		}
	}

	if (summary.reportCount <= 0) {
		showErrorDialog("No transaction found", 30);
		return -1;
	}

	DispPrinting();
	printSummaryReport(&summary, "WEEKLY SUMMARY");


	if (summary.data) {
		free(summary.data);
	}

	return 0;
}

int yearlySummary() {
	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	char date[15 + 1] = { 0 };
	GetDateTime(date);

	DispMessage("Checking transactions");

	SummaryReport summary = { 0 };
	TRAN_LOG trans = { 0 };
	int i = 0;
	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);
		if (isEqual(trans.szDateTime, date, 4)) {
			summary.data = realloc(summary.data, sizeof(ReportData) * (summary.reportCount + 1));
			memset(summary.data + summary.reportCount, 0, sizeof(ReportData));

			ReportData* current = summary.data + summary.reportCount;
			tranLogToReportData(&trans, &summary, summary.reportCount);

			summary.reportCount++;
		}
	}

	if (summary.reportCount <= 0) {
		showErrorDialog("No transaction found", 30);
		return -1;
	}

	DispPrinting();
	printSummaryReport(&summary, "YEARLY SUMMARY");


	if (summary.data) {
		free(summary.data);
	}

	return 0;
}



int closeBatch() {
	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	SummaryReport summary = { 0 };
	TRAN_LOG trans = { 0 };
	int i = 0;
	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);
		summary.data = realloc(summary.data, sizeof(ReportData) * (summary.reportCount + 1));
		memset(summary.data + summary.reportCount, 0, sizeof(ReportData));

		ReportData* current = summary.data + summary.reportCount;
		tranLogToReportData(&trans, &summary, summary.reportCount);

		summary.reportCount++;
	}

	if (summary.reportCount <= 0) {
		showErrorDialog("No transaction found", 30);
		return -1;
	}

	DispPrinting();
	printEODReport(&summary, "CLOSE BATCH", CLOSE_BATCH_COPY);
		
	glPosParams.tranRecordCount = 0;
	glPosParams.batchNo++;
	glPosParams.sequenceNo = 1;
	SavePosParams();
	InitTranLogFile();
}

int processEOD(int option) {
	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	char date[15 + 1] = "\0";
	Prompt prompt = { 0 };

	switch (option) {
	case 0: //Today
	case 1: { //Yesterday
		GetDateTime(date);
		if (option == 1) { //yesterday
			long long_date = PubDay2Long(date);
			long_date--; //reduce days by one.
			CLEAR_STRING(date, lengthOf(date));
			PubLong2Day(long_date, date);
		}
	}
		break;
	default:
		getNumberPrompt(&prompt, "TRANSACTION DATE", "\0");
		strncpy(prompt.hint, "YYYYMMDD", 8);
		prompt.hint[8] = '\0';
		prompt.minLength = 8;
		prompt.maxLength = 8;
		GetDateTime(prompt.value);
		prompt.value[8] = '\0';

		ASSERT_RETURNCODE(showPrompt(&prompt));
		prompt.value[8] = '0';

		if (PubIsValidTime(prompt.value, "YYYYMMDDhhmmss") != 0) {
			showErrorDialog("Invalid Date", 10);
			return -1;
		}

		strmcpy(date, prompt.value, lengthOf(date));
		break;
	}

	logd(("EOD Date: %s", date));

	int type = 0;
	getListItemPrompt(&prompt, "EOD OPTION", "FULL|SUMMARY");
	ASSERT_RETURNCODE(showPrompt(&prompt))

		switch (prompt.selectionOption) {
		case 0:
			type = EOD_FULL_COPY;
			break;
		default:
			type = EOD_TOTAL_COPY;
			break;
		}


	SummaryReport summary = { 0 };
	TRAN_LOG trans = { 0 };
	int i = 0;

	for (; i < glPosParams.tranRecordCount; i++) {
		LoadTranLog(&trans, i);
		if (isEqual(trans.szDateTime, date, 8)) {
			summary.data = realloc(summary.data, sizeof(ReportData) * (summary.reportCount + 1));
			memset(summary.data + summary.reportCount, 0, sizeof(ReportData));

			ReportData* current = summary.data + summary.reportCount;
			tranLogToReportData(&trans, &summary, summary.reportCount);

			summary.reportCount++;
		}
	}

	if (summary.reportCount <= 0) {
		showErrorDialog("No transaction found", 30);
		return -1;
	}

	DispPrinting();
	printEODReport(&summary, "EOD", type);

	return 0;
}