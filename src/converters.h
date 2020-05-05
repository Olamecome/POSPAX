/*
 * converters.h
 *
 *  Created on: Aug 14, 2018
 *      Author: ayodeji.bamitale
 */

#ifndef INC_CONVERTERS_H_
#define INC_CONVERTERS_H_
#include "global.h"

/**
 *
 * @param msg
 * @param sessionkey
 * @param hash
 * @return
 */
int calculateSHA256Digest(char* msg, char* sessionkey, char *hash);

/**
 *
 * @param msg
 * @param keyBin
 * @param hash
 * @return
 */
int calculateSHA256DigestWithKeyBytes(char* msg, unsigned char keyBin[16] , char *hash);



/**
 *
 * @param key
 * @param src
 * @param inlen
 * @param dest
 * @return APP_FAIL, APP_SUCC
 */
int des3EcbEncrypt(uchar key[16], uchar* src, int inlen, uchar* dest);


/**
 *
 * @param key
 * @param srcBytes
 * @param inlen
 * @param destBytes
 * @return APP_FAIL, APP_SUCC
 */
int des3EcbDecrypt(uchar key[16], uchar* srcBytes, int inlen, uchar* destBytes);


/**
 *
 * @param cmp1 [in] Key component 1
 * @param cmp2 [in] Key component 2
 * @param inlen [in] Length in bytes
 * @param outKey [out] Combined key
 * @return
 */
int xor(const uchar* cmp1,const uchar* cmp2, int inlen, uchar* outKey);


/**
 * @brief verify that the key matches the key check value
 * @param key [in] Hexstring representation of the key
 * @param kcv [in] Hexstring representation of the key check value
 * @return APP_SUCC, APP_FAIL
 */
int verifyKey(const char* key, const char* kcv);

#endif /* INC_CONVERTERS_H_ */
