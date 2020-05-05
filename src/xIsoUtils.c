/*
* xIsoUtils.c
*
*  Created on: Oct 8, 2018
*      Author: ayodeji.bamitale
*/

#include "xIsoDef.h"
#include "xDefs.h"
#include "Logger.h"

#define CTMS_DATE_TIME_LEN 14
#define CARD_ACCEPTOR_ID_CODE_LEN 15
#define MERCHANT_CATEGORY_LEN 4


/**
*
* @param isoRequestType [in]
* @param accountType [in]
* @param processingCode [out]
* @return
*/
int getProcessingCode(int isoRequestType, AccountType accountType, char* processingCode) {
	const char* isoAccountType = getIsoAccountType(accountType);

	char* isoCode;
	switch (isoRequestType) {
	case PURCHASE:
	case SPIDER_WEB:
	case PAYXPRESS:
	case COLLECTIONS:
	case PHED:
	case GATEWAY:
		isoCode = PURCHASE_ISO_CODE;
		break;
	case PURCHASE_WITH_CASH_BACK:
		isoCode = PURCHASE_WITH_CASH_BACK_ISO_CODE;
		break;
	case PURCHASE_WITH_ADDITIONAL_DATA:
		isoCode = PURCHASE_WITH_ADDITIONAL_DATA_ISO_CODE;
		break;
	case CASH_ADVANCE:
		isoCode = CASH_ADVANCE_ISO_CODE;
		break;
	case POS_PRE_AUTHORIZATION:
		isoCode = PRE_AUTHORIZATION_ISO_CODE;
		break;
	case POS_PRE_AUTH_COMPLETION:
		isoCode = PRE_AUTHORIZATION_COMPLETION_ISO_CODE;
		break;
	case BALANCE:
		isoCode = BALANCE_INQUIRY_ISO_CODE;
		break;
	case MINI_STATEMENT:
		isoCode = MINI_STATEMENT_ISO_CODE;
		break;
	case LINK_ACCOUNT_INQUIRY:
		isoCode = LINK_ACCOUNT_INQUIRY_ISO_CODE;
		break;
	case REFUND:
		isoCode = REFUND_ISO_CODE;
		break;
	case REVERSAL:
		isoCode = REVERSAL_ISO_CODE;
		break;
	case TERMINAL_MASTER_KEY:
		isoCode = TERMINAL_MASTER_KEY_ISO_CODE;
		break;
	case TERMINAL_SESSION_KEY:
		isoCode = TERMINAL_SESSION_KEY_ISO_CODE;
		break;
	case TERMINAL_PIN_KEY:
		isoCode = TERMINAL_PIN_KEY_ISO_CODE;
		break;
	case TERMINAL_PARAMETER_DOWNLOAD:
		isoCode = TERMINAL_PARAMETER_DOWNLOAD_ISO_CODE;
		break;
	case CALL_HOME:
		isoCode = CALL_HOME_ISO_CODE;
		break;
	case EOD:
		isoCode = DAILY_TRANSACTION_REPORT_DOWNLOAD;
		break;
	case CA_PUBLIC_KEY_DOWNLOAD:
		isoCode = CA_PUBLIC_KEY_DOWNLOAD_ISO_CODE;
		break;
	case EMV_APPLICATION_AID_DOWNLOAD:
		isoCode = EMV_APPLICATION_AID_DOWNLOAD_ISO_CODE;
		break;
	case TRANZAXIS_WORKING_KEY_INQUIRY:
		isoCode = NEW_WORKING_KEY_INQUIRY_FROM_HOST;
		break;
	case TRANZAXIS_TRAFFIC_ENCRYPTION_WORKING_KEY:
		isoCode = NEW_WORKING_KEY_FOR_TRAFFIC_ENCRYPTION_INQUIRY;
		break;
	case TRANZAXIS_ECHO_TEST:
		isoCode = ECHO_TEST_CODE;
		break;
	default:
		return APP_FAIL;
	}

	sprintf(processingCode, "%s%s00", isoCode, isoAccountType);

	return APP_SUCC;
}


/**
*
* @param inputString [in]
* @param additionalAmt [out]
* @return APP_SUCC, APP_FAIL
*/
int getSingleAdditionalAmount(const char* inputString, AdditionalAmount* additionalAmt) {

	CLEAR(additionalAmt, 0);

	strncpy(additionalAmt->accountType, inputString, 2);

	inputString += 2;
	strncpy(additionalAmt->amountType, inputString, 2);

	inputString += 2;
	strncpy(additionalAmt->currencyCode, inputString, 3);

	inputString += 3;
	additionalAmt->amountSign = inputString[0];

	inputString++;
	strncpy(additionalAmt->amount, inputString, 12);

	return APP_SUCC;
}


/**
*
* @param inputString [in]
* @param buffer 	[out]
* @param bufferlen [in]
* @return actual count parsed, APP_FAIL
*/
int parseField54AdditionalAmount(const char* inputString, AdditionalAmount* buffer, int bufferlen) {
	int count = 1;
	if (inputString == NULL || strlen(inputString) < 20 || bufferlen <= 0) {
		return APP_FAIL;
	}

	count = strlen(inputString) / 20;

	int i = 0;

	while (i < count && i < bufferlen) {
		getSingleAdditionalAmount(inputString, &buffer[i]);
		logAdditionalAmount(&buffer[i]);
		inputString += 20; //move to next
		i++;
	}

	return i;
}

void logAdditionalAmount(AdditionalAmount * addAmount) {
	logTrace("========================Additional Amount======================\n");
	logTrace("Account Type:  %s\n", addAmount->accountType);
	logTrace("Amount Type:   %s\n", addAmount->amountType);
	logTrace("Currency Code: %s\n", addAmount->currencyCode);
	logTrace("Amount Sign:   %c\n", addAmount->amountSign);
	logTrace("Amount:        %s\n", addAmount->amount);
	logTrace("================================================================\n");
}


/**
*
* @param accountType [in]
* @return ISO ACCOUNT TYPE CODE
*/
const char* getIsoAccountType(AccountType accountType) {
	switch (accountType) {
	case SAVINGS: return ISO_ACCOUNT_TYPE_SAVINGS;
	case 	CURRENT: return ISO_ACCOUNT_TYPE_CURRENT;
	case CREDIT: return ISO_ACCOUNT_TYPE_CREDIT;
	case UNIVERSAL: return ISO_ACCOUNT_TYPE_UNIVERSAL;
	case INVESTMENT: return ISO_ACCOUNT_TYPE_INVESTMENT;
	case BONUS: return ISO_ACCOUNT_TYPE_BONUS;
	default: return ISO_ACCOUNT_TYPE_DEFAULT;
	}
}



static char* responseCodeToStringL(const char *responseCode)
{

	if (strcmp(responseCode, "00") == 0)   return "Approved or Completed Successfully";
	if (strcmp(responseCode, "01") == 0)   return "Refer to card issuer";
	if (strcmp(responseCode, "02") == 0)   return "Refer to card issuer, special condition";
	if (strcmp(responseCode, "03") == 0)   return "Invalid merchant";
	if (strcmp(responseCode, "04") == 0)   return "Pick-up card";
	if (strcmp(responseCode, "05") == 0)   return "Do not honor";
	if (strcmp(responseCode, "06") == 0)   return "Error";
	if (strcmp(responseCode, "07") == 0)   return "Pick-up card, special condition";
	if (strcmp(responseCode, "08") == 0)   return "Honor with identification";
	if (strcmp(responseCode, "09") == 0)   return "Request in progress";
	if (strcmp(responseCode, "10") == 0)   return "Approved, partial";
	if (strcmp(responseCode, "11") == 0)   return "Approved, VIP";
	if (strcmp(responseCode, "12") == 0)   return "Invalid transaction";
	if (strcmp(responseCode, "13") == 0)   return "Invalid amount";
	if (strcmp(responseCode, "14") == 0)   return "Invalid card number";
	if (strcmp(responseCode, "15") == 0)   return "No such issuer";
	if (strcmp(responseCode, "16") == 0)   return "Approved, update track 3";
	if (strcmp(responseCode, "17") == 0)   return "Customer cancellation";
	if (strcmp(responseCode, "18") == 0)   return "Customer dispute";
	if (strcmp(responseCode, "19") == 0)   return "Re-enter transaction";
	if (strcmp(responseCode, "20") == 0)   return "Invalid response";
	if (strcmp(responseCode, "21") == 0)   return "No action taken";
	if (strcmp(responseCode, "22") == 0)   return "Suspected malfunction";
	if (strcmp(responseCode, "23") == 0)   return "Unacceptable transaction fee";
	if (strcmp(responseCode, "24") == 0)   return "File update not supported";
	if (strcmp(responseCode, "25") == 0)   return "Unable to locate record";
	if (strcmp(responseCode, "26") == 0)   return "Duplicate record";
	if (strcmp(responseCode, "27") == 0)   return "File update edit error";
	if (strcmp(responseCode, "28") == 0)   return "File update file locked";
	if (strcmp(responseCode, "29") == 0)   return "File update failed";
	if (strcmp(responseCode, "30") == 0)   return "Format error";
	if (strcmp(responseCode, "31") == 0)   return "Bank not supported";
	if (strcmp(responseCode, "32") == 0)   return "Completed partially";
	if (strcmp(responseCode, "33") == 0)   return "Expired card, pick-up";
	if (strcmp(responseCode, "34") == 0)   return "Suspected fraud, pick-up";
	if (strcmp(responseCode, "35") == 0)   return "Contact acquirer, pick-up";
	if (strcmp(responseCode, "36") == 0)   return "Restricted card, pick-up";
	if (strcmp(responseCode, "37") == 0)   return "Call acquirer security, pick-up";
	if (strcmp(responseCode, "38") == 0)   return "PIN tries exceeded, pick-up";
	if (strcmp(responseCode, "39") == 0)   return "No credit account";
	if (strcmp(responseCode, "40") == 0)   return "Function not supported";
	if (strcmp(responseCode, "41") == 0)   return "Lost card";
	if (strcmp(responseCode, "42") == 0)   return "No universal account";
	if (strcmp(responseCode, "43") == 0)   return "Stolen card";
	if (strcmp(responseCode, "44") == 0)   return "No investment account";
	if (strcmp(responseCode, "51") == 0)   return "Insufficient funds";
	if (strcmp(responseCode, "52") == 0)   return "No check account";
	if (strcmp(responseCode, "53") == 0)   return "No savings account";
	if (strcmp(responseCode, "54") == 0)   return "Expired card";
	if (strcmp(responseCode, "55") == 0)   return "Incorrect PIN";
	if (strcmp(responseCode, "56") == 0)   return "No card record";
	if (strcmp(responseCode, "57") == 0)   return "Transaction not permitted to cardholder";
	if (strcmp(responseCode, "58") == 0)   return "Transaction not permitted on terminal";
	if (strcmp(responseCode, "59") == 0)   return "Suspected fraud";
	if (strcmp(responseCode, "60") == 0)   return "Contact acquirer";
	if (strcmp(responseCode, "61") == 0)   return "Exceeds withdrawal limit";
	if (strcmp(responseCode, "62") == 0)   return "Restricted card";
	if (strcmp(responseCode, "63") == 0)   return "Security violation";
	if (strcmp(responseCode, "64") == 0)   return "Original amount incorrect";
	if (strcmp(responseCode, "65") == 0)   return "Exceeds withdrawal frequency";
	if (strcmp(responseCode, "66") == 0)   return "Call acquirer security";
	if (strcmp(responseCode, "67") == 0)   return "Hard capture";
	if (strcmp(responseCode, "68") == 0)   return "Response received too late";
	if (strcmp(responseCode, "75") == 0)   return "PIN tries exceeded";
	if (strcmp(responseCode, "77") == 0)   return "Intervene, bank approval required";
	if (strcmp(responseCode, "78") == 0)   return "Intervene, bank approval required for partial amount";
	if (strcmp(responseCode, "90") == 0)   return "Cut-off in progress";
	if (strcmp(responseCode, "91") == 0)   return "Issuer or switch inoperative";
	if (strcmp(responseCode, "92") == 0)   return "Routing error";
	if (strcmp(responseCode, "93") == 0)   return "Violation of law";
	if (strcmp(responseCode, "94") == 0)   return "Duplicate transaction";
	if (strcmp(responseCode, "95") == 0)   return "Reconcile error";
	if (strcmp(responseCode, "96") == 0)   return "System malfunction";
	if (strcmp(responseCode, "98") == 0)   return "Exceeds cash limit";
	if (strcmp(responseCode, "99") == 0)   return "No response received";
	if (strcmp(responseCode, "A1") == 0)   return "Unknown";
	if (strcmp(responseCode, "Z3") == 0)   return "Unable to go online";
	return "Unknown Response Code";
}

static char temp[100] = "\0";
char* responseCodeToString(const char* responseCode) {

	if (memcmp(responseCode, "00", 2) == 0) {
		return  responseCodeToStringL(responseCode);
	}

	memset(temp, '\0', sizeof(temp));
	snprintf(temp, 100, "%s (%s)", responseCodeToStringL(responseCode), responseCode);
	return temp;
}


#ifdef APP_DEBUG
void logField62(const NibssTerminalParameter *field62)
{
	logTrace("ctmsDateAndTime => %s\n", field62->ctmsDateAndTime);
	logTrace("cardAcceptiorIdentificationCode => %s\n", field62->cardAcceptiorIdentificationCode);
	logTrace("timeout => %s\n", field62->timeout);
	logTrace("currencyCode => %s\n", field62->currencyCode);
	logTrace("countryCode => %s\n", field62->countryCode);
	logTrace("callHomeTime => %s\n", field62->callHomeTime);
	logTrace("merchantNameAndLocation => %s\n", field62->merchantNameAndLocation);
	logTrace("merchantCategoryCode => %s\n", field62->merchantCategoryCode);
}
#endif


short validField62(NibssTerminalParameter * field62)
{
#ifdef APP_DEBUG
	logField62(field62);
#endif

	if (strlen(field62->ctmsDateAndTime) != CTMS_DATE_TIME_LEN) {
		logTrace("POS PARAMETERS", "Ctms Date and time is invalid: %s", field62->ctmsDateAndTime);
		return 0;
	}

	if (strlen(field62->cardAcceptiorIdentificationCode) != CARD_ACCEPTOR_ID_CODE_LEN) {
		logTrace("POS PARAMETERS", "Card Acceptor ID Code is invalid: %s", field62->cardAcceptiorIdentificationCode);
		return 0;
	}


	if (strlen(field62->merchantCategoryCode) != MERCHANT_CATEGORY_LEN) {
		logTrace("POS PARAMETERS", "Merchant Category Code is invalid: %s", field62->merchantCategoryCode);
		return 0;
	}

	return 1;
}

static int getLength(char * line, int nCopy)
{
	int len = strlen(line);
	int ret = 0;
	char value[23] = { '\0' };

	if (len && (nCopy < len)) {
		char buffer[0x1000] = { '\0' };

		strncpy(value, line, nCopy);
		sprintf(buffer, "%s", &line[nCopy]);
		memset(line, '\0', strlen(line));
		sprintf(line, "%s", buffer);
		ret = atoi(value);
	}

	return ret;
}

static int getValue(char * line, char * value, int nCopy)
{
	int len = strlen(line);

	if (len && nCopy <= len) {
		char buffer[10000] = { '\0' };
		sprintf(buffer, "%s", &line[nCopy]);
		strncpy(value, line, nCopy);
		memset(line, '\0', strlen(line));
		sprintf(line, "%s", buffer);
		return nCopy;
	}

	return 0;
}


short expandField62(NibssTerminalParameter * field62, char* field62Str)
{
	int tagLen, valueWidth;
	//char * current = field62Str;
	char current[0x512] = { '\0' };
	memset(field62, '\0', sizeof(NibssTerminalParameter));

	sprintf(current, "%s", field62Str);

	tagLen = 2;
	valueWidth = 3;

	while (1)
	{
		char nextTag[3] = { '\0' };
		getValue(current, nextTag, tagLen);

		if (!atoi(nextTag)) break;

		if (strcmp(nextTag, "02") == 0)
		{
			getValue(current, field62->ctmsDateAndTime, getLength(current, valueWidth));
		}
		else if (strcmp(nextTag, "03") == 0)
		{
			getValue(current, field62->cardAcceptiorIdentificationCode, getLength(current, valueWidth));
		}
		else if (strcmp(nextTag, "04") == 0)
		{
			getValue(current, field62->timeout, getLength(current, valueWidth));
		}
		else if (strcmp(nextTag, "05") == 0)
		{
			getValue(current, field62->currencyCode, getLength(current, valueWidth));
		}
		else if (strcmp(nextTag, "06") == 0)
		{
			getValue(current, field62->countryCode, getLength(current, valueWidth));
		}
		else if (strcmp(nextTag, "07") == 0)
		{
			getValue(current, field62->callHomeTime, getLength(current, valueWidth));
		}
		else if (strcmp(nextTag, "08") == 0)
		{
			getValue(current, field62->merchantCategoryCode, getLength(current, valueWidth));
		}
		else if (strcmp(nextTag, "52") == 0)
		{
			getValue(current, field62->merchantNameAndLocation, getLength(current, valueWidth));
		}

	}

	if (!validField62(field62)) return 0;

	return 1;
}

/**
*
* @param reason [in]
* @param outStr [out]
*/
const char* getReversalReasonCode(int reason) {
	switch (reason) {
	case REASON_TIME_OUT:
		return MESSAGE_REASON_CODE_TIMEOUT;
	case REASON_CANCELLATION:
		return MESSAGE_REASON_CODE_CUSTOMER_CANCELLATION;
	case REASON_COMPLETED_PARTIALLY:
		return MESSAGE_REASON_CODE_COMPLETED_PARTIALLY;
	default:
		return MESSAGE_REASON_CODE_UNSPECIFIED;
	}
}


const char* getMTI(int transType) {
	switch (transType) {
	case PURCHASE:
	case PAYXPRESS:
	case SPIDER_WEB:
	case PURCHASE_CASH:
	case PURCHASE_WITH_CASH_BACK:
	case PURCHASE_WITH_ADDITIONAL_DATA:
	case CASH_ADVANCE:
	case DEPOSIT:
	case TRANSFER:
	case BILL_PAYMENT:
	case PREPAID:
	case REFUND:
	case COLLECTIONS:
	case PHED:
	case GATEWAY:
	case CASH_MOP_UP:
		return TRANSACTION_REQUEST_MTI;
	case POS_PRE_AUTH_COMPLETION:
		return TRANSACTION_ADVISE_MTI;
	case POS_PRE_AUTHORIZATION:
	case BALANCE:
	case PIN_CHANGE:
	case MINI_STATEMENT:
	case LINK_ACCOUNT_INQUIRY:
		return AUTHORIZATION_REQUEST_MTI;
	case REVERSAL:
		return REVERSAL_ADVICE_MTI;
	default:
		return NETWORK_MGT_REQUEST_MTI;
	}
}