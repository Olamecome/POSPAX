#include "utils.h"
#include "xIsoDef.h"

extern int InputTransactionAmount(char* title, uchar ucAmtType, uchar amount[12 + 1]);
/**
*
* @param string
* @param separator
* @param tokenArray
* @param length
* @return length of actual tokens
*
* NB: Call deepFreeArray on the tokenArray when you're done using the array element.
*/
int tokenize(char* string, char* separator, char** tokenArray, int length) {
	//	logTrace("=============== %s ====================",__func__);

	int screen_index = 1;
	int len = strlen(string) + 1;
	char* temp = calloc(len, sizeof(char));
	strmcpy(temp, string, len);

	char* token = strtok(temp, separator);
	int count = 0;
	while (token != NULL && count <= 50) {
		if (count >= length) {
			break;
		}

		tokenArray[count] = (char*)calloc(1, strlen(token) + 5);
		snprintf(tokenArray[count], strlen(token) + 5, "%s", token);
		token = strtok(NULL, separator);
		count++;
	}

	free(temp);
	return count;
}

/**
*
* @param array
* @param length
*/
void deepFreeArray(void** array, int length) {
	int i = 0;
	for (; i < length; i++) {
		if (array[i] != NULL) {
			free(array[i]);
		}
	}
}

/**
*
* @param menus
* @param length
* @param titles
*/
void getMenuTitleList(Menu* menus, int length, char* titles[]) {
	int i = 0;

	for (; i < length; i++) {
		titles[i] = (char*)calloc(1, strlen(menus[i].title) + 6);
		sprintf(titles[i], "%d. %s", (i + 1), menus[i].title);
	}
}

/**
*
* @param length [in]
* @param pad [in]
* @param inStr [in]
* @param outStr [out]
*/
void padLeft(int length, char pad, char* inStr, char* outStr) {
	int minLength = length * sizeof(char);
	if (minLength < sizeof(outStr)) {
		return;
	}

	int padLen = length - strlen(inStr);
	padLen = padLen < 0 ? 0 : padLen;

	memset(outStr, 0, sizeof(outStr));
	memset(outStr, pad, padLen);
	memcpy(outStr + padLen, inStr, minLength - padLen);
}

/**
*
* @param inStr [in]
* @param outStr [out]
*/
void toUpper(const char* inStr, char* outStr) {
	if (inStr == NULL || strlen(inStr) == 0) {
		return;
	}

	int i = 0;
	for (; i < strlen(inStr); i++) {
		outStr[i] = toupper(inStr[i]);
	}
}

/**
*
* @param inStr [in]
* @param outStr [out]
*/
void toLower(const char* inStr, char* outStr) {
	if (inStr == NULL || strlen(inStr) == 0) {
		return;
	}

	int i = 0;
	for (; i < strlen(inStr); i++) {
		outStr[i] = tolower(inStr[i]);
	}
}

/*
*@param beep_count[in]
*/
void PubBeep(beep_count) {
	int i = 0;
	for (i = 0; i<beep_count; i++)
	{
		Beef(6, 60);
		DelayMs(80);
	}
}


/**
*
* @param longDate [in]
* @param formattedDate [out]
*/
void formatLongDate(char* longDate, char* formattedDate) {
	if (longDate == NULL) {
		return;
	}
	memset(formattedDate, '\0', sizeof(formattedDate));

	char* d = longDate;

	sprintf(formattedDate, "%c%c/%c%c/%c%c", d[6], d[7], d[4], d[5], d[2],
		d[3]);
}

/**
*
* @param time [in]
* @param formattedTime [out]
*/
void formatShortTime(char* time, char* formattedTime) {
	if (time == NULL) {
		return;
	}
	strncpy(formattedTime, time, 2);
	strcat(formattedTime + 2, ":");
	strncpy(formattedTime + 3, time + 2, 2);
	strcat(formattedTime + 5, ":");
	strncpy(formattedTime + 6, time + 4, 2);
}

/**
* @Brief reverse a string array
* @param array [in out]
*/
void strrev(char* array) {

	if (array == NULL) {
		return;
	}

	int len = strlen(array);
	char* temp =  calloc(len+1, sizeof(char));
	strcpy(temp, array);

	int i = 0;
	while (--len >= 0) {
		array[i++] = temp[len];
	}

	free(temp);
}

/**
* @Brief apply currency-format to the amount
* @param amount [in]
* @param formattedAmount [out]
*/
void formatCurrencyAmount(long amount, char* formattedAmount) {
	double d_amount = amount / 100.0;

	char tempAmount[20] = "\0";
	sprintf(tempAmount, "%#.2f", d_amount);

	if (strlen(tempAmount) > 6) {
		char* numericPart = strtok(tempAmount, ".");
		char* decimalPart = strtok(NULL, ".");

		int delimeter_count = -1;
		int count = strlen(numericPart) - 1;

		int outlen = 0;

		for (; count >= 0; count--) {
			delimeter_count++;

			if (delimeter_count == 3) {
				formattedAmount[outlen++] = ',';
				delimeter_count = 0;
			}

			formattedAmount[outlen++] = tempAmount[count];
		}

		strrev(formattedAmount);
		strcat(formattedAmount, ".");
		strcat(formattedAmount, decimalPart);
	}
	else {
		strcpy(formattedAmount, tempAmount);
	}
}

/**
* @Brief get amount formatted with currency symbol to display on screen
* @param amount [in]
* @param displayAmount [out]
*/
void getDisplayLongAmount(long amount, char* displayAmount) {
	char temp[25] = "\0";
	formatCurrencyAmount(amount, temp);

	getDisplayCurrency(displayAmount);
	strcat(displayAmount, temp);
}

/**
* @Brief get amount formatted with currency symbol to display on screen
* @param bcdAmount [in]
* @param displayAmount [out]
*/
void getDisplayAmount(char* bcdAmount, char* displayAmount) {
	long amt = atol(bcdAmount);

	getDisplayLongAmount(amt, displayAmount);
}


/**
* @Brief get the current display currency defined in params
* @param currency [out]
*/
void getDisplayCurrency(char* currency) {
	CLEAR(currency, '\0');
	sprintf(currency, "%s ", glPosParams.currency.szName);
}



int GetAmountNew(char* title, char amount[12 + 1], uint amtType) {
	int		iRet;

	while (1)
	{
		iRet = InputTransactionAmount(title, amtType, amount);
		if (iRet != 0)
		{
			return GUI_ERR_USERCANCELLED;
		}

		if (!ValidBigAmount(amount))
		{
			// Modified by Kim_LinHB 2014-8-12 v1.01.0003 bug511
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(title, gl_stTitleAttr, _T("Limit Exceeded"), gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);
			continue;
		}


		if (ConfirmAmount(NULL, amount))
		{
			break;
		}
	}

	return GUI_OK;
}

int GetAccountType(char* title, int* acctType) {
	GUI_MENU menu;
	GUI_MENUITEM menuItems[] = {
		{ "Default", DEFAULT, true, NULL },
		{ "Savings", SAVINGS, true, NULL },
		{ "Current", CURRENT, true, NULL },
		{ "Credit", CREDIT, true, NULL },
		{ "Universal", UNIVERSAL, true, NULL },
		{ "Investment", INVESTMENT, true, NULL },
		{ "\0", -1, false, NULL }
	};

	Gui_BindMenu(title, gl_stTitleAttr, gl_stLeftAttr, menuItems, &menu);
	Gui_ClearScr();
	int res = Gui_ShowMenuListWithoutButtons(&menu, GUI_MENU_DIRECT_RETURN, -1, acctType);
	if (GUI_OK != res) {
		return GUI_ERR_USERCANCELLED;
	}

	return GUI_OK;
}

void showCommError(int ret) {
	COMM_ERR_MSG comErr = { 0 };
	CommGetErrMsg(ret, &comErr);
	DispErrMsg(comErr.szMsg, NULL, 10, DERR_BEEP);
	CommOnHook(TRUE);
}

void getRRN(char rrn[12 + 1]) {
	CLEAR_STRING(rrn, 12 + 1);
	char temp[12 + 1] = "\0";
	char tempRrn[12 + 1] = "\0";
	uchar tempTime[6] = { 0 };
	unsigned long sequence = glProcInfo.stTranLog.ulSTAN + glPosParams.batchNo;

	GetTime(tempTime);
	PubBcd2Asc0(tempTime, 6, temp);
	sequence += PubTime2Long(temp);

	sprintf(tempRrn,"%012ld", sequence);
	logd(("Initial RRN: %s", tempRrn));

	CLEAR_STRING(temp, sizeof(temp));
	generateSequence(12, temp);
	logd(("Random temp: %s", temp));

	PubAscAdd(tempRrn, temp, 12, tempRrn);
	
	strncpy(rrn, tempRrn, 12);
	logd(("Final RRN: %s", rrn));
}


static int genRandom(unsigned char *buf, int len)
{
	srand(rand());
	char abRandom8[8];
	int i = 0;

	while (len > 0)
	{
		PciGetRandom(abRandom8);
		memcpy(buf, abRandom8+(rand() % 7), 1);

		len--;
		buf++;
	}

	return 0;
}




/**
*
* @param tranType [In]
* @return title, NULL
*/
const char* getTransactionTitle(int tranType) {
	switch (tranType) {
		//EFT
	case PURCHASE:
		return "PURCHASE";
	case CASH_ADVANCE:
		return "CASH ADVANCE";
	case PURCHASE_CASH:
		return "CASH";
	case PURCHASE_WITH_CASH_BACK:
		return "PURCHASE WITH CASHBACK";
	case PURCHASE_WITH_ADDITIONAL_DATA:
		return "PURCHASE WITH ADDITIONAL DATA";
	case REVERSAL:
		return "REVERSAL";
	case REFUND:
		return "REFUND";
	case POS_PRE_AUTHORIZATION:
		return "PRE-AUTHORIZATION";
	case POS_PRE_AUTH_COMPLETION:
		return "SALES COMPLETION";
	case BALANCE:
		return "BALANCE ENQUIRY";
	case PIN_CHANGE:
		return "PIN CHANGE";
	case MINI_STATEMENT:
		return "MINI STATEMENT";
	case DEPOSIT:
		return "DEPOSIT";
	case TRANSFER:
		return "TRANSFER";

	case COMM_SETTINGS:
		return "COMM SETTINGS";
	case TERMINAL_NIBSS_KEY_EXCHANGE:
		return "KEY EXCHANGE";
	case TERMINAL_NIBSS_PARAM_DOWNLOAD:
		return "PARAMETER DOWNLOAD";
	case TERMINAL_ID_CONFIG:
		return "MERCHANT CONFIG";
	case TMS_CONN_CONFIG:
		return "TMS COMM CONFIG";
	case TMS_MENU_DOWNLOAD:
		return "TMS MENU DOWNLOAD";
	case TERMINAL_MANUAL_UPDATE:
		return "MANUAL UPDATE";
	case PAYXPRESS:
		return "PAYXPRESS";
	case SPIDER_WEB:
		return "SPIDERWEB";
	case COLLECTIONS:
		return "KADIRS COLLECTIONS";
	case CASH_MOP_UP:
		return "CASH MOP-UP";
	case WALLET_MENU:
		return "WALLET";
	case PHED:
		return "PHED";
	case GATEWAY:
		return "OGUN COLLECTIONS";
	case PAYATTITUDE:
		return "PAYATTITUDE";
	default:
		return "\0";
	}
}

/**
*
* @param accountType [in]
* @return Account Type String
*/
const char* getAccountTypeString(AccountType accountType) {
	switch (accountType) {
	case SAVINGS:
		return "SAVINGS";
	case CURRENT:
		return "CURRENT";
	case CREDIT:
		return "CREDIT";
	case UNIVERSAL:
		return "UNIVERSAL";
	case INVESTMENT:
		return "INVESTMENT";
	default:
		return "DEFAULT";
	}
}

void maskTrack2Data(char* track2Data) {
	if (strlen(track2Data) < 32)
		return;

	int x = 7;

	for (; x < 12; x++) {
		track2Data[x] = '*';
	}
}

short maskPan(char * outPan, const char * inPan, char charToUser)
{

	//@Author: Ajani Opeyemi Sunda.
	//updated mask PAN to show only the first 6 digits
	int len = 0;

	char binPart[10] = { '\0' };
	char surfix[7] = { '\0' };

	char buffer[21] = { '\0' };



	len = strlen(inPan);
	if (len < 10) return 0;

	strncpy(binPart, inPan, 6);
	sprintf(surfix, "%s", &inPan[len - 4]);


	//LOG_PRINTF(("Pan => '%s', Bin => '%s', Surfix => '%s'", inPan, binPart, surfix));
	//memset(buffer, charToUser, len - 10);
	memset(buffer, charToUser, len - 6);
	
	memset(outPan, '\0', strlen(outPan));

	//sprintf(outPan, "%s%s%s", binPart, buffer, surfix);
	sprintf(outPan, "%s%s", binPart, buffer);

	//LOG_PRINTF(("Out Pan => '%s'", outPan));
	//get_char();

	return 1;
}

#ifdef _WIN32
int strcasecmp(const char *s1, const char *s2) {
	return -1;
}
#endif // 

bool isEqual(const void* first, const void* second, size_t bytes) {
	return memcmp(first, second, bytes) == 0;
}

bool stringEqualExt(const char* first, const char* second, bool ignore_case) {
	return ignore_case ? strcasecmp(first, second) == 0 : strcmp(first, second) == 0;
}

bool stringEqual(const char* first, const char* second) {
	return stringEqualExt(first, second, false);
}


int dayOfWeek(int year, int month, int day) {
	if (month < 3)
	{
		day = day + year;
		year--;
	}
	else
	{
		day = day + year - 2;
	}

	int val = (23 * month / 9) + day + 4 + year / 4 - year / 100 + year / 400;
	return val % 7;
}

int resetCommCfg() {
	glCommCfg = glPosParams.commConfig;
	CommSetCfgParam(&glCommCfg);
	return 0;
}

int strstrpos(char* haystack, char* needle) {
	char* nextStr = strstr(haystack, needle);

	if (nextStr) {
		return nextStr - haystack;
	}

	return -1;
}