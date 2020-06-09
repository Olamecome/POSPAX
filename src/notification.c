#include "global.h"
#include "utils.h"
#include "xIsoDef.h"
#include "http_handler.h"
#include "logger.h"
#include "stdbool.h"
#include "converters.h"
#include "parsonHelper.h"

extern const char* getEntryMode(int uiEntryMode);

#define TEST_NOTIFICATION_URL "http://80.88.8.245:1211/tms/rws/notification"
#define LIVE_NOTIFICATION_URL "http://80.88.8.24:1211/tms/rws/notification"

const char* TMS_BRIDGE_TEMPLATE = "{"
"\"merchantId\":\"%s\","
"\"user\":\"tms\","
"\"password\":\"+4KJVvZb9s7h+RO1DVzSWg==\","
"\"mti\":\"%s\","
"\"amount\":\"%ld\","
"\"stan\":\"%s\","
"\"pan\":\"%s\","
"\"track2\":\"%s\","
"\"track1\":\"%s\","
"\"iccdata\":\"%s\","
"\"posentrymode\":\"%s\","
"\"posconditioncode\":\"%s\","
"\"refcode\":\"%s\","
"\"aurhorisationresponse\":\"%s\","
"\"currencycode\":\"%s\","
"\"terminalid\":\"%s\","
"\"transactiondate\":\"%s\","
"\"transactiontime\":\"%s\","
"\"responcecode\":\"%s\","
"\"trantype\":\"%s\","
"\"batchno\":\"%s\","
"\"seqno\":\"%s\","
"\"t_status\":\"%s\","
"\"responsedescription\":\"%s\","
"\"timelocaltransaction\":\"%s\","
"\"datelocaltransaction\":\"%s\","
"\"rrn\":\"%s\","
"\"processingcode\":\"%s\","
"\"trancode\": \"%d\","
"\"searchtext\":\"\","
"\"accounttype\":%d,"
"\"cellinfo\":\"%s\","
"\"gpsinfo\":\"\","
"\"otherterminalid\":\"%s\","
"\"channelCode\":\"%d\","
"\"paymentMethod\":\"%d\","
"\"revenueCode\":\"%s\","
"\"customerName\":\"%s\","
"\"narration\":\"%s\","
"\"customerOtherInfo\":\"%s\","
"\"authorizationCode\":\"%s\","
"\"tranTypeCode\":\"%d\""
"}";

const char* getNotificationTemplate() {
	return TMS_BRIDGE_TEMPLATE;
}

static int buildCardNotificationMessage(TRAN_LOG* data, char* message, int outlen) {
	logTrace(__func__);

	char* merchantId = glPosParams.nibssParams.cardAcceptiorIdentificationCode;
	char* terminalId = glPosParams.terminalId;

	char maskedPan[20 + 1] = "\0";
	maskPan(maskedPan, data->szPan, '*');
	logTrace("Masked Pan: %s", maskedPan);

	char maskedTrack2[40 + 1] = "\0";
	sprintf(maskedTrack2, "%sD%s%s0000000000", maskedPan, data->szExpDate, "000");
	logTrace("Masked Track2: %s", maskedTrack2);

	char iccData[1000] = "\0";
	if (data->uiIccDataLen > 0 && data->sIccData != NULL) {
		PubBcd2Asc0(data->sIccData, data->uiIccDataLen, iccData);
		logDirect("ICC Data: ", iccData);
	}

	char date[12] = "\0";
	char* m = data->szDateTime + 4;
	char year[4 + 1] = "\0";
	strmcpy(year, data->szDateTime, lengthOf(year));

	snprintf(date, lengthOf(date), "%s-%c%c-%c%c", year, m[0], m[1], m[2], m[3]);
	logTrace("Date: %s", date);

	char time[9] = "\0";
	formatShortTime(data->szDateTime+8, time);

	char posDate[10] = "\0";
	strmcpy(posDate, date + 2, lengthOf(posDate));
	posDate[5] = posDate[2] = '/';

	const char* szMTI = getMTI(data->ucTranType);
	logTrace("MTI %s", szMTI);

	long amount = atol(data->szAmount) + atol(data->szOtherAmount);
	logTrace("amount: %ld", amount);

	char cellInfo[30] = { 0 };
	WlInfo_T wlInfo = { 0 };	
	if ((get_wl_info(&wlInfo) == 0) && wlInfo.CellInfo.gsm) {
		WlGSMCellInfo_T* cInfo = wlInfo.CellInfo.gsm;
		char signalDesc[30] = { 0 };
		int signalStrength = GetSignal_Status(signalDesc);
		//621,60,24892,1212, 80 => mcc, mnc, lac, ci, ss
		snprintf(cellInfo, lengthOf(cellInfo),"%s,%s,%s,%s,%d", cInfo->mcc,
			cInfo->mnc, cInfo->lac, cInfo->cell, signalStrength);
	}
	logd(("CELL INFO: %s", cellInfo));

	logd(("Response code: %s", data->szRspCode));

	const char* status = (isSuccessResponse(data->szRspCode)) ? "APPROVED" : "DECLINED";
	logTrace("Status: %s", status);
	char otherTerminalId[8 + 1] = "\0";
	char narration[50] = "\0";
	char* otherCustomerInfo = "\0";
	char* revenueCode = "\0";

	int POS_CHANNEL = 2;
	int PAYMENT_METHOD = CARD;
	switch (data->ucTranType)
	{
	case PAYATTITUDE:
		PAYMENT_METHOD = WALLET;
		snprintf(narration, sizeof(narration), "PayAttitude pay with phone number %s", data->szHolderName);
		break;
	default:
		PAYMENT_METHOD = CARD;
		break;
	}

	char stan[10 + 1] = "\0";
	sprintf(stan, "%06d", data->ulSTAN);
	logTrace("STAN: %s", stan);

	char batchNo[10 + 1] = "\0";
	sprintf(batchNo,"%06d", glPosParams.batchNo);
	logTrace("BatchNo: %s", batchNo);

	char sequenceNo[10 + 1] = "\0";
	sprintf(sequenceNo,"%06d", data->ulInvoiceNo);
	logTrace("Sequen No: %s", sequenceNo);

	char processingCode[10 + 1] = "\0";
	getProcessingCode(data->ucTranType, data->ucAccountType, processingCode);
	logTrace("Processing Code: %s", processingCode);

	//logTrace("Currency code: %s", glPosParams.nibssParams.currencyCode);
	//logTrace("CardholderName: %s", data->szHolderName);
	//logTrace("Entry Mode: %s", getEntryMode(data->uiEntryMode));
	//logTrace("Echo Field: %s", data->szEchoField59);
	//logTrace("Auth code: %s", data->szAuthCode);
	//logTrace("Currency Code: %s", glPosParams.nibssParams.currencyCode);
	//logTrace("Transaction title: %s", getTransactionTitle(data->ucTranType));
	//logTrace("Cellinfo: %s", cellInfo);
	//logTrace("Status Reason: %s", data->szResponseReason);

	snprintf(message, outlen, TMS_BRIDGE_TEMPLATE, merchantId, szMTI, amount, stan, maskedPan, maskedTrack2, data->szHolderName,
		iccData, getEntryMode(data->uiEntryMode), "00", data->szEchoField59, data->szAuthCode, glPosParams.nibssParams.currencyCode, 
		terminalId, date, time, data->szRspCode,
		getTransactionTitle(data->ucTranType), batchNo, sequenceNo, status, data->szResponseReason, time, posDate, data->szRRN,
		processingCode, data->ucTranType, data->ucAccountType, cellInfo, otherTerminalId, POS_CHANNEL, PAYMENT_METHOD, revenueCode, 
		data->szHolderName, narration, otherCustomerInfo, data->szAuthCode, data->ucTranType);

	logTrace("Notification data built");
	//logDirect("Request", message);

	return 0;
}


int doNewNotification(char* requestData) {
	logTrace(__func__);
	MemoryStruct chunk = { 0 };
	const char* headers[2] = { "Content-Type: application/json",
		"Accept: application/json" };

	char* URL;
#ifdef APP_DEBUG
	URL = TEST_NOTIFICATION_URL;
#else
	URL = LIVE_NOTIFICATION_URL;
#endif

	int ret = sendHttpRequest(HTTP_POST, URL, requestData, strlen(requestData), headers, 2, &chunk);
	logTrace("send Http Request: %d", ret);
	if (ret != 0) {
		if (chunk.memory) {
			free(chunk.memory);
		}

		return -1;
	}

	logDirect("Notification Response: %s", chunk.memory);
	JsonValue root = json_parse_string(chunk.memory);
	if (!root || (json_value_get_type(root) != JSONObject)) {
		showErrorDialog("Invalid Response Received", 10);
		free(chunk.memory);
		return;
	}

	free(chunk.memory);

	Json jsonObj = json_value_get_object(root);
	char* responseCode = json_object_get_string(jsonObj, "responseCode");
	char* responseMessage = json_object_get_string(jsonObj, "responseMessage");

	if (!responseCode || !responseMessage) {
		ret = -1;
		showErrorDialog("Invalid response", 10);
		goto EXIT;
	}

	logTrace("response: %s", responseMessage);

	if (!isSuccessResponse(responseCode)) {
		if (stringEqual(responseCode, "26")) {
			// Duplicate record, set as successful
			ret = 0;
			goto EXIT;
		}
		else {
			ret = -1;
		}

		showErrorDialog(responseMessage, 10);
	}

EXIT:
	json_value_free(root); 
	return ret;
}

int sendCardNotification(TRAN_LOG* data) {
	char request[0x3000] = { 0 };

	buildCardNotificationMessage(data, request, lengthOf(request)-1);

	return doNewNotification(request);

}

void* notificationHandler(void* arg) {
	RunnerData * data = (RunnerData *)arg;

	TRAN_LOG * eft = (TRAN_LOG *)data->data;
	int ret = sendCardNotification(eft);
	logTrace("SendNotification result: %i", ret);
	data->result = ret;

	eft->notified = !ret;  // ret == 0 ? "true" : false;

	UpdateTranLog(eft, data->recordIdx);

	//NDK_PthreadExit(&data->result);
	return ret;
}