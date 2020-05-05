/*
 * Logger.c
 *
 *  Created on: Jul 30, 2018
 *      Author: ayodeji.bamitale
 */

#include "Logger.h"

static int debugMode = MODE_NONE;
static int debugLevel = LEVEL_ALL;

/**
 *
 * @param mode
 * @param level
 * @param port
 */
void initLogger(DebugMode mode, DebugLevel level, DebugPort port){

	debugMode = mode;
	debugLevel = level;

	debugMode = MODE_PORT;
	port = PORT_COM1;

	if (debugMode == MODE_PORT) {
		setDebugPort(port);
	}
}

void logInfo(char* formatString, ...) {
	va_list ap;
	
	LOG_PRINTF((formatString, ap));
}


void logTrace(char* formatString, ...) {
	va_list ap;
	LOG_PRINTF((formatString, ap));
}


void logError(char* formatString, ...) {
	va_list ap;
	LOG_PRINTF((formatString, ap));
}
void logCritical(char* formatString, ...) {
	va_list ap;
	LOG_PRINTF((formatString, ap));
}
void logEmergency(char* formatString , ...) {
	va_list ap;
	LOG_PRINTF((formatString, ap));
}

/**
 *
 * @param tag
 * @param hexBytes
 * @param len
 */
void logHexString(char*tag, const void* hexBytes, int len){
	LOG_HEX_PRINTF(tag, hexBytes, len);
}


void exportLogFile(){
	//PubExportDebugFile();
}

void closeLogger() {
	//PubSetDebugMode(MODE_NONE);
}
