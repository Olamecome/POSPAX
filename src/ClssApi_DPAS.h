/*****************************************************/
/* clss_api_dpas.h                                   */
/* Define the Application Program Interface          */
/* of Discover D-PAS for all PAX Readers             */
/*                                                   */
/*****************************************************/
#ifndef _CLSS_API_DPAS_H
#define _CLSS_API_DPAS_H
#include "CL_common.h"


// Parameter processing
int Clss_CoreInit_DPAS(void);
int Clss_ReadVerInfo_DPAS(char *paucVer); 

//TLV Data List
int  Clss_GetTLVDataList_DPAS(unsigned char *pucTagList, unsigned char ucTagListLen, unsigned int unExpectDataLen, 
                            unsigned char *pucDataOut, unsigned int *punActualDataOutLen);
int  Clss_SetTLVDataList_DPAS(unsigned char *pucTLVDatas, unsigned int unDataLen);

//CAPK
int  Clss_AddCAPK_DPAS(EMV_CAPK  *ptCAPK );
void Clss_DelAllCAPK_DPAS(void);

// Revocation list
int  Clss_AddRevocList_DPAS(EMV_REVOCLIST *ptRevocList);
void Clss_DelAllRevocList_DPAS(void);

//DPAS Transaction Processing
int Clss_SetFinalSelectData_DPAS(unsigned char *pucRspIn, int nRspLen);
int Clss_InitiateApp_DPAS(unsigned char *pucPathTypeOut); 
int Clss_ReadData_DPAS(void);
int Clss_TransProc_DPAS(unsigned char ucExceptFileFlg);

int Clss_IssuerUpdateProc_DPAS(int nCommuStatus, unsigned char *paucScript, int *pnScriptLen);

int Clss_GetTrackMapData_DPAS(unsigned char ucTrackFlg, unsigned char *pTrackData, int *pLen); 

//Get error code
int Clss_GetDebugInfo_DPAS(int nExpAssistInfoLen, unsigned char *paucAssistInfo, int *pnErrorCode);
#endif
