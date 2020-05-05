#include "global.h"
#include "xIsoDef.h"
#include "xTime.h"
#include "dl_iso8583.h"
#include "dl_iso8583_defs_1987.h"
#include "dl_output.h"
#include "xui.h"
#include "converters.h"


static int getKeyProcessingCode(int keyType, char* processingCode) {

	switch (keyType) {
	case TYPE_MASTER_KEY:
		sprintf(processingCode, "%s0000", TERMINAL_MASTER_KEY_ISO_CODE);
		break;
	case TYPE_SESSION_KEY:
		sprintf(processingCode, "%s0000", TERMINAL_SESSION_KEY_ISO_CODE);
		break;
	case TYPE_PIN_KEY:
		sprintf(processingCode, "%s0000", TERMINAL_PIN_KEY_ISO_CODE);
		break;
	default:
		return APP_FAIL;
	}

	return APP_SUCC;
}


int updateTerminalConfig(NibssTerminalParameter *parameter) {
	//	set POS params
	char temp[20] = "\0";

	strmcpy(glPosParams.merchantId, parameter->cardAcceptiorIdentificationCode, lengthOf(glPosParams.merchantId));
	//strmcpy(glPosParams.merchantName, parameter->merchantNameAndLocation, lengthOf(glPosParams.merchantName));
	glPosParams.requestTimeOutSec = atoi(parameter->timeout);
	glPosParams.callHomeTimeMinutes = atoi(parameter->callHomeTime) * 60;
	

	memset(temp, '0', lengthOf(temp));
	PubAsc2Bcd(parameter->ctmsDateAndTime + 2, strlen(parameter->ctmsDateAndTime + 2), temp);
	SetTime(temp);

	EMVGetParameter(&glEmvParam);
	PubAsc2Bcd(parameter->currencyCode, strlen(parameter->currencyCode), glEmvParam.TransCurrCode);
	PubAsc2Bcd(parameter->countryCode, strlen(parameter->countryCode), glEmvParam.CountryCode);

	memset(temp, '0', lengthOf(temp));
	char paddStr[5] = "\0";
	sprintf(paddStr, "0%s", parameter->countryCode);
	PubAsc2Bcd(paddStr, strlen(paddStr), glEmvParam.CountryCode);
	logHexString("Set country code: ", glEmvParam.CountryCode, 2);

	memset(temp, '0', sizeof(temp));
	memset(paddStr, '\0', sizeof(paddStr));
	sprintf(paddStr, "0%s", parameter->currencyCode);
	PubAsc2Bcd(paddStr, strlen(paddStr), glEmvParam.TransCurrCode);
	logHexString("Set currency code: ", glEmvParam.TransCurrCode, 2);

	memset(temp, '\0', sizeof(temp));
	PubAsc2Bcd(parameter->merchantCategoryCode, strlen(parameter->merchantCategoryCode), glEmvParam.MerchCateCode);
	logHexString("Set merchant category code: ", glEmvParam.MerchCateCode, 2);

	EMVSetParameter(&glEmvParam);
	
	return 0;
}

static int buildNibssKeyRequestNew(int nibssKeyType, char* downloadedKey) {

	DL_ISO8583_HANDLER isoHandler;
	DL_ISO8583_MSG     isoMsg;
	DL_UINT8           packBuf[1000] = "\0", responseBuf[LEN_MAX_COMM_DATA] = "\0";
	DL_UINT16          packedSize = 0, responseSize = 0;



	if (strlen(glPosParams.terminalId) != 8) {
		DispErrMsg("Terminal ID not set", NULL, DEFAULT_PASSWORD_TIMEOUT, DERR_BEEP);
		return APP_FAIL;
	}

	IsoTime isoTime;
	getIsoTime(&isoTime);

	char processingCode[6 + 1] = "\0";
	getKeyProcessingCode(nibssKeyType, processingCode);


	/* get ISO-8583 1987 handler */
	DL_ISO8583_DEFS_1987_Nibss_GetHandler(&isoHandler);

	/* initialise ISO message */
	DL_ISO8583_MSG_Init(NULL, 0, &isoMsg);

	/* set ISO message fields */
	(void)DL_ISO8583_MSG_SetField_Str(MESSAGE_TYPE_INDICATOR_0, NETWORK_MGT_REQUEST_MTI, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(PROCESSING_CODE_3, processingCode, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(TRANSACTION_DATE_TIME_7, isoTime.longDate_F7, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(SYSTEM_TRACE_AUDIT_NUMBER_11, isoTime.time_F12, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(LOCAL_TIME_OF_TRANSACTION_12, isoTime.time_F12, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(LOCAL_DATE_OF_TRANSACTION_13, isoTime.shortDate_F13, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(CARD_ACCEPTOR_TERMINAL_IDENTIFICATION_41, glPosParams.terminalId, &isoMsg);

	/* output ISO message content */
	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);

	/* pack ISO message */
	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf + 2, &packedSize);
	logTrace("Packed data: %s, length: %d", packBuf + 2, packedSize);

	/* free ISO message */
	DL_ISO8583_MSG_Free(&isoMsg);

	packBuf[0] = packedSize >> 8;
	packBuf[1] = packedSize % 256;
	packedSize += 2;

	ASSERT_RETURNCODE(sendSocketRequest(packBuf, packedSize, responseBuf, &responseSize))
	logTrace("Response data: %s, length: %d", responseBuf + 2, responseSize);

	DL_ISO8583_MSG_Init(NULL, 0, &isoMsg);

	/* unpack ISO message */
	int res = DL_ISO8583_MSG_Unpack(&isoHandler, responseBuf + 2, responseSize - 2, &isoMsg); //account for 2 byte header
	logTrace("Unpack response: %d", res);

	if (res != 0) {
		DL_ISO8583_MSG_Free(&isoMsg);
		DispErrMsg("Key Download Error", "Invalid Response", DEFAULT_PASSWORD_TIMEOUT, DERR_BEEP);
		return APP_FAIL;
	}

	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);

	DL_ISO8583_MSG_FIELD field39 = isoMsg.field[39];
	DL_ISO8583_MSG_FIELD field53 = isoMsg.field[53];

	if (!field39.ptr || !field53.ptr) {
		DL_ISO8583_MSG_Free(&isoMsg);
		DispErrMsg("Key Download Error", "Empty security info", DEFAULT_PASSWORD_TIMEOUT, DERR_BEEP);
		return APP_FAIL;
	}

	int ret = APP_FAIL;

	if (!isSuccessResponse(field39.ptr)) {
		DispErrMsg("Key Download Error", responseCodeToString(field39.ptr), DEFAULT_PASSWORD_TIMEOUT, DERR_BEEP);
	}
	else {
		if (field53.ptr) {
			memcpy(downloadedKey, field53.ptr, ASCII_KEY_SIZE + ASCII_KCV_SIZE);
			ret = APP_SUCC;
		}
	}


	DL_ISO8583_MSG_Free(&isoMsg);

	return ret;
}

static int processKeys(char masterKeyData[ASCII_KEY_SIZE + ASCII_KCV_SIZE + 1],
	char sessionKeyData[ASCII_KEY_SIZE + ASCII_KCV_SIZE + 1], char pinKeyData[ASCII_KEY_SIZE + ASCII_KCV_SIZE + 1]) {

	int ret = -1;
	PedErase();

	uchar masterKey[BYTE_KEY_SIZE] = { 0 };
	uchar masterKeyKCV[BYTE_KCV_SIZE] = { 0 };

	uchar sessionKey[BYTE_KEY_SIZE] = { 0 };
	uchar sessionKeyKCV[BYTE_KCV_SIZE] = { 0 };

	uchar zmk[BYTE_KEY_SIZE] = { 0 };
	uchar clrMK[BYTE_KEY_SIZE] = { 0 };
	uchar clrSK[BYTE_KEY_SIZE] = { 0 };


	PubAsc2Bcd(masterKeyData, ASCII_KEY_SIZE, masterKey);
	PubAsc2Bcd(masterKeyData + ASCII_KEY_SIZE, ASCII_KCV_SIZE, masterKeyKCV);

	PubAsc2Bcd(glPosParams.hostZMK, ASCII_KEY_SIZE, zmk);
	des3EcbDecrypt(zmk, masterKey, BYTE_KEY_SIZE, clrMK);

	logHexString("ZMK ", zmk, BYTE_KEY_SIZE);
	logHexString("ENC MK ", masterKey, BYTE_KEY_SIZE);
	logHexString("CLEAR MK ", clrMK, BYTE_KEY_SIZE);


	ST_KEY_INFO     keyInfo = { 0 };
	ST_KCV_INFO		kcvInfo = { 0 };

	keyInfo.ucSrcKeyType = PED_TMK;
	keyInfo.ucSrcKeyIdx = 0;
	keyInfo.ucDstKeyType = PED_TMK;
	keyInfo.ucDstKeyIdx = MASTER_KEY_ID;

	memcpy(keyInfo.aucDstKeyValue, clrMK, BYTE_KEY_SIZE);
	keyInfo.iDstKeyLen = BYTE_KEY_SIZE;

	PubAsc2Bcd(masterKeyData + ASCII_KEY_SIZE, ASCII_KCV_SIZE, kcvInfo.aucCheckBuf);
	kcvInfo.iCheckMode = 0;

	ret = PedWriteKey(&keyInfo, &kcvInfo);
	if (ret != 0) {
		logd(("Load Masterkey::ret = %d", ret));
		return ret;
	}


	PubAsc2Bcd(sessionKeyData, ASCII_KEY_SIZE, sessionKey);
	PubAsc2Bcd(sessionKeyData + ASCII_KEY_SIZE, ASCII_KCV_SIZE, sessionKeyKCV);
	
	des3EcbDecrypt(clrMK, sessionKey, BYTE_KEY_SIZE, clrSK);

	memset(glPosParams.hostSessionKey, 0, lengthOf(glPosParams.hostSessionKey));
	PubBcd2Asc0(clrSK, BYTE_KEY_SIZE, glPosParams.hostSessionKey);
	logHexString("ENC SK ", sessionKey, BYTE_KEY_SIZE);
	logHexString("CLEAR SK ", clrSK, BYTE_KEY_SIZE);
	logd(("Clear Session Key text: %s", glPosParams.hostSessionKey));

	memset(&keyInfo, 0, sizeof(ST_KEY_INFO));
	memset(&kcvInfo, 0, sizeof(ST_KCV_INFO));

	keyInfo.ucSrcKeyType = PED_TMK;
	keyInfo.ucSrcKeyIdx = MASTER_KEY_ID;
	keyInfo.ucDstKeyType = PED_TPK;
	keyInfo.ucDstKeyIdx = DEF_PIN_KEY_ID;

	PubAsc2Bcd(pinKeyData, ASCII_KEY_SIZE, keyInfo.aucDstKeyValue);
	keyInfo.iDstKeyLen = BYTE_KEY_SIZE;

	PubAsc2Bcd(pinKeyData + ASCII_KEY_SIZE, ASCII_KCV_SIZE, kcvInfo.aucCheckBuf);
	kcvInfo.iCheckMode = 0;

	ret = PedWriteKey(&keyInfo, &kcvInfo);
	if (ret != 0) {
		logd(("Load Pinkey::ret = %d", ret));
		return ret;
	}

	SavePosParams();
	return ret;
}


static int downloadNibssKeys() {
	char masterKeyData[ASCII_KEY_SIZE + ASCII_KCV_SIZE + 1] = "\0";
	char sessionKeyData[ASCII_KEY_SIZE + ASCII_KCV_SIZE + 1] = "\0";
	char pinKeyData[ASCII_KEY_SIZE + ASCII_KCV_SIZE + 1] = "\0";

	
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

	if (0 != buildNibssKeyRequestNew(TYPE_PIN_KEY, pinKeyData)) {
		return -1;
	}
	logTrace("Pinkey: %s: ", pinKeyData);
	showInfo(NULL, 1, 1, "PIN KEY Ok");

	return processKeys(masterKeyData, sessionKeyData, pinKeyData);
}



static int processNetworkManagementRequest(int requestType, char* responseMgtData) {

	DL_ISO8583_HANDLER isoHandler;
	DL_ISO8583_MSG     isoMsg;
	DL_UINT8           packBuf[1000] = "\0", responseBuf[0x2000] = "\0";
	DL_UINT16          packedSize = 0, responseSize = 0;

	if (strlen(glPosParams.hostSessionKey) != ASCII_KEY_SIZE) {
		DispErrMsg("Terminal not prepped", "Please download keys first", 10,  DERR_BEEP);
		return APP_QUIT;
	}

	if (strlen(glPosParams.terminalId) != 8) {
		showErrorDialog("Terminal ID not set", DEFAULT_PASSWORD_TIMEOUT);
		return APP_FAIL;
	}

	char processingCode[6 + 1] = "\0";
	ASSERT_RETURNCODE(getProcessingCode(requestType, DEFAULT, processingCode));

	char field62String[100] = "\0";

	char temp[32+1] = "\0";
	ReadSN(temp);
	logTrace("Terminal Serial: %s", temp);

	sprintf(field62String, "01%03d%s", strlen(temp), temp);

	if (requestType == CALL_HOME) {
		char* model = GetTerminalModel();

		sprintf(field62String + strlen(field62String), "09%03d%s10%03d%s", strlen(temp), temp,
			strlen(EDC_VER_PUB), EDC_VER_PUB, strlen(model), model);
	}

	logTrace("Field 62: %s", field62String);


	IsoTime isoTime;
	getIsoTime(&isoTime);

	DL_ISO8583_DEFS_1987_Nibss_GetHandler(&isoHandler);
	DL_ISO8583_MSG_Init(NULL, 0, &isoMsg);

	(void)DL_ISO8583_MSG_SetField_Str(MESSAGE_TYPE_INDICATOR_0, NETWORK_MGT_REQUEST_MTI, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(PROCESSING_CODE_3, processingCode, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(TRANSACTION_DATE_TIME_7, isoTime.longDate_F7, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(SYSTEM_TRACE_AUDIT_NUMBER_11, isoTime.time_F12, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(LOCAL_TIME_OF_TRANSACTION_12, isoTime.time_F12, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(LOCAL_DATE_OF_TRANSACTION_13, isoTime.shortDate_F13, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(CARD_ACCEPTOR_TERMINAL_IDENTIFICATION_41, glPosParams.terminalId, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(PRIVATE_FIELD_MANAGEMENT_DATA_1_62, field62String, &isoMsg);
	(void)DL_ISO8583_MSG_SetField_Str(PRIMARY_MESSAGE_HASH_VALUE_64, "\0", &isoMsg);

	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);

	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf + 2, &packedSize);
	logTrace("First Packed data: %s, length: %d", packBuf + 2, packedSize);

	PubTrimStr(packBuf + 2);
	logTrace("Trimmed data: %s, length: %d", packBuf + 2, strlen(packBuf + 2));

	char hash[64 + 1] = "\0";
	calculateSHA256Digest(packBuf + 2, glPosParams.hostSessionKey, hash);

	(void)DL_ISO8583_MSG_SetField_Str(PRIMARY_MESSAGE_HASH_VALUE_64, hash, &isoMsg);

	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);

	memset(packBuf, lengthOf(packBuf), sizeof(char));

	(void)DL_ISO8583_MSG_Pack(&isoHandler, &isoMsg, packBuf + 2, &packedSize);
	logTrace("Packed data: %s, length: %d", packBuf + 2, packedSize);

	DL_ISO8583_MSG_Free(&isoMsg);

	packBuf[0] = packedSize >> 8;
	packBuf[1] = packedSize % 256;
	packedSize += 2;

	ASSERT_RETURNCODE(sendSocketRequest(packBuf, packedSize, responseBuf, &responseSize))
		logTrace("Response data: %s, length: %d", responseBuf + 2, responseSize);

	DL_ISO8583_MSG_Init(NULL, 0, &isoMsg);

	/* unpack ISO message */
	int res = DL_ISO8583_MSG_Unpack(&isoHandler, responseBuf + 2, responseSize - 2, &isoMsg); //account for 2 byte header
	logTrace("Unpack response: %d", res);

	if (res != 0 || !isoMsg.field[39].ptr) {
		DL_ISO8583_MSG_Free(&isoMsg);
		//		showErrorDialog("Invalid Response", DEFAULT_PASSWORD_TIMEOUT);
		return APP_FAIL;
	}

	DL_ISO8583_MSG_Dump(NULL, NULL, &isoHandler, &isoMsg);

	DL_ISO8583_MSG_FIELD field39 = isoMsg.field[39];
	if (!isSuccessResponse(field39.ptr)) {
		if (!IS_NULL_EMPTY(field39.ptr)) {
			showErrorDialog(responseCodeToString(field39.ptr), 10);
		}

		DL_ISO8583_MSG_Free(&isoMsg);
		return APP_FAIL;
	}

	CLEAR(responseMgtData, '\0');

	if (requestType == TERMINAL_PARAMETER_DOWNLOAD) {
		DL_ISO8583_MSG_FIELD field62 = isoMsg.field[62];
		if (field62.ptr) {
			strcat(responseMgtData, field62.ptr);
		}
	}

	if (requestType == CA_PUBLIC_KEY_DOWNLOAD
		|| requestType == EMV_APPLICATION_AID_DOWNLOAD) {
		DL_ISO8583_MSG_FIELD field63 = isoMsg.field[63];

		if (field63.ptr) {
			logTrace("Field 63: %s", field63.ptr);
			logHexString("Field 63 Hex: ", field63.ptr, 512);
			strcat(responseMgtData, field63.ptr);
		}
	}

	DL_ISO8583_MSG_Free(&isoMsg);

	return APP_SUCC;
}



int downloadNibssTerminalParameters() {
	char temp[0x1000];
	
	ASSERT_RETURNCODE(processNetworkManagementRequest(TERMINAL_PARAMETER_DOWNLOAD, temp));

	if (!expandField62(&glPosParams.nibssParams, temp)) {
		return APP_QUIT;
	}

	if (updateTerminalConfig(&glPosParams.nibssParams) != 0) {
		logTrace("terminal config update failed");
		return -1;
	}

	glPosParams.ucIsPrepped = TRUE;

	SavePosParams();
}

int doNibssCallHome() {
	char temp[0x1000];

	logTrace("Starting call home");
	showNonModalDialog("CALL HOME", "Calling Home...\nPlease wait");
	CommDial(DM_PREDIAL);
	ASSERT_RETURNCODE(processNetworkManagementRequest(CALL_HOME, temp));
	showInfo("CALL HOME", 1, 1, "CALL HOME\nOk");

	logTrace("Call home successful");
	logTrace("Call home field62: %s", temp);

	return APP_SUCC;
}

int doNibssEod() {
	char temp[0x1000];

	logTrace("Starting E0D");
	showNonModalDialog("EOD", "Printing Report...\nPlease wait");
	CommDial(DM_PREDIAL);
	ASSERT_RETURNCODE(processNetworkManagementRequest(EOD, temp));
	logTrace("EOD successful");
	logTrace("EOD field62: %s", temp);


	return APP_SUCC;
}





void prepTerminal(void) {
	SetCurrTitle("Prep Terminal");

	DispMessage("Downloading Keys");
	CommDial(DM_PREDIAL);
	int ret = downloadNibssKeys();
	if (ret != 0) {
		DispErrMsg("Key download Failed", NULL, 10, DERR_BEEP);
		return;
	}

	DispMessage("Downloading Parameters");
	ret = downloadNibssTerminalParameters();
	if (ret != 0) {
		DispErrMsg("Parameter download Failed", NULL, 10, DERR_BEEP);
		return;
	}

	PubBeepOk();
	showMessageDialog(GetCurrTitle(), "Prep OK", 1, 10);

}