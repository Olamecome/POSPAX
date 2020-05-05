
/****************************************************************************
NAME
    util.h - 定义所有实用函数(针对应用封装基本库)

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.14      - created
****************************************************************************/

#ifndef _UITL_H
#define _UITL_H

#include "WlExt.h"

// amount type 
#define AMOUNT		0
#define RFDAMOUNT	1
#define ORGAMOUNT	2
#define CASHBACKAMOUNT	3

// options for DispErrMsg()
#define DERR_BEEP		0x0002

typedef struct _tagLangConfig{
	char	szDispName[16+1];	// language name
	char	szFileName[16+1];	// language translation file
	int	ucCharSet;			// character set
}LANG_CONFIG;

typedef struct _tagCURRENCY_CONFIG 
{
	uchar	szName[3+1];	        // short for currency, e.g. "HKD", "USD"
	uchar	sCurrencyCode[2];		// ISO-4217   currency code, e.g. NTD"\x09\x01"
    uchar   sCountryCode[2];        // ISO-3166-1 Country or area Code, e.g. Taiwan Province"\x01\x58"
	uchar	ucDecimal;		        // Decimal currency. e.g. JPY is 0, USD，HKD are 2, some are 3
	uchar	ucIgnoreDigit;	        // ignore digits at tail, when convert amount from ISO8583 field 4
}CURRENCY_CONFIG;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int CustomizeAppLibForArabiaLang(uchar bSetToArabia);

void InitTransInfo(void);
ulong GetNewTraceNo(void);
ulong GetNewBatchNo(void);
ulong GetNewInvoiceNo(void);
void PromptRemoveICC(void);
void EraseExpireCAPK(void);
void DispSwipeCard(uchar bShowLogo);
int  DetectCardEvent(uchar ucMode);

void DispBlockFunc(void);
void DispProcess(void);
void DispMessage(char* message);
void DispWait(void);
void DispDial(void);
void DispSend(void);
void DispReceive(void);
void DispPrinting(void);
void DispClearOk(void);
void DispWaitRspStatus(ushort uiLeftTime);
int  FunctionInput(void);
void SysHalt(void);
void SysHaltInfo(const void *pszDispInfo, ...);
void DispMagReadErr(void);
int  MatchCardTable(const uchar *pszPAN);
int  SelectTransCurrency(void);
int  ReadMagCardInfo(void);
void DispAccepted(void);
void DispErrMsg(const char *pFirstMsg, const char *pSecondMsg, short sTimeOutSec, ushort usOption);
void PrintOne(void);
uchar IsChipCardSvcCode(const uchar *pszTrack2);
int  ValidPanNo(const uchar *pszPanNo);
int  ValidCardExpiry(void);
void GetDateTime(uchar *pszDateTime);
int UpdateLocalTime(const uchar *pszNewYear, const uchar *pszNewDate, const uchar *pszNewTime);
int  ValidCard(void);
void GetCardHolderName(uchar *pszHolderName);
void GetEngTime(uchar *pszCurTime);
void Conv2EngTime(const uchar *pszDateTime, uchar *pszEngTime);
int  SwipeCardProc(uchar bCheckICC);
int  InsertCardProc(void);
void ConvIssuerName(const uchar *pszOrgName, uchar *pszOutName);
int  ConfirmPanInfo(void);
int  GetCard(uchar ucMode);
int  GetAmount(void);
void AmtConvToBit4Format(uchar *pszStringInOut, uchar ucIgnoreDigit);
void App_ConvAmountLocal(const uchar *pszIn, uchar *pszOut, uchar ucMisc);
int DisplayInputAmount(gui_callbacktype_t type, void *data, int *dataLen);
void App_ConvAmountTran(const uchar *pszIn, uchar *pszOut, uchar ucMisc);
uchar ValidAdjustAmount(const uchar *pszBaseAmt, uchar *pszTotalAmt);
uchar ValidBigAmount(const uchar *pszAmount);
uchar ConfirmAmount(const char *pszDesignation, uchar *pszAmount);
void GetDispAmount(uchar *pszAmount, uchar *pszDispBuff);
int  GetManualPanFromMsg(const void *pszUsrMsg);
int  ManualInputPan(uchar ucInitChar);
int  GetDescriptor(void);
int  GetAddlPrompt(void);
#define GETPIN_EMV		0x80
#define GETPIN_RETRY	0x01
int  GetPIN(uchar ucFlag);
int  GetMAC(uchar ucMode, const uchar *psData, ushort uiDataLen, uchar ucMacKeyID, uchar *psOutMAC);
void DispPPPedErrMsg(uchar ucErrCode);
void DispPciErrMsg(int iErrCode);
void MaskPan(const uchar *pszInPan, uchar *pszOutPan);
void Get8583ErrMsg(uchar bPackErr, int iErrCode, uchar *pszErrMsg);
int  GetPreAuthCode(void);
int  InputInvoiceNo(ulong *pulInvoiceNo);
int  GetStateText(ushort ucStatus, uchar *pszStaText); // Modified by Kim_LinHB 2014-8-8 v1.01.0002 bug508
void DispOperOk(const void *pszMsg);
int  SelectAcq(uchar bAllowSelAll, const uchar *pszTitle, uchar *pucAcqIndex);
int  SelectIssuer(uchar *pucIssuerIndex);
void ClearTotalInfo(void *pstTotalInfo);
int  DispTransTotal(uchar bShowVoidTrans);
//void AdjustLcd(void);
uchar IsNumStr(const char *pszStr);
//void GetEngTranLabel(uchar *pszTranTitle, uchar *pszEngLabel);
uchar GetTranAmountInfo(void *pTranLog);
void DispResult(int iErrCode);
int  NumOfReceipt(void);
//void ModifyTermCapForPIN(void);
void UpdateEMVTranType(void);
int FindCurrency(const uchar *pszCurrencyNameCode, CURRENCY_CONFIG *pstCurrency);
void SyncEmvCurrency(const uchar *psCountryCode, const uchar *psCurrencyCode, uchar ucDecimal);

int  UpdateTermInfo(void);
int  CheckSysFont(void);
int  EnumSysFonts(void);
int  SxxWriteKey(uchar ucSrcKeyIdx, const uchar *psKeyBCD, uchar ucKeyLen, uchar ucDstKeyId, uchar ucDstKeyType, const uchar *psKCV);

void SetOffBase(unsigned char (*Handle)());
uchar ChkOnBase(void);
uchar OffBaseDisplay(void);
uchar OffBaseCheckPrint(void);
uchar OffBaseCheckEcr(void);

// Added by Kim_LinHB 2014-6-21
void SetCurrTitle(const unsigned char *pszTitle);
const unsigned char *GetCurrTitle(void);
// Add End

// Added by Kim_LinHB 2014-08-22 v1.01.0004
void SplitIpAddress(const unsigned char *Ip, unsigned char sub[4]);
void MergeIpAddress(const unsigned char sub[4], unsigned char *Ip);

int GetLastKey();
void SetLastKey(int key);
int DelFilesbyPrefix(const char *prefix);
const char *GetCurSignPrefix(uchar ucAcqKey);
char HasE_Signature();
void DoE_Signature();

int InputTransactionAmount(char* title, uchar ucAmtType, uchar amount[12 + 1]);
const char* GetTerminalModel();

double absolute(double number);
uchar checkTerminalPrepStatus();

int get_wl_info(WlInfo_T *wl_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _UITL_H

// end of file
