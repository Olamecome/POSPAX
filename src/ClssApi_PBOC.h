/*****************************************************/
/* ClssApi_PBOC.h                                    */
/* Define the Application Program Interface          */
/* of qPBOC for all PAX Readers                      */
/* 																	                 */
/*****************************************************/

#ifndef _CLSS_QPBOC_LIB_H
#define _CLSS_QPBOC_LIB_H

#include "CL_common.h"

//Core initialization and data management.
int Clss_CoreInit_Pboc(void);
int Clss_ReadVerInfo_Pboc(char *paucVer);
int Clss_GetTLVData_Pboc(unsigned short Tag, unsigned char *DtOut, int *nDtLen);
int Clss_SetTLVData_Pboc(unsigned short usTag, unsigned char *pucDtIn, int nDtLen);
int Clss_SetQUICSFlag_Pboc(unsigned char ucQUCISFlag);

//Parameters
int Clss_SetReaderParam_Pboc(Clss_ReaderParam *ptParam);
int Clss_GetReaderParam_Pboc(Clss_ReaderParam *ptParam);
int Clss_SetPbocAidParam_Pboc(Clss_PbocAidParam *ptParam);
int Clss_SetExtendFunction_Pboc(unsigned char *paucParam,int nParamLen);

//CAPK 
int  Clss_AddCAPK_Pboc(EMV_CAPK  *capk );
int  Clss_GetCAPK_Pboc(int Index, EMV_CAPK  *capk );
int  Clss_DelCAPK_Pboc(unsigned char KeyID, unsigned char *RID);
void Clss_DelAllCAPK_Pboc(void);

// RevocList
int  Clss_AddRevocList_Pboc(EMV_REVOCLIST *pRevocList);
int  Clss_DelRevocList_Pboc(unsigned char ucIndex, unsigned char *pucRID);
void Clss_DelAllRevocList_Pboc(void);

//Transaction Process.
int Clss_SetFinalSelectData_Pboc(unsigned char *paucDtIn, int nDtLen);
int Clss_SetTransData_Pboc(Clss_TransParam *ptTransParam, Clss_PreProcInterInfo *ptInfo);
int Clss_Proctrans_Pboc(unsigned char *pucTransPath, unsigned char *pucACType);
int Clss_GetGPOData_Pboc(unsigned char *pucGPODt, int *nGPODtLen);
int Clss_ReSendLastCmd_Pboc(unsigned char *pucTransPath, unsigned char *pucACType);
int Clss_InitiateApp_Pboc(unsigned char *pucTransPath, unsigned char *pucACType);
int Clss_ReadData_Pboc(void);

//add for Torn Log
int Clss_TornSetConfig_Pboc(Clss_PbocTornConfig tTornConfig);
int Clss_TornProcessing_Pboc(unsigned char ucCurTornFlg,unsigned char *paucTornRst);
int Clss_ClearTornLog_Pboc(unsigned char ucClearFlag, unsigned char *pucDelTornFlag);
int Clss_GetTornFailFlag_Pboc(unsigned char *pucTornFailFlag);
int Clss_GetTornPAN_Pboc(unsigned char ucTornID,unsigned char *paucPAN, int *pnPANLen);

//Offline Data Authentication
int Clss_CardAuth_Pboc(unsigned char *pucACType, unsigned char *pucDDAFailFlg);

//Get CVM Type
int Clss_GetCvmType_Pboc(unsigned char *pucType);

//Get Data.
int Clss_GetDataCmd_Pboc(unsigned short usTag, unsigned char *DtOut, int *nDtLen);
// MSD track data
int Clss_nGetTrack1MapData_Pboc(unsigned char *pTrackData, int *pLen); 
int Clss_nGetTrack2MapData_Pboc(unsigned char *pTrackData, int *pLen); 

//Unlock Application
int Clss_Proctrans_Pboc_UnlockApp(unsigned char *pucTransPath);

// Call-back Functions
typedef int  (*LCP_CLSSCHECKEXCEPTIONFIILE_PBOC)(unsigned char *pucPAN, int nPANLen, unsigned char *pucPANSeq);
int Clss_SetCBFun_ClssCheckExceptionFile_Pboc(LCP_CLSSCHECKEXCEPTIONFIILE_PBOC cClssCheckExceptionFile_Pboc);

//Debug Functions
int Clss_GetDebugInfo_Pboc(int nExpAssistInfoLen, unsigned char *paucAssistInfo, int *pnErrorCode);
#endif
