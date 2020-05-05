/*
 * MyCjsonHelper.h
 *
 *  Created on: Jul 30, 2018
 *      Author: ayodeji.bamitale
 */

#ifndef INC_MYCJSONHELPER_H_
#define INC_MYCJSONHELPER_H_


#include "parson.h"
#include "xDefs.h"


typedef JSON_Object* Json;
typedef JSON_Value* JsonValue;
typedef JSON_Array* JsonArray;



const char* getJsonError();

/**
 *
 * @param json Json object
 * @param key The key
 * @param outString The output string
 * @param len The expected length
 */
char* getJsonString(Json json, const char* key, char* outString, size_t len);

/**
*
* @param json Json object
* @param key The key
* @param outString The output string
* @param len The expected length
*/
char* getDotJsonString(Json json, const char* key, char* outString, size_t len);



/**
 *
 * @param json
 * @param key
 * @return
 */
int getJsonInt(Json json, const char* key);

/**
 *
 * @param json
 * @param key
 * @return
 */
double getJsonDouble(Json json, const char* key);

/**
*
* @param json
* @param key
* @return
*/
bool getJsonBoolean(Json json, const char* key);

/**
*
* @param json
* @param key
* @return
*/
int getDotJsonInt(Json json, const char* key);

/**
*
* @param json
* @param key
* @return
*/
bool getDotJsonBoolean(Json json, const char* key);
/**
*
* @param json
* @param key
* @return
*/
double getDotJsonDouble(Json json, const char* key);


#endif /* INC_MYCJSONHELPER_H_ */
