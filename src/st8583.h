
/****************************************************************************
NAME
    st8583.h - 定义系统8583结构

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _ST8583_H
#define _ST8583_H

#define	LEN_MSG_CODE			4
#define	LEN_BITMAP				8
#define	LEN_PAN					19
#define	LEN_PROC_CODE			6
#define	LEN_TRAN_AMT			12
#define	LEN_FRN_AMT				12
#define	LEN_DCC_RATE			8
#define	LEN_STAN				6
#define	LEN_LOCAL_TIME			6
#define	LEN_LOCAL_DATE			4
#define	LEN_EXP_DATE			4
#define	LEN_SETTLE_DATE			4
#define	LEN_ENTRY_MODE			4
#define	LEN_PAN_SEQ_NO			3
#define	LEN_NII					3
#define	LEN_COND_CODE			2
#define	LEN_TRACK2				37
#define	LEN_TRACK3				104
#define	LEN_RRN					12
#define	LEN_AUTH_CODE			6
#define	LEN_RSP_CODE			2
#define	LEN_TERM_ID				8
#define	LEN_MERCHANT_ID			15
#define	LEN_ADDL_RSP			2
#define	LEN_TRACK1				76
#define	LEN_FIELD48				100
#define	LEN_CURCY_CODE			3
#define	LEN_PIN_DATA			8
#define	LEN_EXT_AMOUNT			12
#define	LEN_ICC_DATA			260
#define	LEN_ICC_DATA2			110
#define	LEN_FIELD60				22
#define	LEN_TMSFIELD60			600
#define	LEN_FIELD61				30
#define	LEN_INVOICE_NO			6
#define LEN_FIELD62             100
#define	LEN_FIELD63				800
#define LEN_MAC					8

//ISO8583使用说明
//Step2: 根据Step1的定义，定义结构中使用到的有实际意义的变量名，Attr_UnUsed及Attr_Over型的跳过不定义
//注意与glEdcDataDef结构要一定要一一对应,否则打包会出错.
//注意若在glEdcDataDef该域为Attr_a, Attr_n, Attr_z型时，为sz打头
//若在glEdcDataDef该域为Attr_b型时，为s打头

// Usage of ISO8583 module (For step 1, see in st8583.c; For step 3, see in TranProc.c)
// Step 2
// According to the definition in step 1, defines member variable in below "STISO8583".
// Those of type "Attr_UnUsed" and "Attr_Over" needn't to define.
// NOTICE that it must be corresponding to the sequence in "glEdcDataDef", one by one.
// if in "glEdcDataDef", the bit attribute is "Attr_a", "Attr_n" or "Attr_z", the member name should be start with "sz"
// if in "glEdcDataDef", the bit attribute is "Attr_b", the member name should be start with "s"

// 说明：
// 1. 对Binary类型的域，结构成员变量的前面2个字节为数据长度
//    格式为高有效为在前的网络字节顺序
// 2. 对非Binary类型域，直接使用C的字符串操作/格式化函数进行赋值即可
//    多余的2个字节是为了存储'\0'字符(为方便处理冗余了一个字符)
// 1. To those bit attribute "Attr_b" (member name: sxxxxxxx[]), reserves heading 2 bytes for storing length in hex.
//      The length format is like : "\x01\x2A" when length=0x012A
// 2. To those not "Attr_b", can use "sprintf" to fill data in ASCII.
//      The extra 2 bytes are to store the ending "\x00". (for abandon, use 2 bytes.)
typedef struct _tagSTISO8583
{
	uchar	szMsgCode[LEN_MSG_CODE+2];				// message code
	uchar	sBitMap[2*LEN_BITMAP];					// !!!! No leading 2 length bytes !!!!  不要加2
	uchar	szPan[LEN_PAN+2];						// PAN
	uchar	szProcCode[LEN_PROC_CODE+2];			// proc code
	uchar	szTranAmt[LEN_TRAN_AMT+2];				// Txn Amount
	uchar	szFrnAmt[LEN_FRN_AMT+2];				// Foreign amt
	uchar	szDccRate[LEN_DCC_RATE+2];				// DCC Rate
	uchar	szSTAN[LEN_STAN+2];						// STAN
	uchar	szLocalTime[LEN_LOCAL_TIME+2];			// time, hhmmss
	uchar	szLocalDate[LEN_LOCAL_DATE+2];			// date, YYMM
	uchar	szExpDate[LEN_EXP_DATE+2];				// Expiry, YYMM
	uchar	szEntryMode[LEN_ENTRY_MODE+2];			// entry mode
	uchar	szPanSeqNo[LEN_PAN_SEQ_NO+2];			// PAN seq #
	uchar	szNii[LEN_NII+2];						// NII
	uchar	szCondCode[LEN_COND_CODE+2];			// Cond. code
	uchar	szTrack2[LEN_TRACK2+2];					// track 2
	uchar	szTrack3[LEN_TRACK3+2];					// track 3
	uchar	szRRN[LEN_RRN+2];						// RRN
	uchar	szAuthCode[LEN_AUTH_CODE+2];			// auth code
	uchar	szRspCode[LEN_RSP_CODE+2];				// rsp code
	uchar	szTermID[LEN_TERM_ID+2];				// terminal id
	uchar	szMerchantID[LEN_MERCHANT_ID+2];		// merchant id
	uchar	szAddlRsp[LEN_ADDL_RSP+2];				// add'l rsp
	uchar	szTrack1[LEN_TRACK1+2];					// track 1
	uchar	sField48[LEN_FIELD48+2];				// for instalment or cvv2 for visa/master card
	uchar	szTranCurcyCode[LEN_CURCY_CODE+2];		// for DCC, transaction currency
	uchar	szHolderCurcyCode[LEN_CURCY_CODE+2];	// for DCC, holder currency
	uchar	sPINData[LEN_PIN_DATA+2];				// PIN data
	uchar	szExtAmount[LEN_EXT_AMOUNT+2];			// extra amount
	uchar	sICCData[LEN_ICC_DATA+2];				// ICC data, or AMEX non-EMV transaction 4DBC
	uchar	sICCData2[LEN_ICC_DATA2+2];				// ICC data, FOR HK
	uchar	szField60[LEN_FIELD60+2];
	uchar	szField61[LEN_FIELD61+2];
	uchar	sField62[LEN_FIELD62+2];
	uchar	sField63[LEN_FIELD63+2];
	uchar	sMac[LEN_MAC+2];
}STISO8583;

// TMS专用报文
// For TMS use.
typedef struct _tagSTTMS8583
{
	uchar	szMsgCode[LEN_MSG_CODE+2];				// message code
	uchar	sBitMap[2*LEN_BITMAP];					// 不要加2
	uchar	szProcCode[LEN_PROC_CODE+2];			// proc code
	uchar	szSTAN[LEN_STAN+2];						// STAN
	uchar	szLocalTime[LEN_LOCAL_TIME+2];			// time, hhmmss
	uchar	szLocalDate[LEN_LOCAL_DATE+2];			// date, YYMM
	uchar	szNii[LEN_NII+2];						// NII
	uchar	szRspCode[LEN_RSP_CODE+2];				// rsp code
	uchar	szTermID[LEN_TERM_ID+2];				// terminal id
	uchar	szMerchantID[LEN_MERCHANT_ID+2];		// merchant id
	uchar	sField60[LEN_TMSFIELD60+2];
	uchar	szField61[LEN_FIELD61+2];
}STTMS8583;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern FIELD_ATTR glEdcDataDef[];		// 8583消息域定义
extern FIELD_ATTR glTMSDataDef[];		// 8583消息域定义

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _ST8583_H

// end of file
