/*
 * file_handler.h
 *
 *  Created on: Jan 7, 2019
 *      Author: ayodeji.bamitale
 */

#ifndef INC_FILE_HANDLER_H_
#define INC_FILE_HANDLER_H_

#include <stddef.h>


 /**
 *
 * @param fileName
 * @param data
 * @param data_size
 * @return >=0 - success - bytes read, -1 - fail
 */
int loadFileData(char* fileName, void* data, size_t  data_size);

/**
 *
 * @param fileName
 * @param data
 * @param data_size
 * @return 0 - success, -1 - fail
 */
int saveFileData(char* fileName, void* data, size_t  data_size);

int deleteFile(char* fileName);

#endif /* INC_FILE_HANDLER_H_ */
