#include "kedco.h"

extern int rollbackNibssTransaction(int reason);
extern int statusReceiptAndNotification();

int kedcoHandler()
{
	KedcoRequest request;
	KedcoResponse response;

	CLEAR(&request, 0);
	CLEAR(&response, 0);

	while (true)
	{
		logTrace("Retrieving Payment Option");
		if (selectKedcoPaymentOption(&request) != APP_SUCC) 
		{
			logTrace("Exiting KEDCO");
			break;
		}

		logTrace("Retrieving Payment Number");
		if (retreiveKedcoPaymentNumber(&request) != APP_SUCC)
		{
			logTrace("Cancelled Payment Number");
			continue;
		}
		
		request.kedcoService = KEDCO_VALIDATOR;
		logTrace("Requesting Validation");
		if (kedcoWebServiceHandler(request, &response) != APP_SUCC)
		{
			logTrace("[ KEDCO ] %s", response.errorMessage);
			showErrorDialog(response.errorMessage, 10);
			continue;
		}
		
		logTrace("Confirming Response");
		if (confirmKedcoResponse(request, response) != APP_SUCC)
		{
			continue;
		}

	AMOUNT__:
		logTrace("Retrieving Amount To Pay");
		if (retreiveKedcoAmount(&request) != APP_SUCC)
		{
			continue;
		}

		if (request.paymentOption == KEDCO_PREPAID && (atof)(response.minimumPurchase) > (((atof)(request.amount)) / 100))
		{
			char message[200];
			snprintf(message, sizeof(message), "Must Pay Above\nNGN %.2f", (atof)(response.minimumPurchase));
			logTrace("[ KEDCO ] %s", message);
			showMessageDialog("KEDCO", message, 1, 10);
			goto AMOUNT__;
		}

		logTrace("Retreiving Payment Channel");
		if (selectKedcoPaymentChannel(&request) != APP_SUCC)
		{
			logTrace("Cancelling Payment Channel");
			continue;
		}

		if (request.paymentChannel == KEDCO_WALLET)
		{
			logTrace("Retreiving Wallet Information");
			if (retrieveWalletInfo(&request) != APP_SUCC)
			{
				logTrace("Cancelled Wallet Information");
				continue;
			}
		}

		logTrace("Initiating Kedco Transaction Completion");
		completeKedcoTransaction(request, response);
	}

	return APP_QUIT;
}

int retreiveKedcoAmount(KedcoRequest * request)
{
	Prompt prompt;
	getDefaultAmountPrompt(&prompt, "Amount");

	if (showPrompt(&prompt) != 0) 
	{
		return APP_CANCEL;
	}

	strcpy(request->amount, prompt.value);
	return APP_SUCC;
}

int selectKedcoPaymentOption(KedcoRequest * request)
{
	Prompt prompt;
	getListItemPrompt(&prompt, "KEDCO", "PREPAID|POSTPAID");
	if (showPrompt(&prompt) != 0)
	{
		return APP_CANCEL;
	}
	
	switch (prompt.selectionOption)
	{
		case 0:
			request->paymentOption = KEDCO_PREPAID;
			strcpy(request->billerCode, "KEDCOPrepaid");
			break;

		case 1:
			request->paymentOption = KEDCO_POSTPAID;
			strcpy(request->billerCode, "KEDCOPostpaid");
			break;

		default:
			return APP_CANCEL;
	}

	return APP_SUCC;
}

int retreiveKedcoPaymentNumber(KedcoRequest * request)
{
	Prompt prompt;
	char * message = (request->paymentOption == KEDCO_PREPAID) ? "Enter Meter No : " : "Enter Account No :";
	getTextPrompt(&prompt, "KEDCO", message);

	if (showPrompt(&prompt) != 0)
	{
		return APP_CANCEL;
	}

	strcpy(request->accountOrMeterNumber, prompt.value);
	return APP_SUCC;
}

int selectKedcoPaymentChannel(KedcoRequest * request)
{
	Prompt prompt;
	getListItemPrompt(&prompt, "KEDCO", "CARD|WALLET");
	if (showPrompt(&prompt) != 0)
	{
		return APP_CANCEL;
	}

	switch (prompt.selectionOption)
	{
		case 0:
			request->paymentChannel = KEDCO_CARD;
			break;

		case 1:
			request->paymentChannel = KEDCO_WALLET;
			break;

		default:
			return APP_CANCEL;
	}

	return APP_SUCC;
}

int confirmKedcoResponse(KedcoRequest request, KedcoResponse response)
{
	char displayInfo[1024] = "\0";

	switch (request.paymentOption)
	{
		case KEDCO_PREPAID:
			snprintf(displayInfo, sizeof(displayInfo), KEDCO_PREPAID_DISPLAY_TEMPLATE,
				response.customerName, request.accountOrMeterNumber, response.businessUnit, response.phoneNumber,
				response.minimumPurchase, response.lastTransactionDate, response.undertaking);
			break;

		case KEDCO_POSTPAID:
			snprintf(displayInfo, sizeof(displayInfo), KEDCO_POSTPAID_DISPLAY_TEMPLATE,
				response.customerName, request.accountOrMeterNumber, response.businessUnit, response.phoneNumber,
				response.customerArears, response.lastTransactionDate, response.undertaking);
			break;

		default:
			return APP_FAIL;
	}

	showInfo("KEDCO", 60, 1, displayInfo);

	return APP_SUCC;
}

int kedcoWebResponseParser(char * jsonReply, KedcoRequest request, KedcoResponse * response)
{
	int toReturn = APP_FAIL;

	JsonValue rootValue = json_parse_string(jsonReply);
	if (!rootValue || (json_value_get_type(rootValue) != JSONObject)) 
	{
		strcpy(response->errorMessage, "Invalid Response");
		json_value_free(rootValue);
		return toReturn;
	}

	Json json = json_value_get_object(rootValue);

	if (getJsonInt(json, "ResponseCode") != 0)
	{
		getJsonString(json, "ResponseMessage", response->errorMessage, sizeof(response->errorMessage));
		toReturn = APP_CANCEL;
	}
	else
	{
		switch (request.kedcoService)
		{
			case KEDCO_VALIDATOR:
				switch (request.paymentOption)
				{
					case KEDCO_PREPAID:
						getDotJsonString(json, "KEDCOValidationResponse.customerName", response->customerName, strlen(response->customerName));
						getDotJsonString(json, "KEDCOValidationResponse.businessUnit", response->businessUnit, strlen(response->businessUnit));
						getDotJsonString(json, "KEDCOValidationResponse.undertaking", response->undertaking, strlen(response->undertaking));
						getDotJsonString(json, "KEDCOValidationResponse.minimumPurchase", response->minimumPurchase, strlen(response->minimumPurchase));
						getDotJsonString(json, "KEDCOValidationResponse.lastTransactionDate", response->lastTransactionDate, strlen(response->lastTransactionDate));
						getDotJsonString(json, "KEDCOValidationResponse.phoneNumber", response->phoneNumber, strlen(response->phoneNumber));
						toReturn = APP_SUCC;
						break;

					case KEDCO_POSTPAID:
						getDotJsonString(json, "KEDCOValidationResponse.customerName", response->customerName, strlen(response->customerName));
						getDotJsonString(json, "KEDCOValidationResponse.businessUnit", response->businessUnit, strlen(response->businessUnit));
						getDotJsonString(json, "KEDCOValidationResponse.lastTransactionDate", response->phoneNumber, strlen(response->lastTransactionDate));
						getDotJsonString(json, "KEDCOValidationResponse.undertaking", response->undertaking, strlen(response->undertaking));
						getDotJsonString(json, "KEDCOValidationResponse.accountNumber", response->accountNumber, strlen(response->accountNumber));
						getDotJsonString(json, "KEDCOValidationResponse.customerArrears", response->customerArears, strlen(response->customerArears));
						getDotJsonString(json, "KEDCOValidationResponse.phoneNumber", response->phoneNumber, strlen(response->phoneNumber));
						toReturn = APP_SUCC;
						break;

					default:
						toReturn = APP_CANCEL;
						break;
				}
				break;

			case KEDCO_REPORTER:
				getDotJsonString(json, "referenceNumber", response->referenceNumber, strlen(response->referenceNumber));
				getDotJsonString(json, "remoteServiceReferenceNumber", response->remoteReferenceNumber, strlen(response->remoteReferenceNumber));
				getDotJsonString(json, "valueToken", response->valueToken, strlen(response->valueToken));
				toReturn = APP_SUCC;
				break;
			
			case KEDCO_WALLET_PAY:
				break;

			case KEDCO_WALLET_REVERSAL:
				break;

			default:
				toReturn = APP_CANCEL;
		}
	}


	json_value_free(rootValue);
	return toReturn;
}

int kedcoWebServiceHandler(KedcoRequest request, KedcoResponse * response)
{
	char data[500];
	MemoryStruct jsonResponder;
	int toReturn;

	switch (request.kedcoService)
	{
		case KEDCO_VALIDATOR:
			toReturn = sendHttpRequest(HTTP_GET, KEDCO_VALIDATOR_ENDPOINT, NULL, 0, NULL, 0, &jsonResponder);
			break;

		case KEDCO_REPORTER:
			toReturn = sendHttpRequest(HTTP_POST, KEDCO_REPORTER_ENDPOINT, data, strlen(data), NULL, 0, &jsonResponder);
			break;

		default:
			break;
	}

	if (toReturn != 0)
	{
		strcpy(response->errorMessage, "Comm. Error");
		toReturn = APP_CANCEL;
	}
	else
	{
		toReturn = kedcoWebResponseParser(jsonResponder.memory, request, response);
	}

	if (jsonResponder.memory)
	{
		free(jsonResponder.memory);
	}

	return toReturn;
}

void processKedcoReversal(KedcoRequest request, KedcoResponse response)
{
	switch (request.paymentChannel)
	{
		case KEDCO_CARD:
			rollbackNibssTransaction(0);
			break;

		case KEDCO_WALLET:
			request.paymentChannel = KEDCO_WALLET_REVERSAL;
			kedcoWebServiceHandler(request, &response);
			break;
	}
}

void completeKedcoTransaction(KedcoRequest request, KedcoResponse response)
{
	logTrace("Meter|Account Number : %s", request.accountOrMeterNumber);
	logTrace("BillerCode : %s", request.billerCode);
	logTrace("Customer Name : %s", response.customerName);
	logTrace("Business Unit : %s", response.businessUnit);
	logTrace("Last Transaction Date : %s", response.lastTransactionDate);
	logTrace("Undertaking : %s", response.undertaking);
	logTrace("Account Number: %s", response.accountNumber);
	logTrace("Customer Arrears : %s", response.customerArears);
	logTrace("Phone Number : %s", response.phoneNumber);
	logTrace("Minimum Purchase : %s", response.minimumPurchase);

	switch (request.paymentChannel)
	{
		case KEDCO_CARD:
			startEmvTransaction(CARD_INSERTED | CARD_TAPPED, KEDCO, glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount);
			break;

		case KEDCO_WALLET:
			request.paymentOption = KEDCO_WALLET_PAY;
			if (kedcoWebServiceHandler(request, &response) != APP_SUCC)
			{
				showErrorDialog(response.errorMessage, 10);
				return;
			}
			break;

		default:
			return;
	}

	request.kedcoService = KEDCO_REPORTER;
	if (kedcoWebServiceHandler(request, &response) != APP_SUCC)
	{
		showErrorDialog(response.errorMessage, 10);
		return;
	}

	switch (request.paymentChannel)
	{
		case KEDCO_CARD:
			rollbackNibssTransaction(0);
			break;

		case KEDCO_WALLET:
			request.kedcoService = KEDCO_WALLET_REVERSAL;
			kedcoWebServiceHandler(request, &response);
			break;

	}
}

int retrieveWalletInfo(KedcoRequest * request)
{
	Prompt prompt;
	getTextPrompt(&prompt, "WALLET", "Enter Wallet ID : ");
	if (showPrompt(&prompt) != 0)
	{
		return APP_CANCEL;
	}
	snprintf(request->walletID, sizeof(request->walletID), "%s", prompt.value);

	getPasswordPrompt(&prompt, "Enter Wallet PIN :", 45);
	if (showPrompt(&prompt) != 0)
	{
		return APP_CANCEL;
	}
	snprintf(request->walletPin, sizeof(request->walletPin), "%s", prompt.value);

	return APP_SUCC;
}