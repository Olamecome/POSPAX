#pragma once
#include "global.h"
enum {HTTP_POST, HTTP_GET};

#define DEFAULT_REQUEST_TIMEOUT 60L

typedef struct MemoryStruct {
	char *memory;
	size_t size;
}MemoryStruct;


/**
* @param httpMethod
* @param hostURL
* @param postData
* @param post_data_len
* @param headers
* @param header_len
* @param chunk [out] @brief remember to always call free(chunk.memory);
* @return
*/
int sendHttpRequest(uchar httpMethod, 
	const char* hostURL, const char* postData, size_t post_data_len, const char** headers, size_t header_len, MemoryStruct* chunk);

