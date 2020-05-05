/*****************************************************/
/* ClssApi_MC.h                                      */
/* Define the Application Program Interface          */
/* of MasterCard PayPass for all PAX Readers         */
/* Created by Zhou Jie at 2013.1.1                 */
/*****************************************************/

#ifndef _CLSS_LIB_MC_H
#define _CLSS_LIB_MC_H

#include "CL_common.h"
#include "L2_Device.h"

// the source of data elements
#define CLSS_SRC_ICC   0  //ICC
#define CLSS_SRC_TM    1  //terminal
#define CLSS_SRC_ISS   2  //issuer

#define CLSS_DATA_OCPS 0x01//Outcome Parameter Set
#define CLSS_DATA_DISD 0x02//Discretionary Data
#define CLSS_DATA_UIRD 0x04//User Interface Request Data
#define CLSS_DATA_ERRI 0x08//Error Indication

typedef struct{
	unsigned char aucPAN[10];
	unsigned char ucPANLen;
	unsigned char ucPANSeqFlg;
	unsigned char ucPANSeq;	
	unsigned char aucTornData[1024];//the Torn Record start with tag 'FF8101', length and value
	unsigned int unTornDataLen;	//the length of aucTornRecord
}CLSS_TORN_LOG_RECORD;//the format of Torn Record saved in kernel

// Proprietary Tag of parameter defined by this kernel
#define CLSS_PARAM_TIMER    0x01

// Parameter processing
int  Clss_CoreInit_MC(unsigned char ucDESupportFlag);
int  Clss_ReadVerInfo_MC(char *pacVer); 
int  Clss_SetTLVDataList_MC(unsigned char *pucTLVDatas, unsigned int unDataLen);
int  Clss_GetTLVDataList_MC(unsigned char *pucTagList, unsigned char ucTagListLen, 
                            unsigned int unExpectDataLen, unsigned char *pucDataOut, unsigned int *punActualDataOutLen);
int  Clss_SetTagPresent_MC(unsigned char *pucTag, unsigned char ucPresent);
int  Clss_SetParam_MC(unsigned char *paucTLVParam, unsigned int unParamLen);
//CAPK
int  Clss_AddCAPK_MC_MChip(EMV_CAPK  *ptCAPKey);
void Clss_DelAllCAPK_MC_MChip(void);

// Revocation list
int  Clss_AddRevocList_MC_MChip(EMV_REVOCLIST *ptRevocList);
int  Clss_DelRevocList_MC_MChip(unsigned char ucIndex, unsigned char *pucRID);
void Clss_DelAllRevocList_MC_MChip(void);

 //Transaction processing
int Clss_SetFinalSelectData_MC(unsigned char *pucRspIn, int nRspLen);
int Clss_InitiateApp_MC(void); 
int Clss_ReadData_MC(unsigned char *pucPathTypeOut);
// For Mag-Stripe only
int Clss_TransProc_MC_Mag(unsigned char *pucACTypeOut);
//end
// For MCHIP only
int Clss_TransProc_MC_MChip(unsigned char *pucACTypeOut);
int Clss_CleanTornLog_MC_MChip(unsigned char *pucCurrentDateTime, int nDateTimeLen, unsigned char ucCleanAllFlg);
int Clss_SetTornLog_MC_MChip(CLSS_TORN_LOG_RECORD *ptTornLog, int nTornNum);
int Clss_GetTornLog_MC_MChip(CLSS_TORN_LOG_RECORD *ptTornLog, int *pnTornNum, int *pnUpdatedFlg);
//end


//call back function(Paypass transaction used at lab test only now)
typedef int  (*LCP_SENDDEKDATA_MC)(unsigned char*, unsigned int);
int Clss_SetCBFun_SendDEKData_MC(LCP_SENDDEKDATA_MC cClssSendDEKData);

typedef int  (*LCP_RECEIVEDETDATA_MC)(unsigned char*, unsigned int*, unsigned char*);
int Clss_SetCBFun_ReceiveDETData_MC(LCP_RECEIVEDETDATA_MC cClssReceiveDETData);

typedef int  (*LCP_ADDAPDUTOTRANSLOG_MC)(APDU_SEND_L2*, APDU_RESP_L2*);
int Clss_SetCBFun_AddAPDUToTransLog_MC(LCP_ADDAPDUTOTRANSLOG_MC cClssAddAPDUToTransLog);

typedef int  (*LCP_SENDTRANSDATAOUTPUT_MC)(unsigned char);
int Clss_SetCBFun_SendTransDataOutput_MC(LCP_SENDTRANSDATAOUTPUT_MC cClssSendTransDataOutput);

//Debug interface
int  Clss_GetDebugInfo_MC(int nExpAssistInfoLen, unsigned char *paucAssistInfo, int *pnErrorCode);
int  Clss_ProcLmt_MC(void);

#endif

