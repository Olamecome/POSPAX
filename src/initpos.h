
/****************************************************************************
NAME
    initpos.h - ʵ���ն˲�������

DESCRIPTION
	1��֧�ִ�TMS��ProTims
	2��֧��ͬ�����첽ͨѶ��֧�ֱ��ء��������

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _INITPOS_H
#define _INITPOS_H


// terminal risk mask
#define TRM_FLOOR_CHECK			0x01	
#define TRM_RANDOM_TRAN_SEL		0x02
#define TRM_VELOCITY_CHECK		0x04
#define TRM_EXECPTION_FILE		0x08
#define TRM_SUPPORT_TAC			0x10

// app option
#define TAM_PSE_SELECTION		0x01
#define TAM_HOLDER_CONFIRM		0x02
#define TAM_BYPASS_PIN			0x04
#define TAM_GETDATA_FOR_PIN		0x08
#define TAM_HAVE_PIN_PAD		0x10
#define TAM_AMT_ENTER_PAD		0x20


// TMS EDC parameters
typedef struct _tagTMS_EDC_INFO
{
	uchar	ucDllTracking;		// reserved ?
	uchar	bClearBatch;		// 0-none, 1-clear former batch
	uchar	ucPrinterType;		// 0-Sprocket, 1-Thermal
	uchar	ucEcrSpeed;			// ECR speed, reserved ?
	uchar	sInitTime[6];		// YYMMDDhhmmss // 10
	uchar	ucDialOption;		// b4-referal dial
	uchar	sTermPwd[2];		// terminal password, packed
	uchar	sHelpTelNo[12];		// for help function
	
	// b7-confirm/enter total,
	// b6-business date format,0MMDDYY,1DDMMYY
	// b5-print time,b4-lodging,b3-user accounting,b2-tip
	// b1-display track info,0-amount dual entry 
	uchar	ucOption1;
	
	// b5-base total entry, b4-whole settle,
	// b3-business date prompt at settle
	// b2-print reference number, b1-use printer,
	// b0-track,0 track1,1 track2
	uchar	ucOption2;
	
	
	// b7-enable instalment
	// b6-inhibit manual parameter modification 
	// b5-default to auth,else pre-auth 
	// b4-cash processing 
	uchar	ucOption3;
	
	uchar	ucUnused1;			// 29 
	uchar	sMerchantAddr[46];	// line 2 and 3 on invoice // 75
	uchar	sMerchantName[23];	// merchant name, line 1 on invoice
	uchar	ucCurrencySymbol;
	uchar	ucTranAmtLen;			// BCD format
	uchar	ucDecimalPos;
	uchar	ucLanguage;		// 0-english,1-spanish,2-portuguese
	uchar	ucStlAmtLen;
	uchar	sUnused2[3];
	
	// b5-adjust password
	// b4-manual entry password
	// b3-settle password
	// b2-refund password
	// b1-void password
	// b0-keyboard locked
	uchar	ucPwdMask;		// 0: required pwd 1: not
	
	uchar	sAddlPrompt[20];	// for show and print
	uchar	sOfflineLimit[5];
	uchar	sCurrencyName[3];		// 130
	uchar	sVoidPwd[2];
	uchar	sRefundPwd[2];
	uchar	sSettlePwd[2];
	uchar	sAdjustPwd[2];
	uchar	sInitialSTAN[3];		// 146	
}TMS_EDC_INFO;

// TMS issuer parameters
typedef struct _tagTMS_ISSUER
{
	uchar	ucKey;				// id key
	uchar	sName[10];			// issuer name
	uchar	sRefTelNo[12];		// phone # for voice referral

	// sOption[0]:
		// b7-allow adjust, b6-descriptor required
		// b5-voice referral allowed, b4-allow offline
		// b3-expiry required, b2-allow manual pan,
		// b1-pin required, b0-account type selection
	// sOption[1]:
		// b7-inhibit pre-auth, b6-inhibit refund
		// b4-check expiry for offline
		// b3-capture txn, b2-print
		// b1-roc input required, b0-pan MOD 10 check
	// sOption[2]:
		// b7-pan Robinson's MOD 11 check, b4-check expiry
		// b3-omit auth_code display and input for offline
		// b2-extended pay, b1-additional data allowed
		// b0-inhibit void
	// sOption[3]:
		// b6--security code entry
	uchar	sOption[4];
	
	uchar	ucDefAccount;		// reserved ?
	uchar	sPanMask[2];		// for print slips
	uchar	sFloorLimit[2];		// BCD format
	uchar	ucAdjustPercent;	// BCD format

	// sReserved[0]: b8=1, pan mask left align, 0 right
	// b7=1, pan mask for display/print txn list
	uchar	sReserved[2];
}TMS_ISSUER;

// TMS tel NO. parameters
typedef struct _tagTMS_PHONE_INFO
{
	uchar	sTelNo[12];		// BCD format
	uchar	ucDialWait;		// by second, time wait after fail
	uchar	ucDialAttempts;	// dial attempt times, reserved?
}TMS_PHONE_INFO;

// TMS acquirer parameters
typedef struct _tagTMS_ACQUIRER
{
	uchar	ucKey;				// id key
	uchar	sPrgName[10];		// alias
	uchar	sName[10];			// acquirer name
// 	TMS_PHONE_INFO	stTxnPhoneInfo[2];
// 	TMS_PHONE_INFO	stStlPhoneInfo[2];

	uchar	sTxnTelNo1[12];		// BCD format
	uchar	ucTxnDialWait1;		// by second, time wait after fail
	uchar	ucTxnDialAttempts1;	// dial attempt times, reserved?
	uchar	sTxnTelNo2[12];		// BCD format
	uchar	ucTxnDialWait2;		// by second, time wait after fail
	uchar	ucTxnDialAttempts2;	// dial attempt times, reserved?

	uchar	sStlTelNo1[12];		// BCD format
	uchar	ucStlDialWait1;		// by second, time wait after fail
	uchar	ucStlDialAttempts1;	// dial attempt times, reserved?
	uchar	sStlTelNo2[12];		// BCD format
	uchar	ucStlDialWait2;		// by second, time wait after fail
	uchar	ucStlDialAttempts2;	// dial attempt times, reserved?

	uchar	ucTxnModemMode;		// reserved?
								// 0-bell 1200 sdlc,1-bell 1200 async e71
								// 2-ccitt 1200 sdlc
								// 3-ccitt 1200 async e71,4-bell 300 async e71
								// 5-ccitt 300 async e71,6-ccitt 2400 sdlc,
								// 7-ccitt 2400 async e71,20-ccitt 1200 etsl
	

	// sOption[0]:
		// b7-visa choice
		// b6-print audit trail after settle
		// b5-express payment prompt
		// b4-disable business date
		// b3-ignore host time
		// b2-request open suspended batch
		// b1-clear batch after initial
		// b0-amex specific feature
	// sOption[1]:
		// b5-prompt auth_code on receipt 1
		// b4-disable trick feed
		// b3-block auth prompt for offline
		// b2-use track 
		// b1-no rrn on receipt
		// b0-additional prompt
	// sOption[2]:
		// b3-online void, b2-online refund
	// sOption[3]: // unused
	uchar	sOption[4];
	
	uchar	sNii[2];				// BCD format
	uchar	sTermID[8]; 
	uchar	sMerchantID[15];
	uchar	ucTimeOut;				// BCD format, timeout for wait resp
	uchar	sCurBatchNo[3];
	uchar	sNextBatchNo[3];
	uchar	sVisa1TermID[23];		// reserved
	uchar	ucStlModemMode;			// see ucTxnModemMode

	//------------ The below part exist in newer Protims --------------
	uchar	COMM_Mode[8]; //MGCPLO(M-MODEM;G-GPRS;C-CDMA;P-PPP;L-LAN;O-OTHER) UPPER
	
	uchar   TCP_txn_IPADD1[4];
	uchar   TCP_txn_IPPORT1[2];
	uchar   TCP_txn_IPADD2[4];
	uchar   TCP_txn_IPPORT2[2];
	uchar   TCP_stl_IPADD1[4];
	uchar   TCP_stl_IPPORT1[2];
	uchar   TCP_stl_IPADD2[4];
	uchar   TCP_stl_IPPORT2[2];
	uchar   TCP_timeout_val;

	uchar   PPP_txn_PHONE[12];//compress BCD
	uchar   PPP_txn_USERNAME[16];
	uchar   PPP_txn_USERPWD[16];
	uchar   PPP_txn_IPADD1[4];
	uchar   PPP_txn_IPPORT1[2];
	uchar   PPP_txn_IPADD2[4];
	uchar   PPP_txn_IPPORT2[2];
	uchar   PPP_stl_PHONE[12];//compress BCD
	uchar   PPP_stl_USERNAME[16];
	uchar   PPP_stl_USERPWD[16];
	uchar   PPP_stl_IPADD1[4];
	uchar   PPP_stl_IPPORT1[2];
	uchar   PPP_stl_IPADD2[4];
	uchar   PPP_stl_IPPORT2[2];
	uchar   PPP_timeout_val;

	uchar   WIR_txn_PHONE1[12];//compress BCD
	uchar   WIR_txn_PHONE2[12];//compress BCD
	uchar   WIR_txn_IPADD1[4];
	uchar   WIR_txn_IPPORT1[2];
	uchar   WIR_txn_IPADD2[4];
	uchar   WIR_txn_IPPORT2[2];
	uchar   WIR_stl_PHONE1[12];//compress BCD
	uchar   WIR_stl_PHONE2[12];//compress BCD
	uchar   WIR_stl_IPADD1[4];
	uchar   WIR_stl_IPPORT1[2];
	uchar   WIR_stl_IPADD2[4];
	uchar   WIR_stl_IPPORT2[2];
	uchar   WIR_timeout_val;
	uchar   WIR_APN[64];
	uchar   WIR_USER[64];
	uchar   WIR_PWD[64];    
	uchar   WIR_SIMPIN[16];
	//------------ The above part exist in newer Protims --------------

	uchar	sReserved[4];
}TMS_ACQUIRER;

// TMS installment plan parameters
typedef struct _tagTMS_INSTALMENT_PLAN
{ 
	uchar	ucIndex;			// 1~MAX_PLAN, BCD format
	uchar	ucAcqIndex;			// binary
	uchar	sName[7];
	uchar	ucMonths;			// BCD format
	uchar	sBottomAmt[6];		// BCD format
}TMS_INSTALMENT_PLAN;

// TMS goods descriptors
typedef struct _tagTMS_DESCRIPTOR
{
	uchar	ucKey;
	uchar	sCode[2];		// ANS format
	uchar	sText[20];		// for display and print
}TMS_DESCRIPTOR;

// TMS EMV parameters
// Instructions: require to copy the matched element into EMV kernel, 
// notice that some elements of this structure are not need for EMV kernel.
typedef struct _tagTMS_EMV_MISC
{
	uchar	sCourtryCode[2];
	uchar	sCurcyCode[2];
	uchar	sRefCurcyCode[2];
	uchar	ucCurcyExp;
	uchar	ucRefCurcyExp;
	uchar	ucLanguage;
}TMS_EMV_MISC;

// TMS CAPK
typedef struct _tagTMS_EMV_CAPK
{
	uchar	sRID[5];			// registry ID
	uchar	ucKeyID;			// key id
	uchar	ucHashInd;			// HASH flag
	uchar	ucArithInd;			// RSA flag
	uchar	ucModulLen;			// module length
	uchar	sModul[248];		// module
	uchar	ucExpLen;			// exponent length
	uchar	sExponent[3];		// exponent
	uchar	sCheckSum[20];		// checksum
	uchar	sExpiry[3];
}TMS_EMV_CAPK;

// TMS EMV application
typedef struct _tagTMS_EMV_APP
{
	uchar	ucID;
	uchar	ucASI;  		// 0-partial match, 1-match exactly
	uchar	ucAIDLen;
	uchar	sAID[16];		// 5-16
	uchar	bLocalName;		// 0-use card info, 1-use local language.
	uchar	ucLocalNameLen;
	uchar	sLocalName[16];
	
	uchar	sAppVer[2];
	uchar	sTACDenial[5];
	uchar	sTACOnline[5];
	uchar	sTACDefault[5];
	uchar	bTermDDOL;
	uchar	ucTermDDOLLen;
	uchar	sTermDDOL[128];
	uchar	sFloorLimit[4];
	uchar	sThreshold[4];
	uchar	ucTargetPer;
	uchar	ucMaxTargetPer;
	
	uchar	bSupportVLP;			// out of date
	uchar	sVLPTACDenial[5];
	uchar	sVLPTACOnline[5];
	uchar	sVLPTACDefault[5];

	// elements below are not defined in the old protocol
	uchar	sTermType[2];
	uchar	sTDOL[256];
	uchar	ucCVM;					// b0: Plaintext PIN
									// b1: Enciphered PIN(Online)
									// b2: Signature
									// b3: Enciphered PIN(Offline)
									// No CVM.

	uchar	ucSecCapa;				// b0: SDA  b1:DDA  b2:CDA
	uchar	ucTransCapa;			// b0: Cash; b1: Services
									//      Goods; Cash Back.

	uchar	ucOption0;				// b0: Support PSE selection
									// b1: Card holder confirmation
									// b2: Bypass PIN
									// b3: Get Data for PIN
									// b4: Equipped PIN Pad
									// b5: Amount Entered at same pad
									//(4 excludes 5)

	uchar	ucTermRisk;				// b0: Floor limit check
									// b1: Random selection
									// b2: Velocity check
									// b3: exception file
									// b4: support TAC

	uchar	ucCompProc;				// b0: Forced online
									// b1: support advices
									// b2: support issuer referrals
									// b3: support card referral
									// b4: batch data capture
									// b5: online data capture
									// b6: default TDOL
									// (4 excludes 5)
}TMS_EMV_APP;

// record downloading status of each downloading application and its parameters
// definition:
// 0 -- no downloading
// 1 -- download successfully
// 2 -- failed to download
typedef struct _tagTMS_LOADSTATUS
{
	uchar	bAppTotal;			// total amount of applications on POS
	uchar	sAppNum[25];		// application id
	uchar	sLoadAppStat[25];	// applications downloading status
	uchar	sLoadParaStat[25];	// parameters downloading status
}TMS_LOADSTATUS;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void LoadEdcDefault(void);
void LoadEmvDefault(void);
void LoadDefCommPara(void);
void ResetAllPara(uchar bFirstTime);
int  NewTmsDownLoad(uchar ucCommType);
//void UpdateCommType(void);
void LoadEdcLang(void);
void LoadDefaultLang(void);
void NoDownloadInit(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _INITPOS_H

// end of file
