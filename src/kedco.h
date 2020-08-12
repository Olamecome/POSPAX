#include "parson.h"
#include "printHelper.h"
#include "parsonHelper.h"
#include "http_handler.h"
#include "HttpApi.h"
#include "xDefs.h"
#include "xui.h"
#include "util.h"
#include "Logger.h"
#include "utils.h"

#define KEDCO_IP "172.21.7.5"
#define KEDCO_PORT "9002"
#define WALLET_IP "80
#define WALLET_PORT "80.88.8.245"
#define WALLET_USERNAME ""
#define WALLET_PASSWORD ""
#define WALLET_PATH "WalletService/xpress/WalletService"

#define KEDCO_VALIDATOR_ENDPOINT	"http:"KEDCO_IP":"KEDCO_PORT"/api/KEDCO/validation"
#define KEDCO_REPORTER_ENDPOINT		"http:"KEDCO_IP":"KEDCO_PORT"/api/KEDCO/pay"
#define KEDCO_REQUERY_ENDPOINT		"http:"KEDCO_IP":"KEDCO_PORT"/api/KEDCO/requery"
#define KEDCO_REVERSAL_ENDPOINT		"http:"KEDCO_IP":"KEDCO_PORT"/api/KEDCO/paymentReversal"
#define KEDCO_WALLET_ENDPOINT		"http:"WALLET_IP":"WALLET_PORT"/"WALLET_PATH"/POSDirectDebit"

#define KEDCO_REPORTER_TEMPLATE \
"{ \
	\"customerAccountNumber\":\"%s\", \
	\"billerCode\" : \"%s\", \
	\"referenceNumber\" : \"%s\", \
	\"amount\" : \"%s\", \
	\"channel\" : \"POS\" \
}"

#define KEDCO_VALIDATION_TEMPLATE \
"{ \
	\"billerCode\":\"%s\", \
	\"customerAccountNumber\" : \"%s\", \
	\"field2\" : \"POS\" \
}"

#define KEDCO_WALLET_TEMPLATE \
"{ \
	\"channelUsername\" : \"%s\", \
	\"channelPassword\" : \"%s\", \
	\"sourceWalletAccount\" : \"%s\", \
	\"Pin\" : \"%s\", \
	\"Amount\" : \"%s\" \
}"

#define KEDCO_PREPAID_DISPLAY_TEMPLATE "Name : %s\n \
		Account : %s\n \
		Unit : %s\n \
		Phone No : %s\n  \
		Min. Purchase : NGN %.2f\n \
		Last Trans : %s\n \
		Undertaking : %s"

#define KEDCO_POSTPAID_DISPLAY_TEMPLATE "Name : %s\n \
		Account : %s\n \
		Unit : %s\n \
		Phone No : %s\n  \
		Arrears : NGN %.2f\n \
		Last Trans : %s\n \
		Undertaking : %s"

typedef enum paymentOptions
{ 
	KEDCO_PREPAID, 
	KEDCO_POSTPAID 
} KedcoPaymentOption;

typedef enum kedcoService
{
	KEDCO_VALIDATOR,
	KEDCO_REPORTER,
	KEDCO_WALLET_PAY,
	KEDCO_WALLET_REVERSAL
}KedcoService;

typedef enum paymentChannel
{ 
	KEDCO_CARD, 
	KEDCO_WALLET 
} KedcoPaymentChannel;

typedef struct kedcoRequest
{
	char amount[12];
	char billerCode[30];
	char accountOrMeterNumber[30];
	char walletID[50];
	char walletPin[50];
	KedcoPaymentOption paymentOption;
	KedcoPaymentChannel paymentChannel;
	KedcoService kedcoService;
} KedcoRequest;


typedef struct kedcoResponse
{
	char errorMessage[100];

	char customerName[100];
	char businessUnit[50];
	char phoneNumber[12];
	char lastTransactionDate[50];
	char minimumPurchase[12];
	char undertaking[100];
	char accountNumber[20];
	char customerArears[12];

	//Reporting Response
	char referenceNumber[100];
	char valueToken[100];
	char remoteReferenceNumber[100];
} KedcoResponse;

int kedcoWebResponseParser(char * jsonReply, KedcoRequest request, KedcoResponse * response);
int kedcoWebServiceHandler(KedcoRequest request, KedcoResponse * response);
void processKedcoReversal(KedcoRequest request, KedcoResponse response);
void completeKedcoTransaction(KedcoRequest request, KedcoResponse response);
int selectKedcoPaymentOption(KedcoRequest * request);
int retreiveKedcoPaymentNumber(KedcoRequest * request);
int retreiveKedcoAmount(KedcoRequest * request);
int selectKedcoPaymentChannel(KedcoRequest * request);
int confirmKedcoResponse(KedcoRequest request, KedcoResponse response);
int retrieveWalletInfo(KedcoRequest * request);