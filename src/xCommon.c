#include "global.h"
#include "xCommon.h"
#include "xdefs.h"
#include <string.h>


/**
 *
 * @param menu	Menu
 * @param title	char*
 * @param trantype TransactionType
 */
void initMenu(Menu* menu, const char* title, TransactionType trantype) {
	memset(menu, '\0', sizeof(Menu));

	strmcpy(menu->title, title, lengthOf(menu->title));
	strcpy(menu->serviceId, " ");

	menu->location = ADMIN_MENU;
	menu->security = NO_SECURITY;
	menu->readerType = NO_READER;
	menu->hostType = NO_HOST;
	menu->operationMode = OFFLINE;
	menu->transactionType = trantype;

	menu->timeOutInSeconds = DEFAULT_TIME_OUT;

	menu->promptLen = 0;
	memset(menu->prompts, '\0', sizeof(menu->prompts));
}

/**
 *
 * @param prompt
 * @param title
 * @param key
 */
void initPrompt(Prompt* prompt, const char* title,const  char* key) {
	memset(prompt, '\0', sizeof(Prompt));

	prompt->id = PROMPT_ID_DEFAULT;

	strmcpy(prompt->title, title, lengthOf(prompt->title));

	if (key != NULL) {
		strmcpy(prompt->key, key, lengthOf(prompt->key));
	}

//	strcpy(prompt->value, " ");
//	strcpy(prompt->hint, " ");
	strcpy(prompt->vendorId, " ");

	prompt->inputLength = lengthOf(prompt->value);
	prompt->minLength = 0;
	prompt->maxLength = prompt->inputLength - 1;
	prompt->timeOutInSeconds = DEFAULT_TIME_OUT;

	prompt->inputType = INPUT_TYPE_ALPHA;

	prompt->shouldConfirm = false;
	prompt->shouldPrint = true;
	prompt->shouldValidate = false;
	prompt->enabled = true;
}


void getDefaultAccountTypePrompt(Prompt* accountType) {
	getListItemPrompt(accountType, "Account Type", "Default|Savings|Current|Credit");
	accountType->id = PROMPT_ID_ACCOUNT_TYPE;
	accountType->shouldConfirm = false;
	accountType->shouldValidate = false;
	accountType->shouldPrint = true;
}

void getDefaultAmountPrompt(Prompt* amount, char* title) {
	const char* _title = (title == NULL) ? "Amount": title;
	initPrompt(amount, _title, _title);
	amount->id = PROMPT_ID_AMOUNT;
	amount->inputType = INPUT_TYPE_AMOUNT;
	strcpy(amount->value, "");

	strcpy(amount->hint, "Enter amount(");
	strcat(amount->hint+strlen(amount->hint), glPosParams.currency.szName);
	strcat(amount->hint, "): ");

	amount->maxLength = 9;
	amount->minLength = 1;
	amount->shouldConfirm = false;
	amount->shouldPrint = true;
	amount->shouldValidate = false;
}

/**
 *
 * @param prompt [in out]
 * @param title
 * @param content @brief '|' separated item list string
 */
void getListItemPrompt(Prompt* prompt, const char* title, char* content) {
	initPrompt(prompt, title, title);
	prompt->inputType = INPUT_TYPE_SELECT;
	prompt->timeOutInSeconds = DEFAULT_TIME_OUT;
	strcpy(prompt->value, content);
}

/**
 *
 * @param prompt [in out]
 * @param title
 * @param key
 */
void getNumberPrompt(Prompt* prompt, const char* title, char* key) {
	if (key == NULL) {
		key = title;
	}
	initPrompt(prompt, title, key);
	prompt->inputType = INPUT_TYPE_NUMERIC;
	prompt->timeOutInSeconds = DEFAULT_TIME_OUT;
}

/**
 *
 * @param prompt [out]
 * @param title [in]
 * @param content [in]
 */
void getIPAddressPrompt(Prompt* prompt, const char* title, char* content) {

	initPrompt(prompt, title, NULL);
	prompt->inputType = INPUT_TYPE_IP;
	prompt->timeOutInSeconds = DEFAULT_TIME_OUT;
	prompt->minLength = 8;
	prompt->maxLength = 16;
	strcpy(prompt->hint, "Enter IP Address");
	strcpy(prompt->value, content);
}

/**
 *
 * @param prompt [out]
 * @param title  [in]
 * @param content [in]
 */
void getIPPortPrompt(Prompt* prompt, const char* title, char* content) {
	initPrompt(prompt, title, NULL);
	prompt->inputType = INPUT_TYPE_NUMERIC;
	prompt->minLength = 2;
	prompt->maxLength = 5;
	strcpy(prompt->hint, "Enter Port");
	strcpy(prompt->value, content);
}

/**
 *
 * @param prompt [out]
 * @param title  [in]
 * @param content [in]
 */
void getTextPrompt(Prompt* prompt, const char* title, char* content) {
	initPrompt(prompt, title, NULL);
	prompt->inputType = INPUT_TYPE_ALPHA;
	prompt->maxLength = lengthOf(prompt->value) -1;

	if (content != NULL) {
		strcpy(prompt->value, content);
	}
}


/**
 *
 * @param prompt [out]
 * @param title [in]
 * @param length [in]
 */
void getPasswordPrompt(Prompt* prompt, const char* title, int length) {
	initPrompt(prompt, title, NULL);
	prompt->inputType = INPUT_TYPE_PASSWORD;
	prompt->minLength = length;
	prompt->maxLength = length;

	strncpy(prompt->hint, "Enter password", lengthOf(prompt->hint));
}



const char* getPassword(int passwordType) {
	switch (passwordType) {
	case SUPERVISOR_PIN:
		return glPosParams.supervisorPin;
	case OPERATOR_PIN:
		return glPosParams.operatorPin;
	case ADMIN_PIN:
		return glPosParams.adminPass;
	case DEV_PIN:
		return DEV_PASS;
	default:
		return  glPosParams.adminPass;
	}

}
