/*
 * hex.c
 *
 *  Created on: Aug 14, 2018
 *      Author: ayodeji.bamitale
 */

#include "converters.h"
#include "sha256.h"
#include "xDefs.h"
#include "Logger.h"



/**
 *
 * @param msg
 * @param sessionkey
 * @param hash
 * @return
 */
int calculateSHA256Digest(char* msg, char* sessionkey, char *hash) {

	sha256_context Context;
	unsigned char keyBin[BYTE_KEY_SIZE];
	unsigned char digest[BYTE_KEY_SIZE + BYTE_KEY_SIZE + 1];

	sha256_starts(&Context);
	memset(keyBin, 0,  lengthOf(keyBin));

	PubAsc2Bcd(sessionkey, strlen(sessionkey), keyBin);

	sha256_update(&Context, keyBin, BYTE_KEY_SIZE);
	sha256_update(&Context, (unsigned char *) msg, strlen(msg));
	sha256_finish(&Context, digest);

	PubBcd2Asc0(digest, BYTE_KEY_SIZE + BYTE_KEY_SIZE, hash);

	logTrace("hashdata %s \n", hash);

	return 0;
}

/**
 *
 * @param msg
 * @param keyBin
 * @param hash
 * @return
 */
int calculateSHA256DigestWithKeyBytes(char* msg, unsigned char keyBin[16] , char *hash) {

	sha256_context Context;
	unsigned char digest[BYTE_KEY_SIZE + BYTE_KEY_SIZE + 1];

	sha256_starts(&Context);

	sha256_update(&Context, keyBin, BYTE_KEY_SIZE);
	sha256_update(&Context, (unsigned char *) msg, strlen(msg));
	sha256_finish(&Context, digest);

	PubBcd2Asc0(digest, BYTE_KEY_SIZE + BYTE_KEY_SIZE, hash);
	logTrace("hashdata %s \n", hash);

	return 0;
}



/**
 *
 * @param cmp1 [in] Key component 1
 * @param cmp2 [in] Key component 2
 * @param inlen [in] Length in bytes
 * @param outKey [out] Combined key
 * @return
 */
int xor(const uchar* cmp1,const uchar* cmp2, int inlen, uchar* outKey) {
    int i = 0;

    if ( inlen <= 0) {
        return -1;
    }

    memset(outKey, '\0', sizeof(outKey));

    for (; i < inlen; i++) {
        outKey[i] = cmp1[i] ^ cmp2[i];
    }

    return 0;
}

