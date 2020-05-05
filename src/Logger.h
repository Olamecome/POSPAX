/*
 * Logger.h
 *
 *  Created on: Jul 30, 2018
 *      Author: ayodeji.bamitale
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_
#include "global.h"
#include <posapi.h>
#include <posapi_all.h>
#include <posapi_s80.h>


typedef enum DebugLevel {
	LEVEL_NORMAL = 0, LEVEL_WARNING = 1, LEVEL_ALL = 2
} DebugLevel;

typedef enum DebugMode {
	MODE_NONE, MODE_PORT, MODE_FILE
}DebugMode;

typedef enum DebugPort {
	PORT_COM1 = COM1, PORT_COM2 = COM2, PORT_USB = COM_USB

} DebugPort;




/**
 *
 * @param mode
 * @param level
 * @param port
 */
void initLogger(DebugMode mode, DebugLevel level, DebugPort port);

/**
 *
 * @param formatString
 */
void logInfo(char* formatString, ...);

/**
 *
 * @param formatString
 */
void logTrace(char* formatString, ...);

/**
 *
 * @param formatString
 */
void logError(char* formatString, ...);

/**
 *
 * @param formatString
 */
void logCritical(char* formatString, ...);

/**
 *
 * @param formatString
 */
void logEmergency(char* formatString, ...);


/**
 *
 * @param tag
 * @param hexBytes
 * @param len
 */
void logHexString(char* tag, const void* hexBytes, int len);


void exportLogFile();

void closeLogger();




#endif /* INC_LOGGER_H_ */
