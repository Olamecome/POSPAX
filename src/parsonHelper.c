/*
 * MyCjsonHelper.c
 *
 *  Created on: Jul 30, 2018
 *      Author: ayodeji.bamitale
 */

#include <stdio.h>
#include "parsonHelper.h"
#include "Logger.h"


const char* getJsonError() {
	return "\0";
}

/**
 *
 * @param json Json object
 * @param key The key
 * @param outString The output string
 * @param len The expected length
 */
 char* getJsonString(Json json, const char* key, char* outString, size_t len) {


	if (!json_object_has_value(json, key)) {
		return NULL;
	}
	
	const char* temp = json_object_get_string(json, key);

	logTrace("%s : %s", key, temp);

	if (outString) {
		CLEAR_STRING(outString, len);
		strmcpy(outString, temp, len);
	}
	
	return outString != NULL ?  outString : temp;
}

 /**
 *
 * @param json Json object
 * @param key The key
 * @param outString The output string
 * @param len The expected length
 */
 char* getDotJsonString(Json json, const char* key, char* outString, size_t len) {


	 if (!json_object_dothas_value(json, key)) {
		 return NULL;
	 }

	 const char* temp = json_object_dotget_string(json, key);

	 logTrace("%s : %s", key, temp);

	 if (outString) {
		 CLEAR_STRING(outString, len);
		 strmcpy(outString, temp, len);
	 }

	 return outString != NULL ? outString : temp;
 }

/**
 *
 * @param json
 * @param key
 * @return
 */
int getJsonInt(Json json, const char* key) {

	if (json_object_has_value(json, key)) {
		return (int)json_object_get_number(json, key);
	}
	
	return 0;
}

/**
*
* @param json
* @param key
* @return
*/
bool getJsonBoolean(Json json, const char* key) {

	if (json_object_has_value(json, key)) {
		return json_object_get_boolean(json, key);
	}

	return FALSE;
}

/**
 *
 * @param json
 * @param key
 * @return
 */
double getJsonDouble(Json json, const char* key) {
	if (json_object_has_value(json, key)) {
		return json_object_get_number(json, key);
	}

	return 0;
}


/**
*
* @param json
* @param key
* @return
*/
int getDotJsonInt(Json json, const char* key) {

	if (json_object_dothas_value(json, key)) {
		return (int)json_object_dotget_number(json, key);
	}

	return 0;
}

/**
*
* @param json
* @param key
* @return
*/
bool getDotJsonBoolean(Json json, const char* key) {

	if (json_object_dothas_value(json, key)) {
		return json_object_dotget_boolean(json, key);
	}

	return FALSE;
}

/**
*
* @param json
* @param key
* @return
*/
double getDotJsonDouble(Json json, const char* key) {
	if (json_object_dothas_value(json, key)) {
		return json_object_dotget_number(json, key);
	}

	return 0;
}

