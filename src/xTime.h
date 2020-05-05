#pragma once

#ifndef INC_XTIME_H_
#define INC_XTIME_H_

#include <stddef.h>

//typedef struct tm tm;


typedef struct IsoTime {
	char longDate_F7[10 + 1];
	char shortDate_F13[4 + 1];
	char time_F12[6 + 1];

	char fullDateTime[14 + 1];
	//tm* date;
} IsoTime;


void getIsoTime(IsoTime *isoTime);

/**
* @Brief written by Chibuzor
* @param input
* @param output
* @return
*/
char longTimeToStringTime(char input[15], char output[20]);


#endif /* INC_XTIME_H_ */

