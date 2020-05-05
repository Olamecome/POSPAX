/*****************************************************/
/* emvlib.h                                          */
/* Define the Application Program Interface          */
/* of EMV L2 for PAX POS terminals     		         */
/*****************************************************/

#ifndef _EMV_LIB_H
#define _EMV_LIB_H

//Please open the Macro-definition for corresponding platform
#ifndef _PAXME_TERM
#define _PAXME_TERM
#endif

//#ifndef _LINUX_TERM	//Default platform
//#define _LINUX_TERM
//#endif

//#ifndef _ANDROID_TERM
//#define _ANDROID_TERM
//#endif

#ifdef _PAXME_TERM
#include <posapi.h>
#endif

#include "L2_Device.h"
#include "EmvIIApi.h"



#define _PCI_PINPAD_// for PCI mode POS such as SXX

//---------------------------------------------------------------------------
/******************************* Macros declaration ************************/
//---------------------------------------------------------------------------

#ifndef _CLSS_COMMON_H
#define MAX_REVOCLIST_NUM               30        //Maximum number of Issuer Public Key Certificate Recover List for EMV kernel
#define MAX_APP_NUM                     100       //Maximum number of application list.
#ifdef _PAXME_TERM
#define MAX_KEY_NUM                     64        //Maximum number of public key stored in CA public key list of kernel for None Prolin platform.
#else
#define MAX_KEY_NUM                     7         //Maximum number of public key stored in CA public key list of kernel for Prolin platform.
#endif
#endif  //_CLSS_COMMON_H                                         	

#define CLSS_SLOT      0xff
#define CONTACT_SLOT   0x00

#define EMV_NO_CBFUN                   -47       //Callback function has not been set into kernel

//--------------------Return code of cEMVPedVerifyCipherPin-----------------
#define EMV_PIN_OK                     0
#define EMV_NO_PIN_INPUT               -3805 //ERR_PED_NO_PIN_INPUT
#define EMV_PIN_INPUT_CANCEL           -3806 //ERR_PED_PIN_INPUT_CANCEL
#define EMV_WAIT_INTERVAL              -3807 //ERR_PED_WAIT_INTERVAL
#define EMV_INPUT_PIN_TIMEOUT          -3815 //ERR_PED_INPUT_PIN_TIMEOUT
#define EMV_NO_ICC                     -3816 //ERR_PED_NO_ICC
#define EMV_ICC_INIT_ERR               -3817 //ERR_PED_ICC_INIT_ERR

//---------------------------------------------------------------------------
/******************************* End ***************************************/
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/******************************* Structure declaration *********************/
//---------------------------------------------------------------------------
#ifdef WIN32
#pragma warning(disable:4103)
#pragma pack(1)
#endif

#ifndef _CLSS_COMMON_H
typedef struct {
	unsigned char RID[5];                 //Registered Application Provider Identifier
	unsigned char KeyID;                  //Key Index
	unsigned char HashInd;                //Hash arithmetic indicator
	unsigned char ArithInd;               //RSA arithmetic indicator
	unsigned char ModulLen;               //Length of Module
	unsigned char Modul[248];             //Module
	unsigned char ExponentLen;            //Length of exponent
	unsigned char Exponent[3];            //Exponent
	unsigned char ExpDate[3];             //Expiration Date (YYMMDD) 
	unsigned char CheckSum[20];           //Check Sum of Key
}EMV_CAPK;

typedef  struct 
{
	unsigned char ucRid[5];               // Registered Application Provider Identifier
	unsigned char ucIndex;                // Certification Authenticate Public Key Index.
	unsigned char ucCertSn[3];            // Issuer Certificate Serial Number.
}EMV_REVOCLIST;

//refer to EmvIIApi.h
// typedef struct CLSS_TRANSPARAM
// {
// 	unsigned long  ulAmntAuth;            // Authorize amount, for Cash back, the amount required to include the amount of ulAmntOther.
// 	unsigned long  ulAmntOther;           // Other amount 
// 	unsigned long  ulTransNo;             // Transaction Sequence Counter(4 BYTE)
// 	unsigned char  ucTransType;           // Transaction type'9C',00-Goods/Services 01-Cash 09-Cash back
// 	unsigned char  aucTransDate[4];       // Transaction Date YYMMDD
// 	unsigned char  aucTransTime[4];       // Transaction time HHMMSS
// }Clss_TransParam;
#endif //_CLSS_COMMON_H

typedef struct{
	unsigned char AppName[33];            //Local application name. The string ends with '\x00' and is 32 bytes in maximum.
	unsigned char AID[17];                //Application ID. 
	unsigned char AidLen;                 //the length of AID
	unsigned char SelFlag;                //Application selection flag (partial matching PART_MATCH or full matching FULL_MATCH)      
	unsigned char Priority;               //priority indicator
	unsigned char TargetPer;              //Target percent (0 每 99) (provided by acquirer)
	unsigned char MaxTargetPer;           //Max target percent(0 每 99) (provided by acquirer)
 	unsigned char FloorLimitCheck;        //Check the floor limit or not (1: check(default), 0 : not check)
	unsigned char RandTransSel;           //Perform random transaction selection or not (1: perform(default), 0 : not perform)
	unsigned char VelocityCheck;          //Perform velocity check or not (1 : perform(default), 0 not perform)
	unsigned long FloorLimit;             //Floor limit (provided by acquirer)
	unsigned long Threshold;              //Threshold (provided by acquirer)
 	unsigned char TACDenial[6];           //Terminal action code - denial
	unsigned char TACOnline[6];           //Terminal action code 每 online
	unsigned char TACDefault[6];          //Terminal action code 每 default
	unsigned char AcquierId[6];           //Acquirer identifier 
	unsigned char dDOL[256];              //terminal default DDOL
	unsigned char tDOL[256];              //terminal default TDOL
	unsigned char Version[3];             //application version
	unsigned char RiskManData[10];        //Risk management data
}EMV_APPLIST;

typedef struct{
	unsigned char aucAppPreName[17];      //Application preferred name, ending with '\0'.
	unsigned char aucAppLabel[17];        //Application label, end with '\0'.
	unsigned char aucIssDiscrData[244];   //Data in template "BFOC" or "73", in the format of length+value, 
	                                      //where 1st byte for length and other bytes for value.
	unsigned char aucAID[17];             //AID of ICC
	unsigned char ucAidLen;               //Length of AID of ICC
}APPLABEL_LIST;

typedef struct{
	unsigned char MerchName[256];         //Merchant name
	unsigned char MerchCateCode[2];       //Merchant catalog code
	unsigned char MerchId[15];            //Merchant identification
	unsigned char TermId[8];              //Terminal identification
	unsigned char TerminalType;           //Terminal type
	unsigned char Capability[3];          //Terminal capabilities
	unsigned char ExCapability[5];        //Additional terminal capabilities
	unsigned char TransCurrExp;           //Transaction currency exponent
	unsigned char ReferCurrExp;           //Reference currency exponent     
	unsigned char ReferCurrCode[2];       //Reference currency code
	unsigned char CountryCode[2];         //Terminal country code 
	unsigned char TransCurrCode[2];       //Transaction currency code 
	unsigned long ReferCurrCon;           //the conversion quotients between transaction currency 
	                                      //and reference currency (default : 1000)
	                                      //(the exchange rate of transaction currency to reference currency *1000)
	unsigned char TransType;              //Set current transaction type
	                                      //(EMV_CASH EMV_GOODS EMV_SERVICE EMV_GOODS&EMV_CASHBACK EMV_SERVICE&EMV_CASHBACK)
	unsigned char ForceOnline;            //Merchant force online (1 means always require online transaction)
	unsigned char GetDataPIN;             //Read the IC card PIN retry counter before verify the PIN or not 
	                                      //(1 : read(default), 0 : not read)
	unsigned char SurportPSESel;          //Support PSE selection mode or not (1:support(default), 0:not support)
}EMV_PARAM;

typedef struct  
{
	unsigned char ucUseTermAIPFlg;        //0-TRM is based on AIP of card(default),1-TRM is based on AIP of Terminal
	unsigned char aucTermAIP[2];          //The bit4 of byte1 decide whether force to perform TRM: "08 00"-Yes;"00 00"-No(default)
	unsigned char ucBypassAllFlg;         //whether bypass all other pin when one pin has been bypassed 1-Yes,0-No
}EMV_EXTMPARAM;

typedef struct 
{
	unsigned char  ucBypassPin;           // 0-Not supported,1-Supported(default)
	unsigned char  ucBatchCapture;        // 0-Online data capture,1-batch capture(default)
	void *pvoid;
}EMV_MCKPARAM;

typedef struct 
{
	unsigned char ucECTSIFlg;             // TSI value is exist or not
	unsigned char ucECTSIVal;             // TSI value
	unsigned char ucECTTLFlg;             // TTL value is exist or not
	unsigned long ulECTTLVal;             // TTL value
}EMV_TMECPARAM;

typedef struct{
	int MaxLen;                           //The maximum length for this tag
	unsigned short Tag;                   //Tag value
	unsigned short Attr;                  //The format of this data
	unsigned short usTemplate[2];         //The template which this tag belongs, fill zero if none.
	unsigned char ucSource;               //The source of data element(EMV_SRC_TM/EMV_SRC_ICC/EMV_SRC_ISS).
}ELEMENT_ATTR;


typedef struct{
    int  (*cEMVInputAmount)(unsigned long *AuthAmt, unsigned long *CashBackAmt);
    int  (*cEMVWaitAppSel)(int TryCnt, EMV_APPLIST List[], int AppNum);
    int  (*cEMVUnknowTLVData)(unsigned short Tag, unsigned char *dat, int len);
    int  (*cEMVSetParam)(void);
    int  (*cEMVGetHolderPwd)(int TryFlag, int RemainCnt, unsigned char *pin);
    void (*cEMVVerifyPINOK)(void);
    void (*cEMVAdviceProc)(void);
    int  (*cEMVOnlineProc)(unsigned char *RspCode, unsigned char *AuthCode, int *AuthCodeLen, unsigned char *IAuthData, int *IAuthDataLen, unsigned char *script, int *ScriptLen);
    int  (*cEMVReferProc)(void);
    int  (*cCertVerify)(void);
    int  (*cEMVVerifyPINfailed)(unsigned char *RFUbuff);
    int  (*cRFU2)(void);
}EMV_CALLBACK;

typedef struct{
    unsigned char aucAppPreName[17];      //Application Prefer Name
    unsigned char aucAppLabel[17];        //Application Label
    unsigned char aucIssDiscrData[244];   //tag 'BF0C', the first byte is the length.
    unsigned char aucAID[17];             //Card AID
    unsigned char ucAidLen;               //Card AID Length
    unsigned char ucPriority;             //Priority
    unsigned char aucAppName[33];         //The Local Application name
}EMV_CANDLIST;
//---------------------------------------------------------------------------
/******************************* End ***************************************/
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/****************************** Function declaration ***********************/
//---------------------------------------------------------------------------
//Parameter and Data Management
int  EMVCoreInit(void);
void EMVGetParameter(EMV_PARAM *tParam);
void EMVSetParameter(EMV_PARAM *tParam);
int  EMVGetTLVData(unsigned short Tag, unsigned char *DataOut, int *OutLen);
int  EMVSetTLVData(unsigned short Tag, unsigned char *DataIn, int DataLen);

int  EMVGetScriptResult(unsigned char *Result, int *RetLen);
int  EMVReadVerInfo(char *paucVer);
int  EMVAddIccTag(ELEMENT_ATTR tEleAttr[], int nAddNum);
void EMVSetScriptProcMethod(unsigned char ucScriptMethodIn);

int  EMVGetMCKParam(EMV_MCKPARAM *pMCKParam);
int  EMVSetMCKParam(EMV_MCKPARAM *pMCKParam);
void EMVSetConfigFlag(int nConfigflag);
int  EMVGetParamFlag(unsigned char ucParam, int *pnFlg);

//Terminal Application List Management
int  EMVAddApp(EMV_APPLIST *App);
int  EMVGetApp(int Index, EMV_APPLIST *App);
int  EMVDelApp(unsigned char *AID, int AidLen);
int  EMVDelAllApp(void);
int  EMVModFinalAppPara(EMV_APPLIST *ptEMV_APP);
int  EMVGetFinalAppPara(EMV_APPLIST *ptEMV_APP);
int  EMVGetLabelList(APPLABEL_LIST  *ptAppLabel, int *pnAppNum); 

//Certification Authority (CA) Public Key Management
int  EMVAddCAPK(EMV_CAPK *capk);
int  EMVGetCAPK(int Index, EMV_CAPK *capk);
int  EMVDelCAPK(unsigned char KeyID, unsigned char *RID);
int  EMVDelAllCAPK(void);
int  EMVCheckCAPK(unsigned char *KeyID, unsigned char *RID);

//Terminal Revoked Issuer Public Key Certification List Management
int  EMVAddRevocList(EMV_REVOCLIST *pRevocList);
int  EMVDelRevocList(unsigned char ucIndex, unsigned char *pucRID);
void EMVDelAllRevocList(void);

//Transaction Processing
int  EMVAppSelect(int Slot,unsigned long TransNo);
int  EMVReadAppData(void);
int  EMVCardAuth(void);

int  EMVSetAmount(unsigned char * szAuthAmount, unsigned char * szCashBackAmount);
int  EMVStartTrans(unsigned long ulAuthAmt, unsigned long ulCashBackAmt, 
				   unsigned char *pACType); 
int  EMVCompleteTrans(int nCommuStatus, unsigned char *paucScript, 
					  int *pnScriptLen, unsigned char *pACType); 

//Read Transaction log (for PBOC2.0)
int  EMVAppSelectForLog(int Slot, unsigned char ucFlg);
int  ReadLogRecord(int RecordNo);
int  GetLogItem(unsigned short Tag, unsigned char *TagData, int *TagLen);

// For CLSS PBOC
void EMVInitTLVData(void);
int  EMVSwitchClss(Clss_TransParam *ptTransParam,unsigned char *pucSelData, int nSelLen, 
				   unsigned char *pucGPOData, int nGPOLen);
int  EMVSetTmECParam(EMV_TMECPARAM *tParam);
int  EMVGetCardECBalance(unsigned long *plBalance);
//Read Loading Log (PBOC3.0)
int  EMVReadSingleLoadLog(int nRecordNoIn);
int  EMVGetSingleLoadLogItem(unsigned short usTagIn, unsigned char *paucDataOut, int *pnLenOut);
int  EMVReadAllLoadLogs(unsigned char *paucLogDataOut, int *pnLenOut);
int  EMVGetLogData(unsigned char *paucLogDataOut, int *pnLenOut);
int  EMVClearTransLog(void);

//Callback Functions
int  cEMVWaitAppSel(int TryCnt, EMV_APPLIST List[], int AppNum);  
int  cEMVInputAmount(unsigned long *AuthAmt, unsigned long *CashBackAmt);
int  cEMVGetHolderPwd(int TryFlag, int RemainCnt, unsigned char *pin);
void cEMVAdviceProc(void);
void cEMVVerifyPINOK(void);
int  cEMVUnknowTLVData(unsigned short Tag, unsigned char *dat, int len);
int  cCertVerify(void);//for PBOC
int  cEMVSetParam(void);


#if !(defined (_PAXME_TERM) || defined (_ANDROID_TERM))
int cEMVPedVerifyPlainPin (unsigned char IccSlot, unsigned char *ExpPinLenIn, unsigned char *IccRespOut, unsigned char Mode, unsigned long TimeoutMs);
int cEMVPedVerifyCipherPin (unsigned char IccSlot, unsigned char *ExpPinLenIn, RSA_PINKEY_L2 *RsaPinKeyIn, unsigned char *IccRespOut, unsigned char Mode, unsigned long TimeoutMs);
unsigned char cEMVIccIsoCommand(unsigned char ucslot, APDU_SEND_L2 *tApduSend, APDU_RESP_L2 *tApduRecv);
#endif

int  EMVGetVerifyICCStatus(unsigned char *pucSWA, unsigned char *pucSWB);
int  EMVGetDebugInfo(int nExpAssistInfoLen, unsigned char *paucAssistInfo, int *pnErrorCode);
int  EMVGetCandListSWAB(unsigned char aucCandListSWAB[][2], int *pnAppNum);
int  EMVGetExtendFunc(int nFlag, int nExpDataOutLen, unsigned char *paucDataOut, int *pnActualDataOutLen);
int  EMVSetExtendFunc(int nFlag,unsigned char *paucDataIn, int nDataInLen);
#ifdef _PAXME_TERM
int  EMVCoreVersion(void);
int  EMVProcTrans(void);

int  cEMVReferProc(void);
int  cEMVOnlineProc(unsigned char *RspCode, unsigned char *AuthCode, int *AuthCodeLen, 
					unsigned char *IAuthData, int *IAuthDataLen, unsigned char *Script, int *ScriptLen);//ok
int EMVSetPCIModeParam(unsigned char ucPciMode, unsigned char *pucExpectPinLen,unsigned long ulTimeoutMs);
int EMVClearTransLog(void);
#endif
int EMVSetCallback(EMV_CALLBACK *pstCallback);

int EMVGetCandList(EMV_CANDLIST *patCandList, int *pnCandListNum);
int EMVSetCandList(EMV_CANDLIST *patCandList, int nCandListNum);
//---------------------------------------------------------------------------
/******************************* End ***************************************/
//---------------------------------------------------------------------------
#endif //_EMV_LIB_H

