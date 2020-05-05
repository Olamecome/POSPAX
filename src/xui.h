/*
 * ui.h
 *
 *  Created on: Jul 27, 2018
 *      Author: ayodeji.bamitale
 */

#ifndef INC_UI_H_
#define INC_UI_H_


#include "global.h"
#include "debug.h"
#include "xDefs.h"
#include "xPrompt.h"
#include "xMenu.h"
#include "xCommon.h"

#define XPRESS_BG "Xpress_bg.png"
#define XPRESS_LOGO "Xpress_logo.png"
#define XPRESS_TINY_LOGO "tiny_logo.png"

#define ONE_SECOND 1
#define TEN_SECONDS  10
#define ONE_MINUTE 60
#define FIVE_MINUTES  ONE_MINUTE * 5

#define MAX_LINE_LENGTH 29

typedef struct DisplayRange {
	int row;
	int column;
} DisplayRange;


/**
 * Get screen rows and columns
 * @param range
 * @return APP_SUCC, APP_FAIL
 */
int getDisplayRange(DisplayRange* range);
int clearScreen();

int setBackgroundImage(char* imageFile);

/**
 *
 * @param title
 * @param description
 * @param timeOut
 * @param inputText
 * @param inputLen
 * @return
 */
int requestPassword(char* title, char* description, int timeOut, char* inputText, int* inputLen);

/**
 *
 * @param title
 * @param message
 * @param beepCount
 * @param timeOut
 * @return
 */
int showMessageDialog(char * title, const char* message, int beepCount, int timeOut);



/**
 *
 * @param title Nullable
 * @param szContent
 * @param timeout in seconds
 * @return
 */
int showMultiscreenMessage(char* title, char* szContent, int timeout);

/**
 *
 * @param message
 * @param timeOut
 * @return
 */
int showErrorDialog(const char* message, int timeOut);

/**
 *
 * @param timeOut
 * @return
 */
int requestOperatorPassword(int timeOut);

/**
 *
 * @param timeOut
 * @return
 */
int requestSupervisorPassword(int timeOut);

/**
 *
 * @param timeOut
 * @return
 */
int requestAdminPassword(int timeOut);


/**
 *
 * @param prompt
 * @return
 */
int showPrompt(Prompt* prompt);

/**
 *
 * @param prompt
 * @return
 */
int showPromptInputDialog(Prompt* prompt);

/**
 *
 * @param prompt
 * @return
 */
int showPasswordDialog(Prompt* prompt);

/**
 *
 * @param prompt
 * @return
 */
int showPinDialog(Prompt* prompt) ;

/**
 *
 * @param prompt
 * @return
 */
int showIpAddressDialog(Prompt* prompt);

/**
 *
 * @param prompt
 * @return
 */
int showAmountDialog(Prompt* prompt) ;

/**
 *
 * @param prompt
 * @param selectedItem
 * @return
 */
int showSelectDialog(Prompt* prompt);

/**
 *
 * @param title
 * @param message
 */
void showNonModalDialog(char* title, char* message);

void showCommsBlockingMessage();

/**
 *
 * @param title
 * @param caption
 * @param message
 * @param beep
 * @param timeout
 * @return APP_SUCC, APP_QUIT, APP_TIMEOUT
 */
int showConfirmDialog(char* title, char* caption, char* message, int beep, int timeout);

/**
 *
 * @param title
 * @param timeout
 * @param beep_count
 * @param format
 */
void showInfo(char* title,int timeout, int beep_count, char* format, ...);


extern int  GetIPAddress(const uchar *pszPrompts, uchar bAllowNull, uchar *pszIPAddress);

extern int  GetIPPort(const uchar *pszPrompts, uchar bAllowNull, uchar *pszPortNo);

#endif /* INC_UI_H_ */
