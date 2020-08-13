/*
 * xdefs.h
 *
 *  Created on: Jul 30, 2018
 *      Author: ayodeji.bamitale
 */

#ifndef INC_XDEFS_H_
#define INC_XDEFS_H_

#include <stddef.h>
#include "stdbool.h"


#define APP_SUCC		(GUI_OK)   	/**<Success      */
#define APP_FAIL		(GUI_ERR_UNSUPPORTED)    /**<Fail      */
#define APP_QUIT		(GUI_ERR_NOTINIT)    /**<Quit*/
#define APP_TIMEOUT 	(GUI_ERR_TIMEOUT)    /**<Timeout    */
//#define APP_FUNCQUIT	(-4)    /**<Function key*/
#define APP_CANCEL		(GUI_ERR_USERCANCELLED)	/**<Cancelled */

#ifndef uint
#define uint unsigned int
#endif // !uint

#ifndef EOF
# define EOF (-1)
#endif

#define RECEIPT_LOGO_FILE "receipt_logo.bmp"

#define lengthOf(x)  sizeof(x)/sizeof(x[0])

#define PARSE_OK 0
#define PARSE_ERROR -100

#define DEFAULT_TIME_OUT 30
#define DEFAULT_XPRESS_PASSWORD "6798"
#define DEFAULT_OPERATOR_PASSWORD "1234"
#define DEFAULT_SUPERVISOR_PASSWORD "1234"
#define DEV_PASS "9739"

#define INFO_TIME_OUT 5


#define BYTE_KEY_SIZE 16
#define ASCII_KEY_SIZE  32
#define ASCII_KCV_SIZE 6
#define BYTE_KCV_SIZE 3

#define DEFAULT_PASSWORD_TIMEOUT 10

#define START_SCREEN 0

#define NIBSS_MASTER_KEY_INDEX  8
#define NIBSS_SESSION_KEY_INDEX 9
#define NIBSS_PIN_KEY_INDEX 10

#define TRANZ_MASTER_KEY_INDEX 18
#define TRANZ_WORKING_KEY_INDEX 19
#define TRANZ_PIN_KEY_INDEX 20

#define isTimedOutOrCancelled(x) (x == APP_TIMEOUT || x == APP_QUIT)


#define strmcpy(x,y,z) strncpy(x, y, z - 1)

#define ASSERT_RETURNCODE(e) \
	{\
		int nTemp=e;\
		if (nTemp != APP_SUCC)\
		{\
			return nTemp;\
		}\
	}

#define CLEAR(bytes, xter)  memset(bytes, xter, sizeof(*bytes))
#define CLEAR_STRING(string, len)  memset(string, '\0', len)

#define IS_NULL_EMPTY(string) (string == NULL || strlen(string) == 0)


typedef enum {
	CT = 1, CTLS, MSR, MANUAL, CT_MSR, CT_CTLS , CT_CTLS_MSR, CT_CTLS_MANUAL, CT_CTLS_MSR_MANUAL, NO_READER
} ReaderType;

typedef enum HostType {
	NIBSS = 0, TMS, NO_HOST
} HostType;

typedef enum {
	ONLINE = 0, OFFLINE
} OperationMode;

typedef enum {
	//EFT TRAN TYPES
	PURCHASE = 1, PURCHASE_CASH = 2, PURCHASE_WITH_CASH_BACK = 3,
	REVERSAL = 4, REFUND = 5, BALANCE = 7, PIN_CHANGE = 8, MINI_STATEMENT = 9, TRANSFER = 10,
	DEPOSIT = 11, PURCHASE_WITH_ADDITIONAL_DATA,
	POS_PRE_AUTHORIZATION = 33, POS_PRE_AUTH_COMPLETION = 34,
	CASH_ADVANCE = 45, WITHDRAWAL = 46,
	PIN_SELECTOION = 47,
	BILL_PAYMENT, PREPAID, VOID,
	LINK_ACCOUNT_INQUIRY = 102,
	BILLER_LIST_DOWNLOAD, PRODUCT_LIST_DOWNLOAD,
	BILLER_SUBSCRIPTION_INFO_DOWNLOAD, PAYMENT_VALIDATION,

	PAYXPRESS, SPIDER_WEB,

	KEDCO,

	//NETWORK MGT TYPE
	TERMINAL_MASTER_KEY,
	TERMINAL_SESSION_KEY,
	TERMINAL_PIN_KEY,
	TERMINAL_PARAMETER_DOWNLOAD,
	CALL_HOME,
	EOD,
	CA_PUBLIC_KEY_DOWNLOAD,
	EMV_APPLICATION_AID_DOWNLOAD,
	TRANZAXIS_WORKING_KEY_INQUIRY,
	TRANZAXIS_TRAFFIC_ENCRYPTION_WORKING_KEY,
	TRANZAXIS_ECHO_TEST,

	//ADMIN TYPES
	TERMINAL_NIBSS_KEY_EXCHANGE,
	TERMINAL_NIBSS_PARAM_DOWNLOAD,
	TERMINAL_NIBSS_PREP,
	COMM_SETTINGS, CALL_HOME_KEEP_ALIVE, COM_PARAM_PRINT,
	TERMINAL_ID_CONFIG, TMS_CONN_CONFIG, TMS_MENU_DOWNLOAD, REPORTING, TERMINAL_MANUAL_UPDATE, TERMINAL_POWER_SETTINGS,
	COLLECTIONS, ECR_MENU, CASH_MOP_UP, WALLET_MENU, TERMINAL_REMOTE_DOWNLOAD, EXPORT_LOG, STRESS_TEST, PHED, GATEWAY, COUNT_OF_RECEIPT, SUPERVISOR_PIN_UPDATE, PAYATTITUDE, ACCT_SELECT_OPTION
} TransactionType;


//typedef STSYSTEM EftTransactionData;


typedef enum {
	CASH = 1, CARD, CHEQUE, WALLET
} PaymentOption;

typedef enum {
	NORMAL, LOTTO
} TerminalMode;


typedef struct {
	void* data;
	int recordIdx;
	int result;
} RunnerData;


typedef struct {
	char responseCode[2 + 1];
	char cardExpiry[4 + 1];
	char date[8 + 1];
	char RRN[12 + 1];
	char amount[12 + 1];
	char otherAmount[12 + 1];
	char maskedPan[19 + 1];
	char time[6 + 1];
	int tranType;
	int batchNumber;
	int sequenceNumber;
}ReportData;

typedef struct {
	ReportData* data;
	int reportCount;
	long totalApproved;
	long totalDeclined;
}SummaryReport;


//typedef struct Device {
//	bool enableAccountSelection;
//} Device;


#endif /* INC_XDEFS_H_ */
