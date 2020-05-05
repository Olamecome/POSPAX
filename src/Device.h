/*
 ============================================================================
 Name        : Device.h
 Author      : PAX
 Version     : 
 Copyright   : PAX Computer Technology(Shenzhen) CO., LTD
 Description : PAX POS Library
 ============================================================================
 */
 
 
#ifndef L2_DEVICE_H
#define L2_DEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _LINUX_TERM
#ifndef _APDU_DEFINE
#define _APDU_DEFINE
typedef struct
{
    unsigned char       Command[4];
    unsigned short      Lc;
    unsigned char       DataIn[512];
    unsigned short      Le;
}APDU_SEND;

typedef struct
{
    unsigned short      LenOut;
    unsigned char       DataOut[512];
    unsigned char       SWA;
    unsigned char       SWB;
}APDU_RESP;
#endif
#endif

//return code of arithmetic-------------------------------------------------
#define DEVICE_ARITH_OK     0
#define DEVICE_ARITH_ERROR  -1

//return code of PCD-------------------------------------------------
#define DEVICE_PICC_OK              0x00
#define DEVICE_PICC_USER_CANCEL     0x01
#define DEVICE_PICC_PROTOCOL_ERROR  0x02
#define DEVICE_PICC_TRANSMIT_ERROR  0x03
#define DEVICE_PICC_TIME_OUT_ERROR  0x04
#define DEVICE_PICC_OTHER_ERR       0xFF

void DEVICE_GetRand(unsigned char *buf, int len);
void DEVICE_TimerSet(unsigned char *pucTimerNo, unsigned short usTimeMS);
unsigned short DEVICE_TimerCheck(unsigned char TimerNo);
void DEVICE_DelayMs(unsigned short usTimeMS);

void DEVICE_DES(unsigned char *input, unsigned char *output, unsigned char *deskey, int mode);
int DEVICE_AES(unsigned char *Input, unsigned char *Output, unsigned char *AesKey, int KeyLen, int Mode);
void DEVICE_Hash(unsigned char* DataIn, unsigned int DataInLen, unsigned char* DataOut);
int DEVICE_RSARecover(unsigned char *pbyModule, unsigned int dwModuleLen,
					  unsigned char *pbyExp, unsigned int dwExpLen,
					  unsigned char *pbyDataIn, unsigned char *pbyDataOut);

unsigned char	DEVICE_SetControlParam (unsigned char *pucParam);
unsigned char   DEVICE_PiccIsoCommand(unsigned char ucCid, APDU_SEND *ptApduSend, APDU_RESP *ptApduRecv);
int  DEVICE_SetCancelKey(unsigned char ucKeyValue);
int  DEVICE_CheckCancelKey(unsigned char ucCheckFlag);

#ifdef __cplusplus
};
#endif

#endif /* L2_DEVICE_H */

