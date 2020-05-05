#include "xTime.h"
#include "global.h"


void getIsoTime(IsoTime *isoTime) {
	memset(isoTime, 0, sizeof(IsoTime));
	GetDateTime(isoTime->fullDateTime);

	strncpy(isoTime->longDate_F7, isoTime->fullDateTime + 2, lengthOf(isoTime->longDate_F7)-1);
	strncpy(isoTime->shortDate_F13, isoTime->fullDateTime + 4, lengthOf(isoTime->shortDate_F13) - 1);
	strncpy(isoTime->time_F12, isoTime->fullDateTime + 8, lengthOf(isoTime->time_F12) - 1);
}

/**
* @Brief written by Chibuzor
* @param input
* @param output
* @return
*/
char longTimeToStringTime(char input[15], char output[20]) {

	char year[5];
	char month[3];
	char day[3];
	char hour[3];
	char min[3];
	char sec[3];
	strncpy(year, input, 4);
	year[4] = '\0';
	strncpy(month, input + 4, 2);
	month[2] = '\0';
	strncpy(day, input + 6, 2);
	day[2] = '\0';
	strncpy(hour, input + 8, 2);
	hour[2] = '\0';
	strncpy(min, input + 10, 2);
	min[2] = '\0';
	strncpy(sec, input + 12, 2);
	sec[2] = '\0';
	snprintf(output, 20, "%s-%s-%sT%s:%s:%s", year, month, day, hour, min, sec);

	return 0;
}
