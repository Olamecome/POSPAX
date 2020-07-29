#include "global.h"
#include "xIsoDef.h"
#include "xTime.h"
#include "dl_iso8583.h"
#include "dl_iso8583_defs_1987.h"
#include "dl_output.h"
#include "xui.h"
#include "utils.h"
#include "converters.h"
#include "Logger.h"

static DL_ISO8583_HANDLER isoHandler;
static DL_ISO8583_MSG isoMsg;

extern void(*receiveDisplay)();
extern int statusReceiptAndNotification();
extern int buildNibssKeyRequestNew(int nibssKeyType, char* downloadedKey);

static int processPayAttitudeTransaction(char* phoneNumber, char* amount);
static int setFields();
static void adjustPayAttitudeComms();

int downloadPayAttitudeKeys() {
	
	if (!glPosParams.isPayAttitudeEnabled) {
		return 0;
	}

	adjustPayAttitudeComms();
	
	char masterKeyData[ASCII_KEY_SIZE + ASCII_KCV_SIZE + 1] = "\0";
	char sessionKeyData[ASCII_KEY_SIZE + ASCII_KCV_SIZE + 1] = "\0";

	uchar masterKey[BYTE_KEY_SIZE] = { 0 };
	uchar sessionKey[BYTE_KEY_SIZE] = { 0 };

	uchar zmk[BYTE_KEY_SIZE] = { 0 };
	uchar clrMK[BYTE_KEY_SIZE] = { 0 };
	uchar clrSK[BYTE_KEY_SIZE] = { 0 };

	DispMessage("Configuring PayAttitude");
	DelayMs(1000);

	if (0 != buildNibssKeyRequestNew(TYPE_MASTER_KEY, masterKeyData)) {
		return -1;
	}
	showInfo(NULL, 1, 1, "MASTER KEY Ok");
	logTrace("Masterkey: %s: ", masterKeyData);

	if (0 != buildNibssKeyRequestNew(TYPE_SESSION_KEY, sessionKeyData)) {
		return -1;
	}
	logTrace("Sessionkey: %s: ", sessionKeyData);
	showInfo(NULL, 1, 1, "SESSION KEY Ok");
	

	PubAsc2Bcd(masterKeyData, ASCII_KEY_SIZE, masterKey);

	PubAsc2Bcd(glPosParams.payAttitudeZmk, ASCII_KEY_SIZE, zmk);
	des3EcbDecrypt(zmk, masterKey, BYTE_KEY_SIZE, clrMK);

	logHexString("ZMK ", zmk, BYTE_KEY_SIZE);
	logHexString("ENC MK ", masterKey, BYTE_KEY_SIZE);
	logHexString("CLEAR MK ", clrMK, BYTE_KEY_SIZE);

	PubAsc2Bcd(sessionKeyData, ASCII_KEY_SIZE, sessionKey);
	des3EcbDecrypt(clrMK, sessionKey, BYTE_KEY_SIZE, clrSK);

	memset(glPosParams.payAttitudeSessionKey, 0, sizeof(glPosParams.payAttitudeSessionKey));
	PubBcd2Asc0(clrSK, BYTE_KEY_SIZE, glPosParams.payAttitudeSessionKey);
	logHexString("ENC SK ", sessionKey, BYTE_KEY_SIZE);
	logHexString("CLEAR SK ", clrSK, BYTE_KEY_SIZE);
	logd(("Clear Session Key text: %s", glPosParams.payAttitudeSessionKey));
	
	SavePosParams();

	resetCommCfg();
	//CommDial(DM_PREDIAL);

	return 0;
}

int payAttitudeMenu() {

	Prompt prompt = { 0 };
	char amount[12 + 1] = { 0 };
	char phoneNumber[20] = { 0 };


	if (!glPosParams.isPayAttitudeEnabled) {
		showErrorDialog("Function not enabled", 10);
		return -1;
	}

	if (!glPosParams.ucIsPrepped) {
		showErrorDialog("Terminal not prepped", 10);
		return -1;
	}

	SetCurrTitle(getTransactionTitle(PAYATTITUDE));

	adjustPayAttitudeComms();
	CommDial(DM_PREDIAL);

	while (true) {

		if (glPosParams.tranRecordCount >= MAX_TRANLOG) {
			DispErrMsg("Memory Full", "Run Close Batch!!!", USER_OPER_TIMEOUT, DERR_BEEP);
			break;
		}
		
		getDefaultAmountPrompt(&prompt, "Amount");
		if (showPrompt(&prompt) != 0) {
			break;
		}
		strmcpy(amount, prompt.value, sizeof(amount));

		getNumberPrompt(&prompt, "Phone Number", NULL);
		strcpy(prompt.hint, "Enter Phone Number");

#ifdef APP_DEBUG
		strcpy(prompt.value, "08024200989"/*"07030087643"*/);
#endif
		prompt.minLength = 11;
		prompt.maxLength = 11;
		prompt.shouldConfirm = true;

		if (showPrompt(&prompt) != 0) {
			continue;
		}

		memset(phoneNumber, 0, sizeof(phoneNumber));
		strmcpy(phoneNumber, prompt.value, sizeof(phoneNumber));
		logd(("Phone number: %s", phoneNumber));
		int result = processPayAttitudeTransaction(phoneNumber, amount);
		if (result != 0) {
			continue;
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

		statusReceiptAndNotification();

	}

	resetCommCfg();
}

void adjustPayAttitudeComms() {
	COMM_CONFIG config = glPosParams.commConfig;

	logTrace("PayAttitude IP: %s, PORT: %s", glPosParams.payAttitudeIp.szIP, glPosParams.payAttitudeIp.szPort);

	config.stWirlessPara.stHost1 = glPosParams.payAttitudeIp;
	config.stWirlessPara.stHost2 = glPosParams.payAttitudeIp;

	config.stWifiPara.stHost1 = glPosParams.payAttitudeIp;
	config.stWifiPara.stHost2 = glPosParams.payAttitudeIp;
	config.ucPortMode = glPosParams.payAttitudeProtocolFlag;

	glCommCfg = config;
	CommSetCfgParam(&config);
}

static void displayReceive() {
	Gui_ClearScr();
	Gui_DrawText("Authorising", gl_stCenterAttr, 0, 30);
	Gui_DrawText("Please check phone", gl_stCenterAttr, 0, 50);
}

static int sendPayAttitudeRequest(char* dataIn, int inlen, char* dataOut, int* outlen) {
	int ret = -1;
	adjustPayAttitudeComms();
	receiveDisplay = displayReceive;

	ret = sendSocketRequest(dataIn, inlen, dataOut, outlen);

	receiveDisplay = NULL;
	return ret;
}

int processPayAttitudeTransaction(char* phoneNumber, char* amount) {

	DL_UINT8 packBuf[0x1000] = "\0", responseBuf[0x2000] = "\0";
	DL_UINT16 packedSize = 0, responseSize = 0;
	
	if (!checkTerminalPrepStatus())
	{
		showErrorDialog("Terminal not prepped", 10);
		return APP_CANCEL;
	}

	logd(("Phone number: %s", phoneNumber));
	logd(("Amount: %s", amount));

	InitTransInfo();
	glProcInfo.stTranLog.ucTranType = PAYATTITUDE;
	glProcInfo.stTranLog.uiEntryMode = MODE_CHIP_INPUT;

	strmcpy(glProcInfo.stTranLog.szAmount, amount, sizeof(glProcInfo.stTranLog.szAmount));
	snprintf(glProcInfo.stTranLog.szPan, sizeof(glProcInfo.stTranLog.szPan), "950100%s", phoneNumber+1);
	strcat(glProcInfo.stTranLog.szExpDate, "3012");
	strcpy(glProcInfo.stTranLog.szHolderName, phoneNumber);
	snprintf(glProcInfo.szTrack2, sizeof(glProcInfo.szTrack2), "%sD%s", glProcInfo.stTranLog.szPan, glProcInfo.stTranLog.szExpDate);
	snprintf(glProcInfo.szPaymentInformation, sizeof(glProcInfo.szPaymentInformation), "00698MP0101333%s", phoneNumber);

	setFields();

	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf + 2, &packedSize);

	//apply hex bitmap fix for nibss extended messaging
	packBuf[6] = 'F';
	packBuf[7] = '2';

	PubTrimStr(packBuf + 2);

	char hash[64 + 1] = "\0";
	calculateSHA256Digest(packBuf + 2, glPosParams.payAttitudeSessionKey, hash);

	(void)DL_ISO8583_MSG_SetField_Str(SECONDARY_MESSAGE_HASH_VALUE_128, hash, &isoMsg);
	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);

	memset(packBuf, lengthOf(packBuf), sizeof(char));
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf + 2, &packedSize);

	packBuf[6] = 'F';
	packBuf[7] = '2';
	logTrace("Packed data: %s, length: %d", packBuf + 2, packedSize);

	DL_ISO8583_MSG_Free(&isoMsg);

	packBuf[0] = packedSize >> 8;
	packBuf[1] = packedSize % 256;
	packedSize += 2;

	int ret = sendPayAttitudeRequest(packBuf, packedSize, responseBuf, &responseSize);
	if (ret != 0) {
		showErrorDialog("COMM ERROR", 30);
		return ret;
	}
	logTrace("Response data: %s, length: %d", responseBuf + 2, responseSize);

	DL_ISO8583_MSG_Init(NULL, 0, &isoMsg);

	/* unpack ISO message */
	if (DL_ISO8583_MSG_Unpack(&isoHandler, responseBuf + 2, responseSize - 2, &isoMsg) != 0 || !isoMsg.field[39].ptr) {//account for 2 byte header
		DL_ISO8583_MSG_Free(&isoMsg);

		showErrorDialog("Invalid response received", DEFAULT_PASSWORD_TIMEOUT);

		memcpy(glProcInfo.stTranLog.szRspCode, INVALID_RESPONSE_CODE, lengthOf(glProcInfo.stTranLog.szRspCode) - 1);
		memcpy(glProcInfo.stTranLog.szResponseReason, responseCodeToString(INVALID_RESPONSE_CODE), lengthOf(glProcInfo.stTranLog.szResponseReason) - 1);


		return APP_SUCC; //Comms completed successfully, issue was the data received
	}

	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);

	DL_ISO8583_MSG_FIELD field39 = isoMsg.field[39];
	memcpy(glProcInfo.stTranLog.szRspCode, field39.ptr, lengthOf(glProcInfo.stTranLog.szRspCode) - 1);
	memcpy(glProcInfo.stTranLog.szResponseReason, responseCodeToString(field39.ptr), lengthOf(glProcInfo.stTranLog.szResponseReason) - 1);
	DL_ISO8583_MSG_FIELD field38 = isoMsg.field[AUTHORIZATION_CODE_38];
	if (field38.ptr) {
		memcpy(glProcInfo.stTranLog.szAuthCode, field38.ptr, lengthOf(glProcInfo.stTranLog.szAuthCode) - 1);
	}

	DL_ISO8583_MSG_Free(&isoMsg);

	return APP_SUCC;
}


int setFields() {
	DL_ISO8583_DEFS_1987_Nibss_GetHandler(&isoHandler);
	DL_ISO8583_MSG_Init(NULL, 0, &isoMsg);

	DL_ISO8583_MSG_SetField_Str(MESSAGE_TYPE_INDICATOR_0, getMTI(glProcInfo.stTranLog.ucTranType), &isoMsg);

	DL_ISO8583_MSG_SetField_Str(PRIMARY_ACCOUNT_NUMBER_2, glProcInfo.stTranLog.szPan, &isoMsg);


	char processingCode[6 + 1] = "\0";
	logTrace("Tran Type: %d", glProcInfo.stTranLog.ucTranType);
	getProcessingCode(glProcInfo.stTranLog.ucTranType, glProcInfo.stTranLog.ucAccountType, processingCode);

		DL_ISO8583_MSG_SetField_Str(PROCESSING_CODE_3, processingCode, &isoMsg);

	char totalAmount[12 + 1] = "\0";
	sprintf(totalAmount, "%012ld",
		(atol(glProcInfo.stTranLog.szAmount) + atol(glProcInfo.stTranLog.szOtherAmount)));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(TRANSACTION_AMOUNT_4, totalAmount, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(TRANSACTION_DATE_TIME_7,
			glProcInfo.stTranLog.szDateTime + 4, &isoMsg));

	char stan[6 + 1] = "\0";
	sprintf(stan, "%06ld", glProcInfo.stTranLog.ulSTAN);
	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(SYSTEM_TRACE_AUDIT_NUMBER_11,
			stan, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(LOCAL_TIME_OF_TRANSACTION_12,
			glProcInfo.stTranLog.szDateTime + 8, &isoMsg));

	char date[4 + 1] = "\0";
	strmcpy(date, glProcInfo.stTranLog.szDateTime + 4, lengthOf(date));
	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(LOCAL_DATE_OF_TRANSACTION_13,
			date, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(EXPIRATION_DATE_14,
			glProcInfo.stTranLog.szExpDate, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(MERCHANT_TYPE_18,
			glPosParams.nibssParams.merchantCategoryCode, &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(POS_ENTRY_MODE_22,"051", &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(CARD_SEQUENCE_NUMBER_23, "000", &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(POS_CONDITION_CODE_25,
			"00", &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(POS_PIN_CAPTURE_CODE_26,
			"06", &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(AMOUNT_TRANSACTION_FEE_28, "C00000000", &isoMsg));


	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(
			ACQUIRING_INSTITUTION_IDENTIFICATION_CODE_32,
			"111129",
			&isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(TRACK2_DATA_35, glProcInfo.szTrack2, &isoMsg));

	if (strlen(glProcInfo.stTranLog.szRRN) != 12) {
		getRRN(glProcInfo.stTranLog.szRRN);
	}

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(RETRIVAL_REFERENCE_NUMBER_37,
			glProcInfo.stTranLog.szRRN, &isoMsg));


	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(SERVICE_RESTRICTION_CODE_40,
			"221", &isoMsg));

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

	if (glProcInfo.stTranLog.szEchoField59 && strlen(glProcInfo.stTranLog.szEchoField59) > 0) {
		ASSERT_RETURNCODE(
			DL_ISO8583_MSG_SetField_Str(ECHO_DATA_59,
				glProcInfo.stTranLog.szEchoField59, &isoMsg));
	}

	ASSERT_RETURNCODE(DL_ISO8583_MSG_SetField_Str(PRIVATE_FIELD_MANAGEMENT_DATA_1_62, glProcInfo.szPaymentInformation, &isoMsg));

	ASSERT_RETURNCODE(
			DL_ISO8583_MSG_SetField_Str(POS_DATA_CODE_123, "510101513344101", &isoMsg));

	ASSERT_RETURNCODE(
		DL_ISO8583_MSG_SetField_Str(SECONDARY_MESSAGE_HASH_VALUE_128, "\0", &isoMsg));

	return APP_SUCC;
}