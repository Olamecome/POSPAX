#pragma once


#pragma once
/*
* isoDef.h
*
*  Created on: Aug 13, 2018
*      Author: ayodeji.bamitale
*/

#ifndef INC_XISODEF_H_
#define INC_XISODEF_H_


//MTI
#define AUTHORIZATION_REQUEST_MTI  "0100"
#define AUTHORIZATION_RESPONSE_MTI  "0110"

#define TRANSACTION_REQUEST_MTI  "0200"
#define TRANSACTION_RESPONSE_MTI  "0210"
#define TRANSACTION_ADVISE_MTI "0220"

#define REVERSAL_ADVICE_MTI  "0420"
#define REVERSAL_ADVICE_REPEAT_MTI  "0421"
#define REVERSAL_ADVICE_RESPONSE_MTI  "0430"

#define NETWORK_MGT_REQUEST_MTI  "0800"
#define NETWORK_MGT_REQUEST_RESPONSE_MTI  "0810"

#define PURCHASE_ISO_CODE  "00"
#define CASH_ADVANCE_ISO_CODE  "01"
#define REVERSAL_ISO_CODE  "00"
#define REFUND_ISO_CODE  "20"
#define DEPOSIT_ISO_CODE  "21"
#define PURCHASE_WITH_CASH_BACK_ISO_CODE  "09"
#define BALANCE_INQUIRY_ISO_CODE  "31"
#define LINK_ACCOUNT_INQUIRY_ISO_CODE  "30"
#define MINI_STATEMENT_ISO_CODE  "38"
#define FUND_TRANSFER__ISO_CODE  "40"
#define BILL_PAYMENTS_ISO_CODE  "48"
#define PREPAID_ISO_CODE  "4A"
#define BILLER_LIST_DOWNLOAD_ISO_CODE  "4B"
#define PRODUCT_LIST_DOWNLOAD_ISO_CODE  "4C"
#define BILLER_SUBSCRIPTION_INFORMATION_DOWNLOAD_ISO_CODE  "4D"
#define PAYMENT_VALIDATION_ISO_CODE  "4E"
#define PURCHASE_WITH_ADDITIONAL_DATA_ISO_CODE "4F"
#define PRE_AUTHORIZATION_ISO_CODE  "60"
#define PRE_AUTHORIZATION_COMPLETION_ISO_CODE  "61"
#define PIN_CHANGE_ISO_CODE  "90"
#define TERMINAL_MASTER_KEY_ISO_CODE  "9A"
#define TERMINAL_SESSION_KEY_ISO_CODE  "9B"
#define TERMINAL_PIN_KEY_ISO_CODE  "9G"
#define TERMINAL_PARAMETER_DOWNLOAD_ISO_CODE  "9C"
#define CALL_HOME_ISO_CODE  "9D"
#define CA_PUBLIC_KEY_DOWNLOAD_ISO_CODE  "9E"
#define EMV_APPLICATION_AID_DOWNLOAD_ISO_CODE  "9F"
#define DAILY_TRANSACTION_REPORT_DOWNLOAD "9H"
#define INITIAL_PIN_ENCRYPTION_KEY_DOWNLOAD_TRACK2_DATA "9I"
#define INITIAL_PIN_ENCRYPTION_KEY_DOWNLOAD_EMV "9J"
#define DYNAMIC_CURRENCY_CONVERSION "9K"
#define NEW_WORKING_KEY_INQUIRY_FROM_HOST "92"
#define NEW_WORKING_KEY_FOR_TRAFFIC_ENCRYPTION_INQUIRY "95"
#define ECHO_TEST_CODE "99"

#define ISO_ACCOUNT_TYPE_DEFAULT "00"
#define ISO_ACCOUNT_TYPE_SAVINGS "10"
#define ISO_ACCOUNT_TYPE_CURRENT "20"
#define ISO_ACCOUNT_TYPE_CREDIT "30"
#define ISO_ACCOUNT_TYPE_UNIVERSAL "40"
#define ISO_ACCOUNT_TYPE_INVESTMENT "50"
#define ISO_ACCOUNT_TYPE_BONUS "91"

#define isSuccessResponse(responseCode) (memcmp(responseCode, "00", 2) == 0)
#define ERROR_RESPONSE_CODE "06"
#define INVALID_RESPONSE_CODE "20"
#define COMM_ERROR_CODE "Z3"

#define MESSAGE_REASON_CODE_CUSTOMER_CANCELLATION "4000"  //Customer cancellation
#define MESSAGE_REASON_CODE_UNSPECIFIED "4001" //Unspecified, no action taken
#define MESSAGE_REASON_CODE_COMPLETED_PARTIALLY "4004" //Completed partially
#define MESSAGE_REASON_CODE_TIMEOUT "4021"// Timeout waiting for response

typedef enum {
	TYPE_MASTER_KEY, TYPE_SESSION_KEY, TYPE_PIN_KEY
} NIBSS_KEY_TYPE;


typedef enum {
	DEFAULT = 0, SAVINGS, CURRENT, CREDIT, UNIVERSAL, INVESTMENT, BONUS = 91
} AccountType;


typedef enum {
	REASON_TIME_OUT = 0, REASON_CANCELLATION, REASON_UNSPECIFIED, REASON_COMPLETED_PARTIALLY
}ReversalReason;


typedef enum FieldName
{
	MESSAGE_TYPE_INDICATOR_0 = 0,
	BITMAP = 1,
	PRIMARY_ACCOUNT_NUMBER_2 = 2,
	PROCESSING_CODE_3 = 3,
	TRANSACTION_AMOUNT_4 = 4,
	TRANSACTION_DATE_TIME_7 = 7,
	SYSTEM_TRACE_AUDIT_NUMBER_11 = 11,
	LOCAL_TIME_OF_TRANSACTION_12 = 12,
	LOCAL_DATE_OF_TRANSACTION_13 = 13,
	EXPIRATION_DATE_14 = 14, //the date on which settlement between the gateway and intermediate network facilities will be done
	SETTLEMENT_DATE_15 = 15, //n4, MMDD, The month and day for which finacial totals are reconciled between the acquirer and the issuer.
	MERCHANT_TYPE_18 = 18,  //n4, The classification of the merchant's type of business product or service. Codes to be developed within each country.
	POS_ENTRY_MODE_22 = 22, //N3, A series of codes that identify the actual method used to capture the account number.
	CARD_SEQUENCE_NUMBER_23 = 23, //n3,A number distinguishing between separate cards with the same primary account number or primary account number extended.
	POS_CONDITION_CODE_25 = 25, //n2, A code that describes the condition under which the transaction takes place at the Point-Of-Service
	POS_PIN_CAPTURE_CODE_26 = 26, //n2, The maximum number of PIN characters that can be accepted by the Point-of-Service device. Valid values are "04" to "12"("00" to "03" are reserved by ISO) and if the POS device does not accept PINs or it is unknown whether the device does, this value should be set to "12".
	AMOUNT_TRANSACTION_FEE_28 = 28, // x + n8, A fee charged, by the acquirer to the issuer, for transaction activity, in the currency of the amount, transaction.
	AMOUNT_TRANSACTION_PROCESSING_FEE_30 = 30, //x + n8, A fee charged by the network for the handling and routing of messages, in the currency of amount, transaction. This field is usually inserted by the network into the applicable messages
	ACQUIRING_INSTITUTION_IDENTIFICATION_CODE_32 = 32, //an.. 11
	FORWARDING_INSTITUTION_IDENTIFICATION_CODE_33 = 33,
	TRACK2_DATA_35 = 35,
	RETRIVAL_REFERENCE_NUMBER_37 = 37, //an 12, A reference number supplied by the system retaining the original source information and used to assist in locating that information or a copy thereof.
	AUTHORIZATION_CODE_38 = 38, //n6 A code assignd by the authorizing institution indicating approval.
	RESPONSE_CODE_39 = 39, //an 2 A code that defines the disposition of a transaction see responseCodeToString().
	SERVICE_RESTRICTION_CODE_40 = 40,
	CARD_ACCEPTOR_TERMINAL_IDENTIFICATION_41 = 41,
	CARD_ACCEPTOR_IDENTIFICATION_CODE_42 = 42,
	CARD_ACCEPTOR_NAME_OR_LOCATION_43 = 43,
	ADDITIONAL_DATA_48 = 48, //e.g mini statement, see doc
	TRANSACTION_CURRENCY_CODE_49 = 49,
	PIN_DATA_52 = 52, //hex16, The pin data field contains the PIN (a number assigned to a cardholder intended to uniquely identify that cardholder) of the cardholder formatted into a 64-bit block and encrypted with a DES key.
	SECURITY_RELATED_CONTROL_INFORMATION_53 = 53, //eg PIN Change transaction see doc.
	ADDITIONAL_AMOUNTS_54 = 54,
	ICC_DATA_55 = 55,
	MESSAGE_REASON_CODE_56 = 56,
	ECHO_DATA_59 = 59,
	PAYMENT_INFORMATION_60 = 60,
	PRIVATE_FIELD_MANAGEMENT_DATA_1_62 = 62,
	PRIVATE_FIELD_MANAGEMENT_DATA_2_63 = 63,
	PRIMARY_MESSAGE_HASH_VALUE_64 = 64,
	ORIGINAL_DATA_ELEMENTS_90 = 90,
	REPLACEMENT_AMOUNTS_95 = 95,
	ACCOUNT_IDENTIFICATION_1_102 = 102,
	ACCOUNT_IDENTIFICATION_2_103 = 103,
	POS_DATA_CODE_123 = 123,
	CLSS_DATA_124 = 124,
	SECONDARY_MESSAGE_HASH_VALUE_128 = 128,

}FieldName;

typedef struct {
	char accountType[2 + 1];
	char amountType[2 + 1];
	char currencyCode[3 + 1];
	char amountSign;
	char amount[12 + 1];

} AdditionalAmount;

typedef struct NibssTerminalParameter
{
	char ctmsDateAndTime[15];
	char cardAcceptiorIdentificationCode[16];
	char timeout[3];
	char currencyCode[4];
	char countryCode[4];
	char callHomeTime[3];
	char merchantNameAndLocation[41];
	char merchantCategoryCode[5];
} NibssTerminalParameter;


/**
*
* @param isoRequestType [in]
* @param accountType [in]
* @param processingCode [out]
* @return
*/
int getProcessingCode(int isoRequestType, AccountType accountType, char* processingCode);


/**
*
* @param inputString [in]
* @param additionalAmt [out]
* @return APP_SUCC, APP_FAIL
*/
int getSingleAdditionalAmount(const char* inputString, AdditionalAmount* additionalAmt);

/**
*
* @param inputString [in]
* @param buffer 	[out]
* @param bufferlen [in]
* @return actual count parsed, APP_FAIL
*/
int parseField54AdditionalAmount(const char* inputString, AdditionalAmount* buffer, int bufferlen);

/**
*
* @param accountType [in]
* @return ISO ACCOUNT TYPE CODE
*/
const char* getIsoAccountType(AccountType accountType);

void logAdditionalAmount(AdditionalAmount * addAmount);

char* responseCodeToString(const char* responseCode);


short expandField62(NibssTerminalParameter * field62, char* field62Str);

/**
*
* @param reason [in]
* @param outStr [out]
*/
const char* getReversalReasonCode(int reason);



/**
*
* @param isoTransType
* @return
*/
const char* getMTI(int isoTransType);

#endif /* INC_XISODEF_H_ */
