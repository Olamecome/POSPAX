/*
 * xCrypto.c
 *
 *  Created on: Aug 16, 2018
 *      Author: ayodeji.bamitale
 */


#include "global.h"
#include "Des.h"
#include "converters.h"
#include "Logger.h"
#include "xDefs.h"


const int ECB_BLOCK_SIZE = 8;

/**
 *
 * @param key
 * @param srcBytes
 * @param inlen
 * @param destBytes
 * @return APP_FAIL, APP_SUCC
 */
int des3EcbDecrypt(uchar key[16], uchar* srcBytes, int inlen, uchar* destBytes) {
	if (inlen <= 0) {
		return APP_FAIL;
	}

	des3_context ctx;
	memset(&ctx, '\0', sizeof(ctx));
	des3_set_2keys(&ctx, key);

	int index = 0;

	while (index < inlen) {
		des3_decrypt(&ctx, &srcBytes[index], &destBytes[index]);
		index += ECB_BLOCK_SIZE;
	}

	return APP_SUCC;
}

/**
 * @param key
 * @param srcBytes
 * @param inlen
 * @param destBytes
 * @return APP_FAIL, APP_SUCC
 */
int des3EcbEncrypt(uchar key[16], uchar* srcBytes, int inlen, uchar* destBytes) {
	if (inlen <= 0) {
		return APP_FAIL;
	}

	des3_context ctx;
	memset(&ctx, '\0', sizeof(ctx));
	des3_set_2keys(&ctx, key);

	int index = 0;

	while (index < inlen) {
		des3_encrypt(&ctx, &srcBytes[index], &destBytes[index]);
		index += ECB_BLOCK_SIZE;
	}

	return APP_SUCC;
}


/**
 * @brief verify that the key matches the key check value
 * @param key [in] Hexstring representation of the key
 * @param kcv [in] Hexstring representation of the key check value
 * @return APP_SUCC, APP_FAIL
 */
int verifyKey(const char* key, const char* kcv) {

	uchar keyBytes[16] = {NULL};
	uchar kcvBytes[8] = {NULL};

	int keylen = hex2bin(key, keyBytes, lengthOf(keyBytes));
	logHexString("Key bytes: ", keyBytes, keylen);

	int kcvlen = hex2bin(kcv, kcvBytes, lengthOf(kcvBytes));
	logHexString("Kcv bytes: ", kcvBytes, kcvlen);

	uchar testBytes[8] = {NULL};
	PubAsc2Bcd("000000", 6,testBytes);

	logHexString("Test bytes: ", testBytes, 3);
	uchar testKcv[8] = {NULL};

	if (0 != des3EcbEncrypt(keyBytes, testBytes, 3, testKcv)) {
		return -1;
	}
	logHexString("Test Kcv: ", testKcv, 3);

	return memcmp(kcvBytes, testKcv, 3) == 0 ?  APP_SUCC : APP_FAIL;
}


