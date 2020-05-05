/*
 * ui.c
 *
 *  Created on: Jul 27, 2018
 *      Author: ayodeji.bamitale
 */

#include <string.h>
#include "xui.h"
#include "Logger.h"
#include "xDefs.h"
#include "utils.h"


/**
 *
 * @param imageFile
 * @return
 */
int setBackgroundImage(char* imageFile) {
	clearScreen();
	return Gui_DrawImage(imageFile, 100, 100);
}

int clearScreen() {
	return Gui_ClearScr();
}

/**
 *
 * @param title
 * @param message
 * @param beepCount
 * @param timeOut
 * @return
 */
int showMessageDialog(char * title, const char* message, int beepCount, int timeOut) {
	clearScreen();
	if (beepCount) {
		PubBeep();
	}
	return Gui_ShowMsgBox(title, gl_stTitleAttr, message, gl_stCenterAttr, GUI_BUTTON_OK, timeOut, NULL);
}



/**
 *
 * @param title Nullable
 * @param szContent
 * @param timeout in seconds
 * @return
 */
int showMultiscreenMessage(char* title, char* szContent, int timeout) {
	GUI_TEXT_ATTR titleAttr = { 0 };
	titleAttr.eAlign = GUI_ALIGN_CENTER;
	titleAttr.eFontSize = GUI_FONT_BOLD;
	titleAttr.eStyle = GUI_FONT_NORMAL;

	GUI_PAGE page;
	GUI_PAGELINE pageLine[100] = { 0 };

	int len = strlen(szContent);
	int line_count = len / 30;
	int remainder = len % 30;
	line_count = MIN(100, line_count);

	int i = 0, pos = 0;
	for (; i < line_count; i++) {
		pageLine[i].stLineAttr = gl_stLeftAttr;
		memcpy(pageLine[i].szLine, szContent+pos, lengthOf(pageLine[i].szLine) - 1);
		pos += 30;
	}

	if (line_count < 100 && remainder > 0) {
		pageLine[line_count].stLineAttr = gl_stLeftAttr;
		memcpy(pageLine[line_count].szLine, szContent + pos, remainder);
		line_count++;
	}

	Gui_CreateInfoPage(title, titleAttr, pageLine, line_count,  &page);
	
	Gui_ClearScr();
	int nRet = Gui_ShowInfoPage(&page,  FALSE, timeout);
	return nRet;
}

/**
 *
 * @param message
 * @param timeOut
 * @return
 */
int showErrorDialog(const char* message, int timeOut) {
	PubBeepErr();
	clearScreen();
	Gui_ShowMsgBox("Error",
		gl_stCenterAttr,
		message,
		gl_stCenterAttrAlt, GUI_BUTTON_NONE, timeOut, NULL);
	return 0;
}

/**
 *
 * @param title
 * @param description
 * @param timeOut
 * @param inputText
 * @param inputLen
 * @return
 */
int requestPassword(char* title, char* description, int timeOut,
		char* inputText, int* inputLen) {

	GUI_INPUTBOX_ATTR inputAttr = { 0 };
	inputAttr.bSensitive = true;
	inputAttr.nMaxLen = 8;
	inputAttr.nMinLen = 4;
	inputAttr.eType = GUI_INPUT_MIX;
	inputAttr.bEchoMode = false;

	clearScreen();
	int ret = Gui_ShowInputBox(title, gl_stTitleAttr, description, gl_stLeftAttr, inputText, gl_stCenterAttr, &inputAttr, timeOut);

	if (ret != GUI_OK) {
		clearScreen();
	}
	return ret;
}

int baseXrequestPassword(MenuSecurity type, int timeOut) {
	char* title;
	char text[11] = "\0";
	int len = 0;

	char* description;
	switch (type) {
	case OPERATOR_PIN:
		title = "OPERATOR PASSWORD";
		description = "Enter operator password";
		break;
	case SUPERVISOR_PIN:
		title = "SUPERVISOR PASSWORD";
		description = "Enter supervisor password";
		break;
	case ADMIN_PIN:
		title = "ADMIN PASSWORD";
		description = "Enter admin password";
		break;
	default:
		title = "PASSWORD";
		description = "Enter password";
	}

	const char* pass = getPassword(type);

	clearScreen();
	ASSERT_RETURNCODE(requestPassword(title, description, timeOut, text, &len));

	return strcmp(text, pass) == 0 ? APP_SUCC : APP_FAIL;
}

/**
 *
 * @param timeOut
 * @return
 */
int requestOperatorPassword(int timeOut) {
	return baseXrequestPassword(OPERATOR_PIN, timeOut);
}

/**
 *
 * @param timeOut
 * @return
 */
int requestSupervisorPassword(int timeOut) {
	return baseXrequestPassword(SUPERVISOR_PIN, timeOut);
}

/**
 * @param timeOut
 * @return
 */
int requestAdminPassword(int timeOut) {
	return baseXrequestPassword(ADMIN_PIN, timeOut);
}

/**
 *
 * @param range
 * @return
 */
int getDisplayRange(DisplayRange* range) {

	int* row = &range->row;
	int* column = &range->column;

	range->column = Gui_GetScrHeight();
	range->row = Gui_GetScrWidth();
	return APP_SUCC;
}

/**
 *
 * @param prompt
 * @return
 */
int showPrompt(Prompt* prompt) {
	logTrace("showPrompt - %s", prompt->title);
	int ret;

	INPUT: switch (prompt->inputType) {
	case INPUT_TYPE_SELECT:
		ret = showSelectDialog(prompt);
		break;
	case INPUT_TYPE_PIN:
		ret = showPinDialog(prompt);
		break;
	case INPUT_TYPE_IP:
		ret = showIpAddressDialog(prompt);
		break;
	case INPUT_TYPE_AMOUNT:
		ret = showAmountDialog(prompt);
		break;
	default:
		ret = showPromptInputDialog(prompt);
	}

	ASSERT_RETURNCODE(ret)

	if (prompt->shouldConfirm) {
		char* confirmText;

		char amount[50] = { '\0' };

		switch (prompt->inputType) {
		case INPUT_TYPE_AMOUNT:
			logTrace("Enter amount: %s", prompt->value);
			getDisplayAmount(prompt->value, amount);
			confirmText = amount;
			break;
		default:
			PubTrimStr(prompt->value);
			confirmText = prompt->value;
		}

		ret = showConfirmDialog(prompt->title, "Confirm Input?", confirmText, 1,
				prompt->timeOutInSeconds);

		if (ret != APP_SUCC) {
			if (prompt->inputType == INPUT_TYPE_AMOUNT) {
				return APP_FAIL;
			}

			goto INPUT;
		}
	}

	return APP_SUCC;
}

/**
 *
 * @param prompt
 * @return
 * @li APP_FAIL    	Abnormal
 * @li APP_QUIT    	user cancel
 * @li APP_SUCC    	Success
 */
int showPromptInputDialog(Prompt* prompt) {
	clearScreen();

	GUI_INPUTBOX_ATTR inputAttr = { 0 };
	inputAttr.bSensitive = prompt->inputType  == INPUT_TYPE_PASSWORD || prompt->inputType == INPUT_TYPE_PIN;
	inputAttr.nMaxLen = prompt->maxLength;
	inputAttr.nMinLen = prompt->minLength;
	inputAttr.bEchoMode = !inputAttr.bSensitive;

	switch (prompt->inputType) {
		case INPUT_TYPE_NUMERIC:
		case INPUT_TYPE_DECIMAL:
		case INPUT_TYPE_IP:
			inputAttr.eType = GUI_INPUT_NUM;
			break;
		default:
			inputAttr.eType = GUI_INPUT_MIX;
	}

	clearScreen();
	int ret = Gui_ShowInputBox(prompt->title, gl_stTitleAttr, prompt->hint, gl_stLeftAttr,
		prompt->value, gl_stCenterAttr, &inputAttr, prompt->timeOutInSeconds);
	ASSERT_RETURNCODE(ret);
	prompt->inputLength = strlen(prompt->value);

	return GUI_OK;
}

/**
 *
 * @param prompt
 * @return
 * @li APP_FAIL    	Abnormal
 * @li APP_QUIT    	user cancel
 * @li APP_SUCC    	Success
 */
int showAmountDialog(Prompt* prompt) {
	clearScreen();

	GUI_INPUTBOX_ATTR inputAttr = { 0 };
	inputAttr.bSensitive = false;
	inputAttr.nMaxLen = prompt->maxLength;
	inputAttr.nMinLen = prompt->minLength;
	inputAttr.eType = GUI_INPUT_NUM;

	clearScreen();
	int ret = Gui_ShowInputBox(prompt->title, gl_stTitleAttr, prompt->hint, gl_stLeftAttr, 
		prompt->value, gl_stCenterAttr, &inputAttr, prompt->timeOutInSeconds);
	ASSERT_RETURNCODE(ret);
	prompt->inputLength = strlen(prompt->value);

	return GUI_OK;
}

/**
 *
 * @param prompt
 * @return
 */
int showPinDialog(Prompt* prompt) {
	clearScreen();

	GUI_INPUTBOX_ATTR inputAttr = { 0 };
	inputAttr.bSensitive = true;
	inputAttr.nMaxLen = prompt->maxLength;
	inputAttr.nMinLen = prompt->minLength;
	inputAttr.eType = GUI_INPUT_NUM;

	clearScreen();
	int ret = Gui_ShowInputBox(prompt->title, gl_stTitleAttr, prompt->hint, gl_stLeftAttr,
		prompt->value, gl_stCenterAttr, &inputAttr, prompt->timeOutInSeconds);

	ASSERT_RETURNCODE(ret);
	prompt->inputLength = strlen(prompt->value);

	return GUI_OK;
}

/**
 *
 * @param prompt
 * @return
 */
int showIpAddressDialog(Prompt* prompt) {
	clearScreen();

	if (GUI_OK == GetIPAddress(prompt->hint, prompt->minLength == 0, prompt->value)) {
		prompt->inputLength = strlen(prompt->value);
		return APP_SUCC;
	}


	return APP_CANCEL;
}

/*
 *
 * @param prompt
 * @return
 */
int showPasswordDialog(Prompt* prompt) {
	clearScreen();

	GUI_INPUTBOX_ATTR inputAttr = { 0 };
	inputAttr.bSensitive = true;
	inputAttr.nMaxLen = prompt->maxLength;
	inputAttr.nMinLen = prompt->minLength;
	inputAttr.eType = GUI_INPUT_MIX;

	clearScreen();
	return Gui_ShowInputBox(prompt->title, gl_stTitleAttr, 
		prompt->hint, gl_stLeftAttr, prompt->value, gl_stCenterAttr, &inputAttr, prompt->timeOutInSeconds);
}

/**
 *
 * @param prompt
 * @return
 *
 */
int showSelectDialog(Prompt* prompt) {
	clearScreen();

	char* tokens[100];
	int token_count = tokenize(prompt->value, PROMPT_TOKEN_SEPARATOR, tokens,
		lengthOf(tokens));

	if (token_count <= 0) {
		deepFreeArray((void**)tokens, token_count);
		return APP_FAIL;
	}

	//prompt->selectionOption++;

	GUI_MENUITEM* menuItems = calloc(token_count + 1, sizeof(GUI_MENUITEM));
	int i = 0;

	for (; i < token_count; i++) {
		menuItems[i].bVisible = true;
		menuItems[i].nValue = i;
		menuItems[i].vFunc = NULL;

		memset(menuItems[i].szText, '\0', lengthOf(menuItems[i].szText));
		strncpy(menuItems[i].szText, tokens[i], 29);
	}

	menuItems[i].bVisible = false;
	menuItems[i].nValue = -1;
	menuItems[i].vFunc = NULL;
	memset(menuItems[i].szText, '\0', lengthOf(menuItems[i].szText));
	
	GUI_MENU menu = { 0 };
	Gui_BindMenu(prompt->title, gl_stTitleAttr, gl_stLeftAttr, menuItems, &menu);

	clearScreen();
	SetCurrTitle(prompt->title);
	int ret = Gui_ShowMenuList(&menu, GUI_MENU_DIRECT_RETURN, prompt->timeOutInSeconds, &prompt->selectionOption);

	free(menuItems);
	deepFreeArray((void**)tokens, token_count);

	return ret;
}

/**
 *
 * @param title
 * @param message
 */
void showNonModalDialog(char* title, char* message) {

	clearScreen();
	Gui_ShowMsgBox( title ? title : GetCurrTitle(), 
		gl_stTitleAttr, 
		message,
		gl_stCenterAttrAlt, GUI_BUTTON_NONE, 0, NULL);
}

/**
*
* @param title
* @param timeout
* @param beep_count
* @param format
*/
void showInfo(char* title, int timeout, int beep_count, char* format, ...) {
	char message[1000] = "\0";

	va_list va;
	va_start(va, format);
	vsprintf(message, format, va);
	va_end(va);

	PubBeep(beep_count);

	clearScreen();
	Gui_ShowMsgBox(title ? title : GetCurrTitle(),
		gl_stCenterAttr,
		message,
		gl_stCenterAttrAlt, GUI_BUTTON_NONE, timeout, NULL);
}

void showCommsBlockingMessage() {
	showNonModalDialog(NULL, "Please wait");
}

/**
 *
 * @param title
 * @param caption
 * @param message
 * @param beep
 * @param timeout
 * @return APP_SUCC, APP_QUIT, APP_TIMEOUT
 */
int showConfirmDialog(char* title, char* caption, char* message, int beep,
		int timeout) {
	clearScreen();

	PubBeep(beep);
	int res = Gui_ShowMsgBox(caption, gl_stTitleAttr, message, gl_stCenterAttr, GUI_BUTTON_YandN, timeout, NULL);
	return res;
}


