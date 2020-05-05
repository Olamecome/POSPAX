/*
 * Prompt.h
 *
 *  Created on: Jul 30, 2018
 *      Author: ayodeji.bamitale
 */

#ifndef INC_PROMPT_H_
#define INC_PROMPT_H_

#include "xdefs.h"

#define PROMPTS "prompts"
#define PROMPT_TOKEN_SEPARATOR "|"

typedef enum  {
	INPUT_TYPE_ALPHA = 0,
	INPUT_TYPE_NUMERIC,
	INPUT_TYPE_AMOUNT,
	INPUT_TYPE_PIN,
	INPUT_TYPE_PASSWORD,
	INPUT_TYPE_DECIMAL,
	INPUT_TYPE_SELECT,
	INPUT_TYPE_IP,
	INPUT_TYPE_DATE,
	INPUT_TYPE_TIME
} PromptInputType;

typedef enum {
	PROMPT_ID_DEFAULT = -1,
	PROMPT_ID_AMOUNT = 0,
	PROMPT_ID_ACCOUNT_TYPE,
	PROMPT_ID_ADDITIONAL_AMOUNT,
	PROMPT_ID_REFERENCE_NUMBER,

	PROMPT_ID_TERMINAL_ID,
	PROMPT_ID_MERCHANT_ID,
	PROMPT_ID_TMS_IP,
	PROMPT_ID_TMS_PORT,
	PROMPT_ID_TMS_APP_URL
} PromptId;




typedef struct {
	int id;
	char title[50];
	char key [50];
	char value[200];
	char hint [50];
	char vendorId[50];

	int inputLength;  //the length of input string.
	int minLength;
	int maxLength;
	int timeOutInSeconds;

	int selectionOption;  //In the case of INPUT_TYPE_SELECT, the selected option.

	PromptInputType inputType;

	bool shouldValidate;
	bool shouldConfirm;
	bool shouldPrint;
	bool enabled;

} Prompt;





#endif /* INC_PROMPT_H_ */
