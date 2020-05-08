/*
* nibssTransaction.c
*
*  Created on: Aug 20, 2018
*      Author: ayodeji.bamitale
*/

#include "global.h"
#include "xIsoDef.h"
#include "xTime.h"
#include "dl_iso8583.h"
#include "dl_iso8583_defs_1987.h"
#include "dl_output.h"
#include "xui.h"
#include "utils.h"
#include "converters.h"

#define  POS_PIN_CAPTURE_CODE "12"
#define AMOUNT_TRANSACTION_FEE "D00000000"
#define POS_DATA_CODE  "51011151134C101" //"510111511344101" //"510101511344101" switched subfield 5 0 for 1 to pass MTIP validation
#define POS_DATA_CODE_CLSS "51010171134C101"

#define transData glProcInfo.stTranLog

static DL_ISO8583_HANDLER isoHandler;
static DL_ISO8583_MSG isoMsg;


const char* getEntryMode(int uiEntryMode) {

	if (uiEntryMode & MODE_MANUAL_INPUT) {
		return "011";
	}
	else if (uiEntryMode & MODE_SWIPE_INPUT) {
		return "021";
	}
	else if (uiEntryMode & MODE_CHIP_INPUT) {
		return "051";
	}
	else if (uiEntryMode & MODE_FALLBACK_SWIPE)
	{
		return  "801";
	}
	else if (uiEntryMode & MODE_CONTACTLESS)
	{
		return  "071";
	}
	else {
		return "\0";
	}
}
/**
*
* @param track2Data [in]
* @param acqInstitutionCode [out]
*/
void getAcquiringInstitutionCode(char * track2Data,
	char acqInstitutionCode[6 + 1]);


/**
*
* @param originalMTI
* @param transData
* @return 1 - success, 0 - fail
*/
int setOriginalDataElementField90();

/**
*
* @param transData
* @return
*/
int setReplaceAmountField95();

/**
*
* @param track2Data [in]
* @param serviceCode [out]
*/
void getServiceCode(char* track2Data, char serviceCode[3 + 1]);

/**
*
* @param track2Data [in]
* @param expiryDate [out]
*/
void getExpiryDate(char* track2Data, char expiryDate[4 + 1]);

int setBaseTransactionField() {
	DL_ISO8583_DEFS_1987_Nibss_GetHandler(&isoHandler);
	DL_ISO8583_MSG_Init(NULL, 0, &isoMsg);

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(MESSAGE_TYPE_INDICATOR_0,
			getMTI(glProcInfo.stTranLog.ucTranType), &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(PRIMARY_ACCOUNT_NUMBER_2,
			glProcInfo.stTranLog.szPan, &isoMsg));


	char processingCode[6 + 1] = "\0";
	logd(("Tran Type: %d", transData.ucTranType));
	getProcessingCode(transData.ucTranType, transData.ucAccountType, processingCode);
	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(PROCESSING_CODE_3,
			processingCode, &isoMsg));

	char totalAmount[12 + 1] = "\0";
	sprintf(totalAmount, "%012ld",
		(atol(transData.szAmount) + atol(transData.szOtherAmount)));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(TRANSACTION_AMOUNT_4, totalAmount, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(TRANSACTION_DATE_TIME_7,
			transData.szDateTime+4, &isoMsg));
	
	char stan[6 + 1] = "\0";
	sprintf(stan, "%06ld", transData.ulSTAN);
	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(SYSTEM_TRACE_AUDIT_NUMBER_11,
			stan, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(LOCAL_TIME_OF_TRANSACTION_12,
			transData.szDateTime+8, &isoMsg));

	char date[4 + 1] = "\0";
	strmcpy(date, transData.szDateTime+4, lengthOf(date));
	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(LOCAL_DATE_OF_TRANSACTION_13,
			date, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(EXPIRATION_DATE_14,
			transData.szExpDate, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(MERCHANT_TYPE_18,
			glPosParams.nibssParams.merchantCategoryCode, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(POS_ENTRY_MODE_22,
			getEntryMode(transData.uiEntryMode), &isoMsg));

	if (transData.bPanSeqOK) {
		char panSeqNo[3 + 1] = "\0";
		logd(("Pan Sequence Number: %d", transData.ucPanSeqNo));
		sprintf(panSeqNo, "%03d", transData.ucPanSeqNo);
		ASSERT_RETURNCODE(
			DL_ISO8583_MSG_SetField_Str(CARD_SEQUENCE_NUMBER_23, panSeqNo, &isoMsg));
	}

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(POS_CONDITION_CODE_25,
			"00", &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(POS_PIN_CAPTURE_CODE_26,
			"12", &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(AMOUNT_TRANSACTION_FEE_28, AMOUNT_TRANSACTION_FEE, &isoMsg));


	char acquiringIdCode[11 + 1] = "\0";
	getAcquiringInstitutionCode(glProcInfo.szTrack2, acquiringIdCode);
	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(
			ACQUIRING_INSTITUTION_IDENTIFICATION_CODE_32,
			acquiringIdCode,
			&isoMsg));

	if (strlen(glProcInfo.szTrack2) > 37) {
		glProcInfo.szTrack2[37] = '\0';
	}

	int j = 0;
	for (j = 0; j < strlen(glProcInfo.szTrack2); j++) {
		if (glProcInfo.szTrack2[j] == '=') {
			glProcInfo.szTrack2[j] = 'D';
		}
	}

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(TRACK2_DATA_35, glProcInfo.szTrack2, &isoMsg));

	if (strlen(transData.szRRN) != 12) {
		getRRN(transData.szRRN);
	}

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(RETRIVAL_REFERENCE_NUMBER_37,
			transData.szRRN, &isoMsg));

	char serviceCode[3 + 1] = "\0";
	getServiceCode(glProcInfo.szTrack2, serviceCode);
	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(SERVICE_RESTRICTION_CODE_40,
			serviceCode, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(
			CARD_ACCEPTOR_TERMINAL_IDENTIFICATION_41,
			glPosParams.terminalId, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(CARD_ACCEPTOR_IDENTIFICATION_CODE_42,
			glPosParams.nibssParams.cardAcceptiorIdentificationCode, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(CARD_ACCEPTOR_NAME_OR_LOCATION_43,
			glPosParams.nibssParams.merchantNameAndLocation, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(TRANSACTION_CURRENCY_CODE_49,
			glPosParams.nibssParams.currencyCode, &isoMsg));

	if (glProcInfo.sPinBlock != NULL
		&& memcmp(glProcInfo.sPinBlock, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) != 0) {
		char pinBlock[16 + 1] = "\0";
		PubBcd2Asc0(glProcInfo.sPinBlock, lengthOf(glProcInfo.sPinBlock), pinBlock);
		ASSERT_RETURNCODE(DL_ISO8583_MSG_SetField_Str(PIN_DATA_52, pinBlock, &isoMsg));
	}

	char iccData[1000] = "\0";
	if (transData.uiIccDataLen > 0 && transData.sIccData != NULL) {
		PubBcd2Asc0(transData.sIccData, transData.uiIccDataLen, iccData);

		/*if (transData.uiEntryMode & MODE_CONTACTLESS) {
			strcat(iccData, "9F6E0420700000");
		}*/
		ASSERT_RETURNCODE(DL_ISO8583_MSG_SetField_Str(ICC_DATA_55, iccData, &isoMsg));
	}
	else if (!(transData.uiEntryMode & MODE_SWIPE_INPUT) && !(transData.uiEntryMode & MODE_FALLBACK_SWIPE)) {
		ASSERT_RETURNCODE(DL_ISO8583_MSG_SetField_Str(ICC_DATA_55, iccData, &isoMsg));
	}

	if (transData.szEchoField59 && strlen(transData.szEchoField59) > 0) {
		ASSERT_RETURNCODE(
			DL_ISO8583_MSG_SetField_Str(ECHO_DATA_59,
				transData.szEchoField59, &isoMsg));
	}

	if (transData.uiEntryMode & MODE_CONTACTLESS) {
		ASSERT_RETURNCODE(
			DL_ISO8583_MSG_SetField_Str(POS_DATA_CODE_123, POS_DATA_CODE_CLSS, &isoMsg));
	}
	else {
		ASSERT_RETURNCODE(
			DL_ISO8583_MSG_SetField_Str(POS_DATA_CODE_123, POS_DATA_CODE, &isoMsg));
	}
	

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(SECONDARY_MESSAGE_HASH_VALUE_128, "\0", &isoMsg));

	return APP_SUCC;
}

/**
* @return APP_SUCC, APP_FAIL, APP_TIMEOUT, APP_CANCEL
*/
int processNibssTransaction() {

	DL_UINT8 packBuf[0x1000] = "\0", responseBuf[0x2000] = "\0";
	DL_UINT16 packedSize = 0, responseSize = 0;

	if (transData.ucTranType == REVERSAL) {
		transData.ulSTAN = transData.ulOrgSTAN;
		strmcpy(transData.szDateTime, transData.szOrgDateTime, lengthOf(transData.szDateTime));

		strcpy(transData.szRRN, transData.szOrgRRN);

		memset(transData.sIccData, 0, lengthOf(transData.sIccData));
		transData.uiIccDataLen = 0;
	}

	if (setBaseTransactionField(transData) != APP_SUCC) {
		return APP_FAIL;
	}

	switch (transData.ucTranType) {
	case PURCHASE_WITH_CASH_BACK: {
		char additionalAmounts[120 + 1] = "\0";
		sprintf(additionalAmounts, "%s05%sD%s",
			getIsoAccountType(transData.ucAccountType), glPosParams.nibssParams.currencyCode,
			transData.szOtherAmount);
		if (DL_ISO8583_MSG_SetField_Str(ADDITIONAL_AMOUNTS_54,
			additionalAmounts, &isoMsg) != 0) {
			return APP_FAIL;
		}
	}
		break;
	case REVERSAL: {
		ASSERT_RETURNCODE(
			DL_ISO8583_MSG_SetField_Str(MESSAGE_REASON_CODE_56,
				getReversalReasonCode(REASON_TIME_OUT), &isoMsg));
	}
	case REFUND:
		ASSERT_RETURNCODE(setReplaceAmountField95(transData))
	case POS_PRE_AUTH_COMPLETION:
		ASSERT_RETURNCODE(setOriginalDataElementField90(transData))
			if (!IS_NULL_EMPTY(transData.szAuthCode)) {
				ASSERT_RETURNCODE(
					DL_ISO8583_MSG_SetField_Str(AUTHORIZATION_CODE_38,
						transData.szAuthCode, &isoMsg))
			}
		break;
	}

	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf + 2, &packedSize);

	//apply hex bitmap fix for nibss extended messaging
	packBuf[6] = 'F';
	packBuf[7] = '2';

	PubTrimStr(packBuf + 2);

	char hash[64 + 1] = "\0";
	calculateSHA256Digest(packBuf + 2, glPosParams.hostSessionKey, hash);

	(void)DL_ISO8583_MSG_SetField_Str(SECONDARY_MESSAGE_HASH_VALUE_128, hash, &isoMsg);
	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);

	memset(packBuf, lengthOf(packBuf), sizeof(char));
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf + 2, &packedSize);
	//	packedSize = strlen(packBuf+2); // packing not setting length, most likely because of the extended higi-haga
	packBuf[6] = 'F';
	packBuf[7] = '2';
	logTrace("Packed data: %s, length: %d", packBuf + 2, packedSize);

	DL_ISO8583_MSG_Free(&isoMsg);

	packBuf[0] = packedSize >> 8;
	packBuf[1] = packedSize % 256;
	packedSize += 2;

	int ret = sendSocketRequest(packBuf, packedSize, responseBuf, &responseSize);
	if (ret != 0) {
		showErrorDialog("COMM ERROR", 30);
		return ret;
	}
	logTrace("Response data: %s, length: %d", responseBuf + 2, responseSize);

	DL_ISO8583_MSG_Init(NULL, 0, &isoMsg);

	/* unpack ISO message */
	if (DL_ISO8583_MSG_Unpack(&isoHandler, responseBuf + 2, responseSize - 2, &isoMsg) != 0 || !isoMsg.field[39].ptr) {//account for 2 byte header
		DL_ISO8583_MSG_Free(&isoMsg);

		showErrorDialog("Message unpack error.\nPlease re-configure terminal.", DEFAULT_PASSWORD_TIMEOUT);

		memcpy(transData.szRspCode, INVALID_RESPONSE_CODE, lengthOf(transData.szRspCode) - 1);
		memcpy(transData.szResponseReason, responseCodeToString(INVALID_RESPONSE_CODE), lengthOf(transData.szResponseReason) - 1);

		memset(glProcInfo.sResponseIcc, 0, lengthOf(glProcInfo.sResponseIcc));
		glProcInfo.uiResponseIccLen = 0;

		return APP_SUCC; //Comms completed successfully, issue was the data received
	}

	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);

	DL_ISO8583_MSG_FIELD field39 = isoMsg.field[39];
	memcpy(transData.szRspCode, field39.ptr, lengthOf(transData.szRspCode) - 1);
	memcpy(transData.szResponseReason, responseCodeToString(field39.ptr), lengthOf(transData.szResponseReason) - 1);

	DL_ISO8583_MSG_FIELD field38 = isoMsg.field[AUTHORIZATION_CODE_38];
	if (field38.ptr) {
		memcpy(transData.szAuthCode, field38.ptr, lengthOf(transData.szAuthCode) - 1);
	}

	DL_ISO8583_MSG_FIELD field33 =
		isoMsg.field[FORWARDING_INSTITUTION_IDENTIFICATION_CODE_33];
	if (field33.ptr) {
		memcpy(transData.szOriginalForwdInstCode, field33.ptr, lengthOf(transData.szOriginalForwdInstCode) - 1);
	}

	DL_ISO8583_MSG_FIELD field55 = isoMsg.field[ICC_DATA_55];
	if (field55.ptr) {
		memset(glProcInfo.sResponseIcc, 0, lengthOf(glProcInfo.sResponseIcc));
		PubAsc2Bcd(field55.ptr, field55.len, glProcInfo.sResponseIcc);
		glProcInfo.uiResponseIccLen = field55.len / 2;

		logHexString("Response DE 55: ", glProcInfo.sResponseIcc, glProcInfo.uiResponseIccLen);
	}
	else {
		memset(glProcInfo.sResponseIcc, 0, lengthOf(glProcInfo.sResponseIcc));
		glProcInfo.uiResponseIccLen = 0;
	}

	DL_ISO8583_MSG_FIELD field54 = isoMsg.field[ADDITIONAL_AMOUNTS_54];
	if (field54.ptr) {
		strmcpy(glProcInfo.szAdditionalAmtF54, field54.ptr, lengthOf(glProcInfo.szAdditionalAmtF54));
	}

	DL_ISO8583_MSG_Free(&isoMsg);

	return APP_SUCC;
}

/**
*
* @param data [in]
* @param reason [in]
* @return APP_SUCC, APP_FAIL
*/
int rollbackNibssTransaction(int reason) {

	DL_UINT8 packBuf[0x1000] = "\0", responseBuf[0x2000] = "\0";
	DL_UINT16 packedSize = 0, responseSize = 0;

	const char* reversalReasonCode = getReversalReasonCode(reason);

	char totalAmount[12 + 1] = "\0";
	sprintf(totalAmount, "%012ld",
		(atol(transData.szAmount) + atol(transData.szOtherAmount)));
	logTrace("Total Amount: %s", totalAmount);

	transData.ucOrgTranType = transData.ucTranType;
	transData.ulOrgSTAN = transData.ulSTAN;
	CLEAR_STRING(transData.szOrgDateTime,  lengthOf(transData.szOrgDateTime));
	strmcpy(transData.szOrgDateTime, transData.szDateTime, lengthOf(transData.szOrgDateTime));

	memset(transData.sIccData, 0, lengthOf(transData.sIccData));
	transData.uiIccDataLen = 0;

	ASSERT_RETURNCODE(setBaseTransactionField());

	char processingCode[6] = "\0";
	getProcessingCode(REVERSAL, transData.ucAccountType, processingCode);
	DL_ISO8583_MSG_SetField_Str(PROCESSING_CODE_3, processingCode, &isoMsg);

	char* mti;
	if (transData.ucOrgTranType == REVERSAL) {
		mti = REVERSAL_ADVICE_REPEAT_MTI;
	}
	else {
		mti = REVERSAL_ADVICE_MTI;
	}

	DL_ISO8583_MSG_SetField_Str(MESSAGE_TYPE_INDICATOR_0, mti, &isoMsg);

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(TRANSACTION_AMOUNT_4, totalAmount, &isoMsg))

	logTrace("Reversal Reason code: %s", reversalReasonCode);
	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(MESSAGE_REASON_CODE_56,
			reversalReasonCode, &isoMsg))
	ASSERT_RETURNCODE(setOriginalDataElementField90());
	ASSERT_RETURNCODE(
			DL_ISO8583_MSG_SetField_Str(REPLACEMENT_AMOUNTS_95,
				"000000000000000000000000D00000000D00000000", &isoMsg))

	CLEAR(transData.szRspCode, 0);
	CLEAR(transData.szResponseReason, 0);

	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf + 2, &packedSize);

	//apply hex bitmap fix for nibss extended messaging
	packBuf[6] = 'F';
	packBuf[7] = '2';

	PubTrimStr(packBuf + 2);

	char hash[64 + 1] = "\0";
	calculateSHA256Digest(packBuf + 2, glPosParams.hostSessionKey, hash);

	(void)DL_ISO8583_MSG_SetField_Str(SECONDARY_MESSAGE_HASH_VALUE_128, hash, &isoMsg);
	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);

	memset(packBuf, lengthOf(packBuf), sizeof(char));
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf + 2, &packedSize);
	packedSize = strlen(packBuf + 2); // packing not setting length, most likely because of the extended higi-haga
	packBuf[6] = 'F';
	packBuf[7] = '2';
	logTrace("Packed data: %s, length: %d", packBuf + 2, packedSize);

	DL_ISO8583_MSG_Free(&isoMsg);

	packBuf[0] = packedSize >> 8;
	packBuf[1] = packedSize % 256;
	packedSize += 2;

	int ret = sendSocketRequest(packBuf, packedSize, responseBuf, &responseSize);
	if (ret != 0) {
		showErrorDialog("COMM ERROR. Auto-Reversal failed", 30);
		memcpy(transData.szRspCode, COMM_ERROR_CODE,
			lengthOf(transData.szRspCode) - 1);
		memcpy(transData.szResponseReason, "Auto-reversal failed",
			sizeof(transData.szResponseReason));
		return ret;
	}

	if (DL_ISO8583_MSG_Unpack(&isoHandler, responseBuf + 2, responseSize - 2, &isoMsg) != 0 || !isoMsg.field[RESPONSE_CODE_39].ptr) {//account for 2 byte header
		DL_ISO8583_MSG_Free(&isoMsg);

		showErrorDialog("Message unpack error. Please re-configure terminal.", DEFAULT_PASSWORD_TIMEOUT);

		memcpy(transData.szRspCode, INVALID_RESPONSE_CODE, lengthOf(transData.szRspCode) - 1);
		memcpy(transData.szResponseReason, "Auto-reversal failed", sizeof(transData.szResponseReason));

		memset(glProcInfo.sResponseIcc, 0, lengthOf(glProcInfo.sResponseIcc));
		glProcInfo.uiResponseIccLen = 0;

		return APP_SUCC; //Comms completed successfully, issue was the data received
	}

	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);
	DL_ISO8583_MSG_FIELD field39 = isoMsg.field[RESPONSE_CODE_39];

	if (!isSuccessResponse(field39.ptr)) {
		memcpy(transData.szRspCode, field39.ptr, lengthOf(transData.szRspCode) - 1);
		memcpy(transData.szResponseReason, "Auto-reversal failed",
			sizeof(transData.szResponseReason));
		showErrorDialog(transData.szResponseReason, 10);
	}
	else {
		memcpy(transData.szRspCode, ERROR_RESPONSE_CODE,
			lengthOf(transData.szRspCode) - 1);
		memcpy(transData.szResponseReason, "Auto-reversal successful",
			sizeof(transData.szResponseReason));
	}

	memset(glProcInfo.sResponseIcc, 0, lengthOf(glProcInfo.sResponseIcc));
	glProcInfo.uiResponseIccLen = 0;

	DL_ISO8583_MSG_Free(&isoMsg);
	return APP_SUCC;
}

/**
*
* @param track2Data [in]
* @param acqInstitutionCode [out]
*/
void getAcquiringInstitutionCode(char * track2Data,
	char acqInstitutionCode[6 + 1]) {
	memset(acqInstitutionCode, '\0', sizeof(acqInstitutionCode));
	memcpy(acqInstitutionCode, track2Data, 6);
}

/**
*
* @param track2Data [in]
* @param serviceCode [out]
*/
void getServiceCode(char* track2Data, char serviceCode[3 + 1]) {
	memset(serviceCode, '\0', sizeof(serviceCode));

	char* temp;
	if ((temp = strchr(track2Data, 'D')) || (temp = strchr(track2Data, '='))) {
		memcpy(serviceCode, temp + 5, 3);
	}
}

/**
*
* @param track2Data [in]
* @param expiryDate [out]
*/
void getExpiryDate(char* track2Data, char expiryDate[4 + 1]) {
	memset(expiryDate, '\0', sizeof(expiryDate));

	char* temp;
	if ((temp = strchr(track2Data, 'D')) || (temp = strchr(track2Data, '='))) {
		memcpy(expiryDate, temp + 1, 4);
	}
}

/**
*
* @param originalMTI
* @param transData
* @return 1 - success, 0 - fail
*/
int setOriginalDataElementField90() {
	const char* originalMTI = getMTI(transData.ucOrgTranType);

	char acqCode[11 + 1] = "\0";
	char acqInstCode[6 + 1] = "\0";
	getAcquiringInstitutionCode(glProcInfo.szTrack2, acqInstCode);
	padLeft(11, '0', acqInstCode, acqCode);

	char forwardingInstCode[11 + 1] = "\0";
	if (transData.szOriginalForwdInstCode != NULL
		&& strlen(transData.szOriginalForwdInstCode) > 0) {
		padLeft(11, '0', transData.szOriginalForwdInstCode,
			forwardingInstCode);
	}
	else {
		sprintf(forwardingInstCode, "%011d", 0);
	}

	char originalDataElements[42 + 1] = "\0";
	CLEAR(originalDataElements, '\0');

	sprintf(originalDataElements, "%s%06ld%s%s%s", originalMTI,
		transData.ulOrgSTAN, transData.szOrgDateTime+4,
		acqCode, forwardingInstCode);

	logTrace("Original Data elements %s", originalDataElements);

	return DL_ISO8583_MSG_SetField_Str(ORIGINAL_DATA_ELEMENTS_90, originalDataElements, &isoMsg) == 0 ? APP_SUCC : APP_FAIL;
}

/**
*
* @param transData
* @return
*/
int setReplaceAmountField95() {
	long oldAmount = atol(transData.szOrgAmount);
	long currentAmount = atol(transData.szAmount);

	long replacementAmount = oldAmount - currentAmount;

	char data[42 + 1] = "\0";
	sprintf(data, "%012ld%012ldD00000000D00000000", replacementAmount, replacementAmount);
	logTrace("Field 95: %s", data);
	return DL_ISO8583_MSG_SetField_Str(REPLACEMENT_AMOUNTS_95, data, &isoMsg) == 0 ? APP_SUCC : APP_FAIL;
}

