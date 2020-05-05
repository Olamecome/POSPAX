/*
 * FileHandler.c
 *
 *  Created on: Jan 7, 2019
 *      Author: ayodeji.bamitale
 */


#include "file_handler.h"
#include "global.h"
#include "Logger.h"

/**
 *
 * @param fileName
 * @param data
 * @param data_size
 * @return >=0 - success - bytes read, -1 - fail
 */
int loadFileData(char* fileName, void* data, size_t  data_size) {
	int file = open(fileName, O_RDWR);
	if (file == -1) {
		logTrace("Could not load file");
		return -1;
	}

	int ret = read(file, data, data_size);
	close(file);


	return ret;
}


/**
 *
 * @param fileName
 * @param data
 * @param data_size
 * @return 0 - success, -1 - fail
 */
int saveFileData(char* fileName, void* data, size_t  data_size) {
	int file = open(fileName, O_CREATE | O_RDWR);
	if (file == -1) {
		logTrace("Could not write file");
		return -1;
	}

	int ret = write(file, data, data_size);
	if (ret == -1) {
		logTrace("Write error: %d", ret);
	}

	close(file);
	return 0;
}

int deleteFile(char* fileName) {
	return remove(fileName);
}

