/*****************************************************/
/*                                                   */
/* Define the Application Program Interface          */
/* of PURE for all PAX Readers                       */
/*                                                   */
/*****************************************************/
#ifndef _CLSS_API_PURE_H
#define _CLSS_API_PURE_H
#include "CL_common.h"

// Parameter processing
int Clss_CoreInit_PURE(void);
int Clss_ReadVerInfo_PURE(char *pucVer);

//TLV Data List
int  Clss_GetTLVDataList_PURE(unsigned char *pucTagList, unsigned char ucTagListLen, unsigned int unExpectDataLen, 
                            unsigned char *pucDataOut, unsigned int *punActualDataOutLen);
int  Clss_SetTLVDataList_PURE(unsigned char *pucTLVDatas, unsigned int unDataLen);

//CAPK
int  Clss_AddCAPK_PURE(EMV_CAPK  *ptCAPK );
void Clss_DelAllCAPK_PURE(void);

// Revocation list
int  Clss_AddRevocList_PURE(EMV_REVOCLIST *ptRevocList);
void Clss_DelAllRevocList_PURE(void);

//PURE Transaction Processing
int Clss_SetFinalSelectData_PURE(unsigned char *pucRspIn, int nRspLen);
int Clss_InitiateApp_PURE(Clss_PreProcInterInfo  * ptPreProcFlg); 
int Clss_ReadData_PURE(void);
int Clss_StartTrans_PURE(unsigned char ucExceptFileFlg);

int Clss_CardAuth_PURE(void);
int Clss_CompleteTrans_PURE(unsigned char *paucScript, int *pnScriptLen);
//Get error code
int Clss_GetDebugInfo_PURE(int nExpAssistInfoLen, unsigned char *paucAssistInfo, int *pnErrorCode);
#endif
