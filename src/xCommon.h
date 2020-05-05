/*
 * xCommon.h
 *
 *  Created on: Aug 8, 2018
 *      Author: ayodeji.bamitale
 */

#ifndef INC_XCOMMON_H_
#define INC_XCOMMON_H_

#include "xPrompt.h"
#include "xMenu.h"

/**
 *
 * @param menu	Menu
 * @param title	char*
 * @param trantype TransactionType
 */
void initMenu(Menu* menu, const char* title, TransactionType trantype);

/**
 *
 * @param prompt
 * @param title
 * @param key
 */
void initPrompt(Prompt* prompt, const char* title,const  char* key);


/**
 *
 * @param accountType [out]
 */
void getDefaultAccountTypePrompt(Prompt* accountType);

/**
 *
 * @param amount [in]
 * @param title [in]
 */
void getDefaultAmountPrompt(Prompt* amount, char* title) ;


/**
 *
 * @param prompt [out]
 * @param title
 * @param content @brief '|' separated item list string
 */
void getListItemPrompt(Prompt* prompt, const char* title, char* content);

/**
 *
 * @param prompt [in out]
 * @param title
 * @param key
 */
void getNumberPrompt(Prompt* prompt, const char* title, char* key);


/**
 *
 * @param prompt [out]
 * @param title [in]
 * @param content [in]
 */
void getIPAddressPrompt(Prompt* prompt, const char* title, char* content);


/**
 *
 * @param prompt [out]
 * @param title  [in]
 * @param content [in]
 */
void getIPPortPrompt(Prompt* prompt, const char* title, char* content);


/**
 *
 * @param prompt [out]
 * @param title  [in]
 * @param content [in]
 */
void getTextPrompt(Prompt* prompt, const char* title, char* content);


/**
 *
 * @param prompt [out]
 * @param title [in]
 * @param length [in]
 */
void getPasswordPrompt(Prompt* prompt, const char* title, int length);


const char* getPassword(int passwordType);

#endif /* INC_XCOMMON_H_ */
