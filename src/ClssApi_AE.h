/*****************************************************/
/* ClssApi_AE.h                                      */
/* Define the Application Program Interface          */
/* of American Express for PAXMe Platform Readers    */
/* of PAX    */
/*****************************************************/

#ifndef _CLSS_LIB_AE_H
#define _CLSS_LIB_AE_H
#include <posapi.h>
#include "CL_Common.h"

#define AE_MAGMODE 0x01
#define AE_EMVMODE 0x02
#define AE_FULLONLINE    0x03
#define AE_PARTIALONLINE 0x04
#define AE_DELAYAUTH_PARTIALONLINE 0x05

#define OPT_YES 0x01
#define OPT_NO  0x00

typedef struct
{
	unsigned char FloorLimitCheck;   
    unsigned long FloorLimit;        
	unsigned char TACDenial[6];      
	unsigned char TACOnline[6];     
	unsigned char TACDefault[6];     
    unsigned char AcquierId[6];      
	unsigned char dDOL[256];         
	unsigned char tDOL[256];        
	unsigned char Version[3];        
	unsigned char ucAETermCap;
	unsigned char aucRFU[3];
}CLSS_AEAIDPARAM;


typedef struct  
{
	Clss_ReaderParam stReaderParam;
	unsigned char   ucTmSupOptTrans;        //Not used
	unsigned char   aucUNRange[2];          //Unpredictable Number Range
}Clss_ReaderParam_AE;


typedef struct
{
	unsigned char aucRspCode[2];
	unsigned char aucAuthCode[6];
	int nAuthCodeLen;
	unsigned char aucIAuthData[16];
	int nIAuthDataLen;
	unsigned char aucScript[300];
	int nScriptLen;
}ONLINE_PARAM;


typedef struct  
{
	unsigned char   aucTmTransCapa[4];        //Terminal Transaction Capabilities - EMV Tag ¡®9F6E¡¯
	unsigned char   ucDelayAuthFlag;		  //1: support Delayed Authorization; 0:not support Delayed Authorization
	unsigned char   aucRFU[27];
}Clss_AddReaderParam_AE;


//Parameters and Data Management Functions
int Clss_CoreInit_AE(void);
int Clss_ReadVerInfo_AE(char *paucVer);
int Clss_GetTLVData_AE(unsigned short usTag, unsigned char *pucDtOut, int *pnDtLen);
int Clss_SetTLVData_AE(unsigned short usTag, unsigned char *pucDtIn, int nDtLen);
int Clss_GetReaderParam_AE(Clss_ReaderParam_AE *ptParam);
int Clss_SetReaderParam_AE(Clss_ReaderParam_AE *ptParam);
int Clss_GetAddReaderParam_AE(Clss_AddReaderParam_AE *ptParam);
int Clss_SetAddReaderParam_AE(Clss_AddReaderParam_AE *ptParam);

int Clss_SetAEAidParam_AE(CLSS_AEAIDPARAM *ptParam);
int Clss_nGetTrackMapData_AE(unsigned char ucTrackFlg, unsigned char *pucDataOut, unsigned char *pucLenOut);
unsigned char Clss_GetOptimizeFlag_AE(void);

//Certification Authority (CA) Public Key  Management Functions
int Clss_AddCAPK_AE(EMV_CAPK  *ptCAPK);
void Clss_DelAllCAPK_AE(void);

//Terminal Issuer Public Key Certification Revocation List Management Functions
int  Clss_AddRevocList_AE(EMV_REVOCLIST *ptRevocList);
void Clss_DelAllRevocList_AE(void);

//Transaction Processing Functions
int Clss_SetFinalSelectData_AE(unsigned char *pucRspIn, int nRspLen);
int Clss_SetTransData_AE(Clss_TransParam *ptTransParam,Clss_PreProcInterInfo *ptInfo);
int Clss_Proctrans_AE(unsigned char *pucTransMode);
int Clss_ReadRecord_AE(unsigned char *pucOptimizeFlag);
int Clss_CardAuth_AE(void);
int Clss_StartTrans_AE(unsigned char ucSupFullOnline, unsigned char *pucAdviceFlag, unsigned char *pucOnlineFlag);
int Clss_CompleteTrans_AE(unsigned char ucOnlineResult,unsigned char ucOnlineMode,
						  ONLINE_PARAM *ptOnlineParam, unsigned char *pucAdviceFlag);
int Clss_GetCVMType_AE(unsigned char *pucType);

//Call-back Functions
int Clss_cSetCAPKandRevoc_AE(unsigned char ucIndex, unsigned char *pucRid);
unsigned char cPiccIsoCommand_AE(unsigned char cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv);

//Debug Functions
void Clss_SetDebug_AE(int nEnableFlag);
void Clss_GetICCStatus_AE(unsigned char *SWA, unsigned char *SWB);

#endif
