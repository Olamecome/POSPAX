/*****************************************************/
/* ClEntryApi.h                                      */
/* Define the Application Program Interface          */
/* of Entry Point for all PAX Readers                */
/* Created by Liuxl at July 30, 2009                 */
/*****************************************************/

#ifndef _CLSS_ENTRY_LIB_H
#define _CLSS_ENTRY_LIB_H


#include "CL_common.h"
#include "L2_Device.h"

/* Candidate list */
typedef struct CLSS_TERMCANDLIST_STRUCT
{      
	unsigned char   ucIccAidLen;     
	unsigned char   aucIccAID[17]; //4F 
	unsigned char   ucAppLabelLen;
	unsigned char   aucAppLabel[17];//50
	unsigned char   ucPreferNameLen;
	unsigned char   aucPreferName[17];//9F12
	unsigned char   ucPriority; //81
	unsigned char   ucKernType;
	unsigned char   ucTmAidLen;     
	unsigned char   aucTmAID[17]; 
	//The presence of both of the following data elements identifies the AID as relating to a U.S. debit or prepaid program:
	unsigned char   aucIssCountryCode[2];// Issuer Country Code (2 digit alpha) (tag '5F55') with a value of '5553' ("US")
	unsigned char   aucIssIDNum[3];// Issuer Identification Number (IIN) (tag '42')
	unsigned char   aucRFU[5];
}Clss_CandList; 

// Support Book B EntryPoint processing, candidate list with kernel ID length of 8 bytes
typedef struct CLSS_TERMCANDLISTEX_STRUCT
{
	unsigned char   ucIccAidLen;
	unsigned char   aucIccAID[17]; //4F
	unsigned char   ucAppLabelLen;
	unsigned char   aucAppLabel[17];//50
	unsigned char   ucPreferNameLen;
	unsigned char   aucPreferName[17];//9F12
	unsigned char   ucPriority; //81
	unsigned char   ucKernIDLen;
	unsigned char   aucKernelID[8];
	unsigned char   ucTmAidLen;
	unsigned char   aucTmAID[17];
	//The presence of both of the following data elements identifies the AID as relating to a U.S. debit or prepaid program:
	unsigned char   aucIssCountryCode[2];// Issuer Country Code (2 digit alpha) (tag '5F55') with a value of '5553' ("US")
	unsigned char   aucIssIDNum[3];// Issuer Identification Number (IIN) (tag '42')
	unsigned char   ucReqKernIDLen;	// for NovelPay
	unsigned char   aucReqKernelID[8];
	unsigned char   ucExtendedSelLen;	// for NovelPay
	unsigned char   aucExtendedSel[11];
	unsigned char   aucRFU[4];
}Clss_CandListEx;

//pre-processing parameters and extended selection parameter of each Combination
typedef struct Clss_Combination
{
	unsigned long ulTermFLmt;
	unsigned char aucRdClssTxnLmt[6];
	unsigned char aucRdCVMLmt[6];
	unsigned char aucRdClssFLmt[6];
	unsigned char aucAID[17];
	unsigned char ucAidLen;
	unsigned char ucSelFlg;
	unsigned char ucKernIDLen;
	unsigned char aucKernelID[8];
	unsigned char ucCrypto17Flg;
	unsigned char ucZeroAmtNoAllowed;
	unsigned char ucStatusCheckFlg;
	unsigned char aucReaderTTQ[4];
	unsigned char ucTermFLmtFlg;
	unsigned char ucRdClssTxnLmtFlg;
	unsigned char ucRdCVMLmtFlg;
	unsigned char ucRdClssFLmtFlg;
	unsigned char ucExSltSuptFlg; // Extended selection support flag
	unsigned char aucRFU[10];	// aucRFU[0] - dynamic exponent for status check // for NovelPay 20170801(1-0.10, 2-1.00, 0-default etc.)
}Clss_Combination;


int Clss_CoreInit_Entry(void);
int Clss_ReadVerInfo_Entry(char *paucVer);
//for paypass 3.0
int Clss_SetMCVersion_Entry(unsigned char ucVer);

//Not Support Book B EntryPoint Processing
int Clss_SetPreProcInfo_Entry(Clss_PreProcInfo *ptPreProcInfoIn);
void Clss_DelAllPreProcInfo(void);
int Clss_DelPreProcInfo_Entry(unsigned char  *pucAID, unsigned char  ucAidLen);
void Clss_DelAllAidList_Entry(void);
int Clss_DelAidList_Entry(unsigned char  *pucAID, unsigned char  ucAidLen);
int Clss_AddAidList_Entry(unsigned char *pucAID, unsigned char ucAidLen, unsigned char ucSelFlg, unsigned char ucKernType);
int Clss_GetCandList_Entry(Clss_CandList *patCandList, int *pnCandListNum);
int Clss_SetCandList_Entry(Clss_CandList *patCandList, int nCandListNum);
//end

//Support Book B EntryPoint Processing
int Clss_GetCandListEx_Entry(Clss_CandListEx *patCandList, int *pnCandListNum);
int Clss_SetCandListEx_Entry(Clss_CandListEx *patCandList, int nCandListNum);
int Clss_AddCombination_Entry(Clss_Combination *ptCombination);
int Clss_DelCombination_Entry(unsigned char *paucAID, unsigned char ucAidLen, unsigned char *paucKernID, unsigned char ucKernIDLen);
void Clss_DelAllCombination_Entry(void);
int Clss_GetPreProcInterFlgByAid_Entry(unsigned char * paucAid, unsigned char ucAidLen, unsigned char * paucKernelID, 
									   unsigned char ucKernIDLen, Clss_PreProcInterInfo *ptInfo);
int Clss_SetAmount_Entry(unsigned char *paucAuthAmt);
//end

int Clss_PreTransProc_Entry(Clss_TransParam *pstTransParam);
int Clss_AppSlt_Entry(int Slot, int ReadLogFlag);
int Clss_FinalSelect_Entry(unsigned char *pucKernType, unsigned char *pucDtOut, int *pnDtLen);
int Clss_GetFinalSelectData_Entry(unsigned char *paucDtOut, int *pnDtLen);
int Clss_GetPreProcInterFlg_Entry(Clss_PreProcInterInfo *ptInfo);
int Clss_DelCurCandApp_Entry(void);

int Clss_GetErrorCode_Entry(int *pnErrorCode);
int Clss_SetExtendFunction_Entry(unsigned char *paucExFunc);
int Clss_GetExtendFunction_Entry(int nIndex, unsigned char *paucExFunc, int nExFuncLen);

int clss_AppSelect_Entry_UnlockApp(Clss_TransParam *ptTransParam, ClssTmAidList *ptTermAid);

typedef int  (*LCP_ADDAPDUTOTRANSLOG_ENTRY)(APDU_SEND_L2*, APDU_RESP_L2*);
int Clss_SetCBFun_AddAPDUToTransLog_Entry(LCP_ADDAPDUTOTRANSLOG_ENTRY cClssAddAPDUToTransLog);

int Clss_GetDebugInfo_Entry(int nExpAssistInfoLen, unsigned char *paucAssistInfo, int *pnErrorCode);

#endif

