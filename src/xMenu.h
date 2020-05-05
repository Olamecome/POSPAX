/*
 * Menu.h
 *
 *  Created on: Jul 30, 2018
 *      Author: ayodeji.bamitale
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

#include "xPrompt.h"



typedef enum Location {
	MAIN_MENU = 0, ADMIN_MENU
} MenuLocation;

typedef enum MenuSecurity {
	NO_SECURITY = 1, OPERATOR_PIN, SUPERVISOR_PIN, ADMIN_PIN, DEV_PIN
}MenuSecurity;


typedef struct MenuItem {
	char title[50];
	char serviceId[50];

	MenuLocation location;
	MenuSecurity security;
	ReaderType readerType;
	HostType hostType;
	OperationMode operationMode;
	TransactionType transactionType;

	int timeOutInSeconds;

	Prompt prompts[20];
	int promptLen;

	bool skipNotify;
	bool skipPrintIfApproved;
	bool skipPrintIfDeclined;
} Menu;

#endif /* INC_MENU_H_ */
