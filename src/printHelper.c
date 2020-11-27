#include "printHelper.h"
#include "utils.h"
#include "xIsoDef.h"


int checkPrinter() {
	int ret = PrnStatus();
	logd(("Printer check: %d", ret));
	if (ret != PRN_OK) {
		DispPrnError(ret);
		return 0;
	}


	return 1;
}


static ST_FONT normalFont = {CHARSET_WEST, 12, 24, 1,0};
static ST_FONT bigFont = { CHARSET_WEST, 16, 24, 0,0 };
static ST_FONT bigBoldFont = { CHARSET_WEST, 16, 24, 1,0 };

static void resetPrnFormat() {
	PrnSelectFont(&normalFont, NULL);
	PrnSpaceSet(0, 0);
	PrnSetGray(1);
	PrnAttrSet(0);
	PrnDoubleWidth(0, 0);
	PrnDoubleHeight(0, 0);

}

void prnDoubleStr(char val1[32], char val2[32])
{
	int rest = 0, i;
	char buf[32];

	memset(buf, 0, sizeof(buf));
	memset(buf, ' ', sizeof(buf) - 1);
	i = strlen(val1) + strlen(val2);

	if (i<32)
	{
		rest = 32 - (strlen(val1) + strlen(val2));
		buf[rest] = 0;
		PrnStr("%s%s%s\n", val1, buf, val2);
	}
	else {
		PrnStr("%s %s\n", val1, val2);

	}
}

static void prnCenter(char *pszString)
{
	if (strlen(pszString) > 32) {
		PrnStr(pszString);
		return;
	}

	char  szPrefix[40]="\0", szSuffix[40] = "\0";
	int   iDatalen, i;
	iDatalen = strlen(pszString);


	for (i = 1; i <= (16 - (iDatalen / 2)); i++) {
		strcat(szPrefix, " ");
	}
		
	for (i = 1; i <= (31 - (strlen(szPrefix) + iDatalen)); i++) {
		strcat(szSuffix, " ");
	}
		

	PrnStr("%s%s%s\n", szPrefix, pszString, szSuffix);
}

static void prnFullLenChar(char xter) {
	char output[32 + 1] = { 0 };
	memset(output, xter, 32);
	output[31] = '\n';

	PrnStr(output);
}

static void prnFeedPaper(int count) {
	char output[10 + 1] = { 0 };
	memset(output, '\n', MIN(10, count));
	PrnStr(output);
}


static void prnHeader(char* dateTime) {
	int ret = 0;
	//print logo;
	if (fexist(RECEIPT_LOGO_FILE) != -1) {
		unsigned char* gSigToPrn = malloc(20000 * sizeof(unsigned char));
		PrnBmp(RECEIPT_LOGO_FILE, 0, 0, gSigToPrn);
		logd(("PrnBmp::ret = %d", ret));
		free(gSigToPrn);
	}
	else {
		logd(("No logo file"));
	}
	
	logd(("Slipheader: %s", glPosParams.slipHeader));
	logd(("Merchant Location: %s", glPosParams.merchantLocation));

	if (strlen(glPosParams.slipHeader) > 0) {
		prnCenter(glPosParams.slipHeader);
		logd(("slipheader set"));
	}
	else if (strlen(glPosParams.merchantName) > 0) {
		prnCenter(glPosParams.merchantName);
		logd(("merchantname set"));
	}

	if (strlen(glPosParams.merchantLocation) > 0) {
		logd(("merchant location set"));
		prnCenter(glPosParams.merchantLocation);
	}
	
	char date[20] = "\0";
	formatLongDate(dateTime, date);

	char time[20] = "\0";
	formatShortTime(dateTime + 8, time);
	
	prnDoubleStr(date, time);
	prnDoubleStr("TERMINAL ID", glPosParams.terminalId);
	prnDoubleStr("MERCHANT ID", glPosParams.merchantId);
	prnFullLenChar('*');
	resetPrnFormat();
	logd(("Done printing headers"));
}

static void printFooter() {
	resetPrnFormat();
	PrnStr("\n");
	logd(("Footer: %s", glPosParams.slipFooter));
	if (strlen(glPosParams.slipFooter) > 0) {
		prnCenter(glPosParams.slipFooter);
	}

	prnFullLenChar('*');

	char versionInfo[50] = "\0";
	sprintf(versionInfo, "***%s %s***", AppInfo.AppName, EDC_VER_PUB);
	prnCenter(versionInfo);

	prnCenter("www.xpresspayments.com");
	prnCenter("+23416312430");
	prnFullLenChar('-');
	prnFeedPaper(3);
	resetPrnFormat();
}

int printTransactionReceipt(TRAN_LOG* transData, int copy, uchar reprint) {
	int ret = 0;
	ret = PrnInit();
	if (ret != PRN_OK) {
		DispPrnError(ret);
		return -1;
	}
	
	resetPrnFormat();


	prnHeader(transData->szDateTime);

	//PrnDoubleHeight(1, 1);
	prnCenter(getTransactionTitle(transData->ucTranType));
	resetPrnFormat();
	prnFullLenChar('-');
	
	char* copyString = copy == CUSTOMER_COPY ? "***CUSTOMER COPY***" : "***MERCHANT COPY***";
	//PrnDoubleHeight(1, 1);
	prnCenter(copyString);

	if (reprint) {
		prnCenter("***REPRINT***");
	}

	resetPrnFormat();
	prnFullLenChar('-');

	if (transData->ucTranType == PAYATTITUDE) {
		prnDoubleStr("PHONE", transData->szHolderName);
	}
	else {
		char maskedPan[21] = "\0";
		maskPan(maskedPan, transData->szPan, '*');
		prnDoubleStr("PAN", maskedPan);
		prnDoubleStr("EXPIRY", transData->szExpDate);
		if (transData->ucTranType != KEDCO)
		{
			prnDoubleStr("NAME", transData->szHolderName);
		}
	}


	char temp[50] = "\0";
	sprintf(temp, "%06ld", glPosParams.batchNo);
	prnDoubleStr("BATCH NO", temp);

	CLEAR_STRING(temp, lengthOf(temp));
	sprintf(temp, "%06ld", transData->ulInvoiceNo);
	prnDoubleStr("SEQ NO", temp);

	CLEAR_STRING(temp, lengthOf(temp));
	sprintf(temp, "%06ld", transData->ulSTAN);
	prnDoubleStr("STAN", temp);

	prnDoubleStr("RRN", transData->szRRN);

	prnDoubleStr("AUTH ID", transData->szAuthCode);

	if (transData->ucTranType != KEDCO)
	{
		GetDispAmount(transData->szAmount, temp);
		prnDoubleStr("AMOUNT", temp);
	}

	if (atol(transData->szOtherAmount) > 0) {
		GetDispAmount(transData->szOtherAmount, temp);
		prnDoubleStr(transData->ucTranType == PURCHASE_WITH_CASH_BACK ? "CASHBACK AMOUNT" : "OTHER AMOUNT", temp);
	}

	if (transData->ucTranType != PAYATTITUDE) {
		prnDoubleStr("ACCOUNT", getAccountTypeString(transData->ucAccountType));
	}
	

	if (strlen(transData->szOrgRRN) > 0 && transData->ucTranType != REVERSAL) {
		prnDoubleStr("ORIGINAL RRN", transData->szOrgRRN);
	}

	if (transData->ucTranType != KEDCO)
	{
		PrnAttrSet(1);
		PrnDoubleHeight(1, 0);
		//PrnDoubleWidth(1, 0);
		prnCenter(isSuccessResponse(transData->szRspCode) ? "APPROVED" : "DECLINED");
		resetPrnFormat();
		prnFeedPaper(1);

		if (!isSuccessResponse(transData->szRspCode)) {
			prnCenter(transData->szResponseReason);
		}

		prnFullLenChar('-');
	}else
	{
		char * storedInfo = (char *)calloc(strlen(transData->szEchoField59), sizeof(char));
		strcpy(storedInfo, transData->szEchoField59);

		PrnAttrSet(1);
		PrnDoubleHeight(1, 0);
		prnCenter(strtok(storedInfo, "|"));
		resetPrnFormat();
		prnFeedPaper(1);

		if (!isSuccessResponse(transData->szRspCode)) {
			prnCenter(transData->szResponseReason);
		}

		prnFullLenChar('-');

		prnDoubleStr("NAME", strtok(NULL, "|"));
		prnDoubleStr("METER", strtok(NULL, "|"));
		prnDoubleStr("ACCOUNT.", strtok(NULL, "|"));
		prnDoubleStr("ADDRESS", strtok(NULL, "|"));
		prnDoubleStr("TARIFF", strtok(NULL, "|"));
		prnDoubleStr("RATE", strtok(NULL, "|"));
		prnDoubleStr("AMOUNT", strtok(NULL, "|"));
		prnDoubleStr("OPERATOR", strtok(NULL, "|"));

		free(storedInfo);

		prnFullLenChar('-');
	}

	if (transData->sAppCrypto[0]) {

		//------Added to print CVM result-----
		char CVMResult[25] = { 0 };
		if (!strncmp(transData->CVMResult, "1E", 2)) strcpy(CVMResult, "Signature");
		else if (!strncmp(transData->CVMResult, "1F", 2)) strcpy(CVMResult, "No CVM");
		else if (!strncmp(transData->CVMResult, "41", 2)) strcpy(CVMResult, "Plain Offline");
		else if (!strncmp(transData->CVMResult, "42", 2)) strcpy(CVMResult, "Enciphered Online");
		else if (!strncmp(transData->CVMResult, "44", 2)) strcpy(CVMResult, "Enciphered offline");
		else strcpy(CVMResult, "Unknown");
		prnDoubleStr("CVM", CVMResult);
		//-----------------------------------------

		CLEAR_STRING(temp, lengthOf(temp));
		PubBcd2Asc0(transData->sAID, transData->ucAidLen, temp);
		prnDoubleStr("AID", temp);

		prnDoubleStr("CARD", strlen(transData->szAppPreferName) > 0 ? transData->szAppPreferName : transData->szAppLabel);

		CLEAR_STRING(temp, lengthOf(temp));
		PubBcd2Asc0(transData->sAppCrypto, lengthOf(transData->sAppCrypto), temp);
		prnDoubleStr("AC", temp);

		CLEAR_STRING(temp, lengthOf(temp));
		PubBcd2Asc0(transData->sTVR, lengthOf(transData->sTVR), temp);
		prnDoubleStr("TVR", temp);

		CLEAR_STRING(temp, lengthOf(temp));
		PubBcd2Asc0(transData->sTSI, lengthOf(transData->sTSI), temp);
		prnDoubleStr("TSI", temp);
	}

	printFooter();

	ret = PrnStart();
	if (ret != PRN_OK) {
		DispPrnError(ret);
		return -1;
	}


	return 0;
}


int printTerminalDetails() {
	int ret = PrnInit();
	if (ret != PRN_OK) {
		DispPrnError(ret);
		return -1;
	}

	resetPrnFormat();

	char dateTime[15 + 1] = "\0";
	GetDateTime(dateTime);
	prnHeader(dateTime);

	//PrnDoubleHeight(1, 1);
	prnCenter("MENU DOWNLOAD");
	resetPrnFormat();
	prnFullLenChar('-');

	prnDoubleStr("HOST IP", glPosParams.commConfig.stWirlessPara.stHost1.szIP);
	prnDoubleStr("HOST PORT", glPosParams.commConfig.stWirlessPara.stHost1.szPort);
	prnDoubleStr("HOST DOMAIN", glPosParams.switchHostName);

	char temp[32] = "\0";
	sprintf(temp, "%d Minutes", glPosParams.callHomeTimeMinutes);
	prnDoubleStr("CALL HOME TIME", temp);

	//prnDoubleStr("INSTITUTION CODE", glPosParams.institutionCode);
	//prnDoubleStr("CONSULTANT CODE", glPosParams.consultantCode);

	printFooter();
	ret = PrnStart();
	if (ret != PRN_OK) {
		DispPrnError(ret);
		return -1;
	}


	return 0;
}


static int printSummaryReportFooter(SummaryReport* report) {
	resetPrnFormat();
	char amount[100] = "\0";
	sprintf(amount, "%d", report->reportCount);
	prnDoubleStr("COUNT", amount);

	/*char unpushed[5 + 1] = "\0";
	snprintf(unpushed, lengthOf(unpushed), "%d", getUnPushedTransactionCount());
	prnDoubleStr("UN-PUSHED COUNT", unpushed);*/

	prnFeedPaper(1);
	PrnStr("------------SUMMARY------------");
	prnFeedPaper(1);

	char totalApproved[100] = "\0";
	getDisplayLongAmount(report->totalApproved, totalApproved);
	prnDoubleStr("APPROVED", totalApproved);

	char totalDeclined[100] = "\0";
	getDisplayLongAmount(report->totalDeclined, totalDeclined);
	prnDoubleStr("DECLINED", totalDeclined);

	PrnStr("\n");
	prnFullLenChar('-');
	PrnStr("\n");

	printFooter();

	return APP_SUCC;
}

static int printSummaryData(SummaryReport* report) {
	resetPrnFormat();
	int i = 0;

	while (i < report->reportCount) {
		ReportData current = report->data[i];
		prnDoubleStr("TYPE", getTransactionTitle(current.tranType));
		prnDoubleStr("STATUS",
			isSuccessResponse(current.responseCode) ? "APPROVED" : "DECLINED");
		prnDoubleStr("PAN", current.maskedPan);
		prnDoubleStr("EXPIRY", current.cardExpiry);
		prnDoubleStr("RRN", current.RRN);

		char seqNo[7] = "\0";
		snprintf(seqNo, 7, "%06d", current.sequenceNumber);
		prnDoubleStr("SEQ NO", seqNo);

		char batchNo[7] = "\0";
		snprintf(batchNo, 7, "%06d", current.batchNumber);
		prnDoubleStr("BATCH NO", batchNo);

		char amount[100] = "\0";
		GetDispAmount(current.amount, amount);
		prnDoubleStr("AMOUNT", amount);

		
		if (current.otherAmount != NULL && (strlen(current.otherAmount) > 0) &&  current.tranType == CASH_ADVANCE) {
			char otherAmount[100] = "\0";
			GetDispAmount(current.otherAmount, otherAmount);
			prnDoubleStr("OTHER AMOUNT", otherAmount);
		}

		char date[100] = "\0";
		formatLongDate(current.date, date);
		prnDoubleStr("DATE", date);
		i++;
		if (i < report->reportCount) {
			prnFullLenChar('-');
		}
	}

	prnFeedPaper(1);
	prnFullLenChar('*');
	prnFeedPaper(1);
}

int printSummaryReport(SummaryReport* summary, char* title) {
	int ret = PrnInit();
	if (ret != PRN_OK) {
		DispPrnError(ret);
		return -1;
	}

	resetPrnFormat();

	char dateTime[15 + 1] = "\0";
	GetDateTime(dateTime);
	prnHeader(dateTime);

	//PrnDoubleHeight(1, 1);
	prnCenter(title);
	resetPrnFormat();
	prnFullLenChar('-');
	
	printSummaryData(summary);

	printSummaryReportFooter(summary);
	ret = PrnStart();
	if (ret != PRN_OK) {
		DispPrnError(ret);
		return -1;
	}


	return 0;

}

static void prnEODData(ReportData* data, int length) {
	int i = 0;

	//PrnAttrSet(1);
	PrnDoubleHeight(1, 0);
	prnDoubleStr("TIME       RRN   ", "AMOUNT S");
	resetPrnFormat();
	prnFeedPaper(1);
	while (i < length) {
		ReportData current = data[i];
		char status = isSuccessResponse(current.responseCode) ? 'A' : 'D';

		char time[6] = "\0";
		strncpy(time, current.time, 2);
		time[2] = ':';
		strncpy(time + 3, current.time + 2, 2);

		char left[20] = "\0";
		snprintf(left, lengthOf(left), "%s  %s", time, current.RRN);


		char right[25] = "\0";
		double totalAmount = (atol(current.amount) + atol(current.otherAmount)) / 100.0;
		snprintf(right, lengthOf(right), "%.2f  %c ", totalAmount, status);

		prnDoubleStr(left, right);
		i++;
	}
	resetPrnFormat();
	prnFeedPaper(1);

}

int printEODReport(SummaryReport* summaryReport, char* title, int type) {

	int ret = PrnInit();
	if (ret != PRN_OK) {
		DispPrnError(ret);
		return -1;
	}

	resetPrnFormat();

	char dateTime[15 + 1] = "\0";
	GetDateTime(dateTime);
	prnHeader(dateTime);

	//PrnDoubleHeight(1, 1);
	prnCenter(title);
	resetPrnFormat();
	prnFullLenChar('-');

	if (type != CLOSE_BATCH_COPY) {
		PrnDoubleHeight(1, 0);
		char* d = summaryReport->data[0].date;

		char dateString[12 + 1] = "\0";
		strncpy(dateString, d + 6, 2);
		dateString[strlen(dateString)] = '/';
		strncpy(dateString + strlen(dateString), d + 4, 2);
		dateString[strlen(dateString)] = '/';
		strncpy(dateString + strlen(dateString), d, 4);
		prnCenter(dateString);

		resetPrnFormat();
		prnFeedPaper(1);
	}

	if (type == EOD_FULL_COPY) {
		prnFeedPaper(1);
		prnEODData(summaryReport->data, summaryReport->reportCount);
	}


	prnFeedPaper(1);
	prnFullLenChar('*');
	prnFeedPaper(1);

	printSummaryReportFooter(summaryReport);

	ret = PrnStart();
	if (ret != PRN_OK) {
		DispPrnError(ret);
		return -1;
	}


	return 0;
}