#pragma once
#include "global.h"
#include "xCommon.h"
#include "stdbool.h"


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
int tokenize(char* string, char* separator, char** tokenArray, int length);
/**
*
* @param array
* @param length
*/
void deepFreeArray(void** array, int length);


/**
*
* @param length [in]
* @param pad [in]
* @param inStr [in]
* @param outStr [out]
*/
void padLeft(int length, char pad, char* inStr, char* outStr);


/**
*
* @param inStr [in]
* @param outStr [out]
*/
void toUpper(const char* inStr, char* outStr);

/**
*
* @param inStr [in]
* @param outStr [out]
*/
void toLower(const char* inStr, char* outStr);

/**
*
* @param longDate [in]
* @param formattedDate [out]
*/
void formatLongDate(char* longDate, char* formattedDate);

/**
*
* @param time [in]
* @param formattedTime [out]
*/
void formatShortTime(char* time, char* formattedTime);


/**
* @Brief apply currency-format to the amount
* @param amount
* @param formattedAmount
*/
void formatCurrencyAmount(long amount, char* formattedAmount);

/**
* @Brief reverse a string array
* @param array [in out]
*/
void strrev(char* array);


/**
* @Brief get the current display currency defined in params
* @param currency [out]
*/
void getDisplayCurrency(char* currency);


/**
* @Brief get amount formatted with currency symbol to display on screen
* @param bcdAmount [in]
* @param displayAmount [out]
*/
void getDisplayAmount(char* bcdAmount, char* displayAmount);

/**
* @Brief get amount formatted with currency symbol to display on screen
* @param amount [in]
* @param displayAmount [out]
*/
void getDisplayLongAmount(long amount, char* displayAmount);


/*
*@param beep_count[in]
*/
void PubBeep(beep_count);

int GetAmountNew(char* title, char amount[12 + 1], uint amtType);

void showCommError(int ret);

extern void generateSequence(int count, char* output);

void getRRN(char rrn[12 + 1]);


/**
*
* @param tranType [In]
* @return title, NULL
*/
const char* getTransactionTitle(int tranType);

/**
*
* @param accountType [in]
* @return Account Type String
*/
const char* getAccountTypeString(AccountType accountType);

void maskTrack2Data(char* track2Data);

short maskPan(char * outPan, const char * inPan, char charToUser);

bool isEqual(const void* first, const void* second, size_t bytes);

bool stringEqual(const char* first, const char* second);

bool stringEqualExt(const char* first, const char* second, bool ignore_case);

/*
@param year [in] : 1900 <= year
@Param month [in] :  1 <= month <= 12
@Param day [in] : 1 <= day <= 31
@return day of week 0 <= result <= 6
*/
int dayOfWeek(int year, int month, int day);