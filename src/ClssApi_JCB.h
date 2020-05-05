/*****************************************************/
/* clss_api_jcb.h                                    */
/* Define the Application Program Interface          */
/* of JSpeedy JCB for all PAX Readers                */
/*                                                   */
/*****************************************************/
#ifndef _CLSS_API_JCB_H
#define _CLSS_API_JCB_H
#include "CL_common.h"

// Parameter processing
int Clss_CoreInit_JCB(void);
int Clss_ReadVerInfo_JCB(char *paucVer); 

//TLV Data List
int  Clss_GetTLVDataList_JCB(unsigned char *pucTagList, unsigned char ucTagListLen, unsigned int unExpectDataLen, 
                            unsigned char *pucDataOut, unsigned int *punActualDataOutLen);
int  Clss_SetTLVDataList_JCB(unsigned char *pucTLVDatas, unsigned int unDataLen);

//CAPK
int  Clss_AddCAPK_JCB(EMV_CAPK  *ptCAPK );
void Clss_DelAllCAPK_JCB(void);

// Revocation list
int  Clss_AddRevocList_JCB(EMV_REVOCLIST *ptRevocList);
void Clss_DelAllRevocList_JCB(void);

//JCB Transaction Processing
int Clss_SetFinalSelectData_JCB(unsigned char *pucRspIn, int nRspLen);
int Clss_InitiateApp_JCB(unsigned char *pucPathTypeOut); 
int Clss_ReadData_JCB(void);
int Clss_TransProc_JCB(unsigned char ucExceptFileFlg);

int Clss_CardAuth_JCB(void);
int Clss_IssuerUpdateProc_JCB(unsigned char *paucScript, int *pnScriptLen);
//Get error code
int Clss_GetDebugInfo_JCB(int nExpAssistInfoLen, unsigned char *paucAssistInfo, int *pnErrorCode);
#endif
