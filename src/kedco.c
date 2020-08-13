#include "kedco.h"

extern int rollbackNibssTransaction(int reason);
extern int statusReceiptAndNotification();

int kedcoHandler()
{
	KedcoRequest request;
	KedcoResponse response;

	while (1)
	{
		KedcoTrace
		CLEAR(&request, 0);
		CLEAR(&response, 0);

		if (selectKedcoPaymentOption(&request) != APP_SUCC) 
		{
			KedcoDebug("Cancelled At Payment Option")
			break;
		}

		if (retreiveKedcoPaymentNumber(&request) != APP_SUCC)
		{
			KedcoDebug("Cancelled At Payment Number")
			continue;
		}
		
		request.kedcoService = KEDCO_VALIDATOR;
		if (kedcoWebServiceHandler(request, &response) != APP_SUCC)
		{
			KedcoDebug(response.errorMessage)
			showErrorDialog(response.errorMessage, 10);
			continue;
		}
		
		if (confirmKedcoResponse(request, response) != APP_SUCC)
		{
			continue;
		}

	AMOUNT__:
		if (retreiveKedcoAmount(&request) != APP_SUCC)
		{
			continue;
		}

		if (request.paymentOption == KEDCO_PREPAID && (atof)(response.minimumPurchase) > (((atof)(request.amount)) / 100))
		{
			char message[200];
			snprintf(message, sizeof(message), "Pay NGN %.2f\n Or Above", (atof)(response.minimumPurchase));
			KedcoDebug(message);
			showMessageDialog("KEDCO", message, 1, 10);
			goto AMOUNT__;
		}

		if (selectKedcoPaymentChannel(&request) != APP_SUCC)
		{
			KedcoDebug("Cancelled At Payment Channel")
			continue;
		}

		if (request.paymentChannel == KEDCO_WALLET)
		{
			if (retrieveWalletInfo(&request) != APP_SUCC)
			{
				KedcoDebug("Cancelled At Wallet Information")
				continue;
			}
		}

		completeKedcoTransaction(request, response);
	}

	return APP_QUIT;
}

int retreiveKedcoAmount(KedcoRequest * request)
{
	KedcoTrace
	Prompt prompt;
	getDefaultAmountPrompt(&prompt, "Amount");
	strcpy(prompt.hint, "Enter Amount to Pay :");

	if (showPrompt(&prompt) != 0) 
	{
		return APP_CANCEL;
	}

	strcpy(request->amount, prompt.value);
	return APP_SUCC;
}

int selectKedcoPaymentOption(KedcoRequest * request)
{
	KedcoTrace
	if (!glPosParams.ucIsPrepped) {
		showErrorDialog("Terminal not prepped", 10);
		return APP_FAIL;
	}

	if (glPosParams.tranRecordCount >= MAX_TRANLOG) {
		DispErrMsg("Memory Full", "Run Close Batch!!!", USER_OPER_TIMEOUT, DERR_BEEP);
		return APP_FAIL;
	}

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
	KedcoTrace
	Prompt prompt;
	char * message = (request->paymentOption == KEDCO_PREPAID) ? "Enter Meter No : " : "Enter Account No :";
	getTextPrompt(&prompt, "KEDCO", NULL);
	snprintf(prompt.hint, sizeof(prompt.hint), "%s", message);

#ifdef APP_DEBUG
	strcpy(prompt.value, "301001002001");
#endif //APP_DEBUG

	if (showPrompt(&prompt) != 0)
	{
		return APP_CANCEL;
	}

	strcpy(request->accountOrMeterNumber, prompt.value);
	return APP_SUCC;
}

int selectKedcoPaymentChannel(KedcoRequest * request)
{
	KedcoTrace
	Prompt prompt;
	getListItemPrompt(&prompt, "PAYMENT CHANNEL", "CARD|WALLET");
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
	KedcoTrace
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
	KedcoTrace
	int toReturn = APP_FAIL;

	logTrace(jsonReply);

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
						getDotJsonString(json, "KEDCOValidationResponse.customerArrears", response->customerArears, strlen(response->customerArears));
						getDotJsonString(json, "KEDCOValidationResponse.phoneNumber", response->phoneNumber, strlen(response->phoneNumber));
						getDotJsonString(json, "KEDCOValidationResponse.accountNumber", response->accountNumber, strlen(response->accountNumber));
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
				
				if (strlen(response->valueToken) > 5)
				{
					strcpy(response->status, "APPROVED");
					toReturn = APP_SUCC;
				}
				else
				{
					toReturn = APP_CANCEL;
				}
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
	logTrace("Finish Parsing Web Response");
	return toReturn;
}

int kedcoWebServiceHandler(KedcoRequest request, KedcoResponse * response)
{
	KedcoTrace
	char data[500];
	char * HEADERS[1] = {"Content-Type: application/json"};

	MemoryStruct jsonResponder;
	int toReturn;
	CLEAR(&data, 0);

	switch (request.kedcoService)
	{
		case KEDCO_VALIDATOR:
			snprintf(data, sizeof(data), KEDCO_VALIDATION_TEMPLATE, request.billerCode, request.accountOrMeterNumber);
			logTrace(data);
			toReturn = sendHttpRequest(HTTP_POST, KEDCO_VALIDATOR_ENDPOINT, data, strlen(data), HEADERS, 1, &jsonResponder);
			break;

		case KEDCO_REPORTER:
			snprintf(data, sizeof(data), KEDCO_REPORTER_TEMPLATE, request.accountOrMeterNumber, request.billerCode, 
				glProcInfo.stTranLog.szRRN, (((atof)(request.amount)) / 100));
			logTrace(data);

			strcpy(response->status, "DECLINED");
			toReturn = sendHttpRequest(HTTP_POST, KEDCO_REPORTER_ENDPOINT, data, strlen(data), HEADERS, 1, &jsonResponder);
			break;

		case KEDCO_WALLET_PAY:
			snprintf(data, sizeof(data), KEDCO_WALLET_TEMPLATE, KEDCO_WALLET_USERNAME, KEDCO_WALLET_PASSWORD,
				request.walletID, request.walletPin, (atof(request.amount) / 100));
			logTrace(data);
			toReturn = sendHttpRequest(HTTP_POST, KEDCO_VALIDATOR_ENDPOINT, data, strlen(data), HEADERS, 1, &jsonResponder);
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

	logTrace("Free Up Memory Used");
	if (jsonResponder.memory)
	{
		free(jsonResponder.memory);
	}

	return toReturn;
}

void processKedcoReversal(KedcoRequest request, KedcoResponse response)
{
	KedcoTrace
	switch (request.paymentChannel)
	{
		case KEDCO_CARD:
			rollbackNibssTransaction(REASON_COMPLETED_PARTIALLY);
			break;

		case KEDCO_WALLET:
			request.paymentChannel = KEDCO_WALLET_REVERSAL;
			kedcoWebServiceHandler(request, &response);
			break;
	}
}

void completeKedcoTransaction(KedcoRequest request, KedcoResponse response)
{
	KedcoTrace
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

	snprintf(glProcInfo.stTranLog.szAmount, sizeof(glProcInfo.stTranLog.szAmount), "%s", request.amount);

	switch (request.paymentChannel)
	{
		case KEDCO_CARD:
			startEmvTransaction(CARD_INSERTED | CARD_TAPPED, KEDCO, glProcInfo.stTranLog.szAmount, NULL);
			break;

		case KEDCO_WALLET:
			request.paymentOption = KEDCO_WALLET_PAY;
			if (kedcoWebServiceHandler(request, &response) != APP_SUCC)
			{
				KedcoDebug(response.errorMessage)
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
		processKedcoReversal(request, response);
	}

	CLEAR(&glProcInfo.stTranLog.szEchoField59, 0);
	snprintf(glProcInfo.stTranLog.szEchoField59, sizeof(glProcInfo.stTranLog.szEchoField59),
		"%s|%s|%s|%s|%s|%s|%s|NGN %.2f|%s", response.status,
		response.customerName, response.meterNumber, response.accountNumber, response.address,
		response.tariff, response.rate, (atof(glProcInfo.stTranLog.szAmount) / 100), response.businessUnit);
	
	KedcoDebug(glProcInfo.stTranLog.szEchoField59)
	statusReceiptAndNotification();
}

int retrieveWalletInfo(KedcoRequest * request)
{
	KedcoTrace
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