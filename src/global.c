
// 定义全局变量、常量等
#include "global.h"

// Added by Kim_LinHB 2014-4-4
#ifdef AREA_Arabia
unsigned int gl_AR_FONT_ID = AR_OPENFILE_ERROR;
#endif

PosParams		glPosParams;
//SYS_PARAM		glSysParam, glSysParamBak;		// sys config parameters
SYS_CONTROL		glSysCtrl;						// sys control parameters
SYS_PROC_INFO	glProcInfo;						// transaction processing information

COMM_DATA		glSendData, glRecvData;			// communication data
STISO8583		glSendPack;						// transaction sending package
STISO8583		glRecvPack;						// transaction receiving package 

COMM_CONFIG		glCommCfg;						// current communication config

#ifdef ENABLE_EMV
EMV_PARAM		glEmvParam;
EMV_STATUS		glEmvStatus;
#endif

// supported languages list
//FIXME Please set path for the font you need
const LANG_CONFIG glLangList[] = {
	{"English",      "",               CHARSET_WEST},
	{"Chinese Simp", "CHS.LNG",        CHARSET_GB2312},
	{"Chinese Trad", "CHT.LNG",        CHARSET_BIG5},
	{"Vietnamese",   "VIETNAMESE.LNG", CHARSET_VIETNAM},
	{"Thai",         "THAI.LNG",       CHARSET_WEST},
	{"Japanese",     "JAPANESE.LNG",   CHARSET_SHIFT_JIS},
	{"Korea",        "KOREA.LNG",      CHARSET_KOREAN},
	{"Arabia",       "ARABIA.LNG",	   CHARSET_ARABIA},
	{"", "", 0},
	// need pending
};

const CURRENCY_CONFIG glCurrency[] =
{
	{"NGN", "\x05\x66", "\x05\x66", 2, 0},
	{"HKD", "\x03\x44", "\x03\x44", 2, 0}, // 1.Hong Kong Dollars
	{"HK",  "\x03\x44", "\x03\x44", 2, 0}, // 2.
	{"HK$", "\x03\x44", "\x03\x44", 2, 0}, // 3.
	{"CNY", "\x01\x56", "\x01\x56", 2, 0}, // 4.Chinese Yuan
	{"RMB", "\x01\x56", "\x01\x56", 2, 0}, // 5.
	{"JPY", "\x03\x92", "\x03\x92", 0, 2}, // 6.Japanese Yen				// special
	{"EUR", "\x09\x78", "\x00\x00", 2, 0}, // 7.Euro -- Country not determined
	{"MOP", "\x04\x46", "\x04\x46", 2, 0}, // 8.Macao Pataca
	{"MYR", "\x04\x58", "\x04\x58", 2, 0}, // 9.Malaysian Ringgit
	{"PHP", "\x06\x08", "\x06\x08", 2, 0}, // 10.Philippine Pesos
	{"SGD", "\x07\x02", "\x07\x02", 2, 0}, // 11.Singapore Dollars
	{"THB", "\x07\x64", "\x07\x64", 2, 0}, // 12.Thai Baht
	{"TWD", "\x09\x01", "\x01\x58", 2, 0}, // 13.New Taiwanese Dollars
	{"NT",  "\x09\x01", "\x01\x58", 2, 0}, // 14.
	{"NT$", "\x09\x01", "\x01\x58", 2, 0}, // 15.
	{"USD", "\x08\x40", "\x08\x40", 2, 0}, // 16.US Dollars
	{"VND", "\x07\x04", "\x07\x04", 0, 2}, // 17.Vietnam DONG				// special
	{"AED", "\x07\x84", "\x07\x84", 2, 0}, // 18.United Arab Durham
	{"AUD", "\x00\x36", "\x00\x36", 2, 0}, // 19.Australian Dollars
	{"CAD", "\x01\x24", "\x01\x24", 2, 0}, // 20.Canadian Dollars
	{"CYP", "\x01\x96", "\x01\x96", 2, 0}, // 21.Cypriot Pounds
	{"CHF", "\x07\x56", "\x07\x56", 2, 0}, // 22.Swiss Francs
	{"DKK", "\x02\x08", "\x02\x08", 2, 0}, // 23.Danish Krone
	{"GBP", "\x08\x26", "\x08\x26", 2, 0}, // 24.British Pounds Sterling
	{"IDR", "\x03\x60", "\x03\x60", 0, 0}, // 25.Indonesia Rupiah			// special
	{"INR", "\x03\x56", "\x03\x56", 2, 0}, // 26.Indian Rupee
	{"ISK", "\x03\x52", "\x03\x52", 2, 0}, // 27.Icelandic krone
	{"KRW", "\x04\x10", "\x04\x10", 0, 2}, // 28.South Korean Won			// special
	{"LKR", "\x01\x44", "\x01\x44", 2, 0}, // 29.Sri-Lanka Rupee
	{"MTL", "\x04\x70", "\x04\x70", 2, 0}, // 30.Maltese Lira
	{"NOK", "\x05\x78", "\x05\x78", 2, 0}, // 31.Norwegian Krone
	{"NZD", "\x05\x54", "\x05\x54", 2, 0}, // 32.New Zealand Dollars
	{"RUB", "\x06\x43", "\x06\x43", 2, 0}, // 33.Russian Ruble
	{"SAR", "\x06\x82", "\x06\x82", 2, 0}, // 34.Saudi Riyal
	{"SEK", "\x07\x52", "\x07\x52", 2, 0}, // 35.Swedish krone
	{"TRL", "\x07\x92", "\x07\x92", 2, 0}, // 36.Turkey Lira
	{"VEF", "\x09\x37", "\x08\x62", 2, 0}, // 37.Bolivar Fuerte (Venezuela)
	{"ZAR", "\x07\x10", "\x07\x10", 2, 0}, // 38.South African Rand
	{"KWD", "\x04\x14", "\x04\x14", 3, 0}, // 39.Kuwaiti Dinar			// special
	{"CLP", "\x01\x52", "\x01\x52", 0, 2}, // 40.Chilean Piso				// special
	{"JOD", "\x04\x00", "\x04\x00", 3, 0}, // 41.Jordanian Dinar(JOR)
	{"MAD", "\x05\x04", "\x05\x04", 3, 0}, // 42.Morocco Dirham(MAR)
	{"", "", "", 0, 0} // maximum number of currencies per terminal is 50 
};

// !!!! Here must be one-one and accordant to the sequence of enum{PREAUTH,AUTH,...}
// !!!! See definition of "SALE","PREAUTH", ...
TRAN_CONFIG		glTranConfig[] =
{
	{_T_NOOP("PRE-AUTH"),       "0100", "300000", PRN_RECEIPT+ACT_INC_TRACE+NEED_REVERSAL},
	{_T_NOOP("AUTH"),           "0100", "000000", PRN_RECEIPT+ACT_INC_TRACE+NEED_REVERSAL}, // 1
	{_T_NOOP("SALE"),           "0200", "000000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},		// 2
	{_T_NOOP("INSTAL"),         "0200", "000000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},		// 3
	{_T_NOOP("ENQUIRE"),        "0800", "970000", ACT_INC_TRACE},							// 4
	{_T_NOOP("ENQUIRE"),        "0100", "000000", ACT_INC_TRACE},							// 5
	{_T_NOOP("UPLOAD"),         "0320", "000000", ACT_INC_TRACE},							// 6
	{_T_NOOP("LOGON"),          "0800", "920000", ACT_INC_TRACE},							// 7
	{_T_NOOP("REFUND"),         "0200", "200000", PRN_RECEIPT+ACT_INC_TRACE+IN_REFUND_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},	// 8
	{_T_NOOP("REVERSAL"),       "0400", "000000", NO_ACT},									// 9
	{_T_NOOP("SETTLEMENT"),     "0500", "920000", ACT_INC_TRACE},							// 10
	{_T_NOOP("INITIALIZATION"), "0800", "920000", ACT_INC_TRACE},							// 11
	{_T_NOOP("VOID"),           "0200", "020000", PRN_RECEIPT+NEED_REVERSAL+ACT_INC_TRACE},	// 12
	{_T_NOOP("UPLOAD OFFLINE"), "0220", "000000", PRN_RECEIPT+ACT_INC_TRACE},				// 13
	{_T_NOOP("OFFLINE"),        "0220", "000000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+WRT_RECORD+VOID_ALLOW},					// 14
	{_T_NOOP("SALECOMP"),       "0220", "000000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},		// 15
	{_T_NOOP("CASH"),           "0200", "010000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},		// 16
	{_T_NOOP("SALE"),			"0200", "000000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},		// 17.SALE_OR_AUTH // Modified by Kim_LinHB 2014-08-18 v1.01.0004
	{_T_NOOP("TC-ADVICE"),      "0320", "940000", ACT_INC_TRACE},							// 18
	{_T_NOOP("ECHO-TEST"),      "0800", "990000", NO_ACT},									// 19
	{_T_NOOP("ENQUIRE"),        "0800", "930000", ACT_INC_TRACE},							// 20
	{_T_NOOP("BIN DOWNLOAD"),   "0900", "000000", ACT_INC_TRACE},							// 21
};

HOST_ERR_MSG	glHostErrMsg[] =
{
	{"00", _T_NOOP("TXN. ACCEPTED")},
	{"01", _T_NOOP("PLS CALL BANK")},
	{"02", _T_NOOP("CALL REFERRAL")},
	{"03", _T_NOOP("INVALID MERCHANT")},
	{"04", _T_NOOP("PLS PICK UP CARD")},
	{"05", _T_NOOP("DO NOT HONOUR")},
	{"08", _T_NOOP("APPROVED WITH ID")},
	{"12", _T_NOOP("INVALID TXN")},
	{"13", _T_NOOP("INVALID AMOUNT")},
	{"14", _T_NOOP("INVALID ACCOUNT")},
	{"19", _T_NOOP("RE-ENTER TRANS.")},
	{"21", _T_NOOP("APPROVED. IDLE")},
	{"25", _T_NOOP("INVALID TERMINAL")},
	{"30", _T_NOOP("FORMAT ERROR")},
	{"41", _T_NOOP("PLEASE CALL-LC")},
	{"43", _T_NOOP("PLEASE CALL-CC")},
	{"51", _T_NOOP("TXN DECLINED")},
	{"54", _T_NOOP("EXPIRED CARD")},
	{"55", _T_NOOP("INCORRECT PIN")},
	{"58", _T_NOOP("INVALID TXN")},
	{"60", _T_NOOP("CALL ACQUIRER")},
	{"76", _T_NOOP("BAD PRODUCT CODE")},
	{"77", _T_NOOP("RECONCILE ERROR")},
	{"78", _T_NOOP("TRACE NOT FOUND")},
	{"80", _T_NOOP("BAD BATCH NUMBER")},
	{"85", _T_NOOP("BATCH NOT FOUND")},
	{"88", _T_NOOP("APPRV, CALL AMEX")},
	{"89", _T_NOOP("BAD TERMINAL ID")},
	{"91", _T_NOOP("SYSTEM NOT AVAIL")},
	{"94", _T_NOOP("DUPLICATE TRACE")},
	{"95", _T_NOOP("BATCH TRANSFER")},
	{"96", _T_NOOP("System Error")},
	{"97", _T_NOOP("Host Unavailable")},
	{"N1", _T_NOOP("Not DCC Eligible")},	//  "DCC 无 效"
	{"Q1", _T_NOOP("CARD AUTH FAIL")},		//  "CARD AUTH FAIL"
	{"Y1", _T_NOOP("OFFLINE APPROVAL")},	//  "离线授权"
	{"Z1", _T_NOOP("OFFLINE DECLINE")},		//  "离线拒绝交易"
	{"Y2", _T_NOOP("APPROVED")},			//  "APPROVED"
	{"Z2", _T_NOOP("DECLINED")},			//  "DECLINED"
	{"Y3", _T_NOOP("GO ONLINE FAIL")},		//  "GO ONLINE FAIL"
	{"Z3", _T_NOOP("GO ONLINE FAIL")},		//  "GO ONLINE FAIL"
	{"NA", _T_NOOP("NO UPDATE")},			//  "INVALID INFO"
	{"P0", _T_NOOP("ERROR SERIAL NO.")},	//  "INVALID INFO"
	{"XY", _T_NOOP("Duplicate Trans")},		//  "INVALID INFO"
	{"XX", _T_NOOP("NO DCC SGD REQ")},		//  "INVALID INFO"
	{"**", _T_NOOP("NO RESPONSE CODE")},
	{"\0\0", _T_NOOP("PLS CALL BANK")},
};

// 终端错误信息
// error message for internal error.
TERM_ERR_MSG	glTermErrMsg[] = 
{
	{ERR_COMM_MODEM_OCCUPIED,  _T_NOOP("PHONE OCCUPIED")},
	{ERR_COMM_MODEM_NO_LINE,   _T_NOOP("TRY AGAIN - NC")},
	{ERR_COMM_MODEM_LINE,      _T_NOOP("TRY AGAIN - CE")},
	{ERR_COMM_MODEM_NO_ACK,    _T_NOOP("NO ACK")},
	{ERR_COMM_MODEM_LINE_BUSY, _T_NOOP("LINE BUSY")},
	{ERR_NO_TELNO,             _T_NOOP("NO TEL NO")},
	{ERR_USERCANCEL,           _T_NOOP("USER CANCELED")},
	{ERR_TRAN_FAIL,            _T_NOOP("PROCESS FAILED")},
	{0, ""},
};

GUI_TEXT_ATTR gl_stTitleAttr = { GUI_FONT_LARGE, GUI_ALIGN_CENTER, GUI_FONT_BOLD };// | GUI_FONT_REVERSAL | GUI_FONT_OPAQUE };
GUI_TEXT_ATTR gl_stLeftAttr = {GUI_FONT_NORMAL, GUI_ALIGN_LEFT, GUI_FONT_STD};
GUI_TEXT_ATTR gl_stCenterAttr = {GUI_FONT_NORMAL, GUI_ALIGN_CENTER, GUI_FONT_STD};
GUI_TEXT_ATTR gl_stCenterAttrAlt = { GUI_FONT_NORMAL, GUI_ALIGN_CENTER, GUI_FONT_REVERSAL | GUI_FONT_OPAQUE };
GUI_TEXT_ATTR gl_stRightAttr = {GUI_FONT_NORMAL, GUI_ALIGN_RIGHT, GUI_FONT_STD};

unsigned char gl_szCurrTitle[50];
// Add End

// Added by Kim_LinHB 2014-8-8 v1.01.0002
#if defined(_Sxxx_)
ST_FONT gl_Font_Def[3] = {
	{CHARSET_WEST, 8, 16, 0,0},
	{CHARSET_WEST, 15, 30,0,0},
	{CHARSET_WEST, 20, 40, 0,0},
};
#elif defined(_Dxxx_)
#ifdef _MIPS_
ST_FONT gl_Font_Def[3] = {
	{CHARSET_WEST, 8, 15, 0,0},
	{CHARSET_WEST, 16, 30,0,0},
	{CHARSET_WEST, 20, 48, 0,0},
};
// Added by Kim_LinHB 2014-08-14 v1.01.0003
#else
ST_FONT gl_Font_Def[3] = {
	{CHARSET_WEST, 8, 16, 0,0},
	{CHARSET_WEST, 15, 30,0,0},
	{CHARSET_WEST, 20, 40, 0,0},
};
#endif
#else
ST_FONT gl_Font_Def[3] = {
	{CHARSET_WEST, 6, 8, 0,0},
	{CHARSET_WEST, 8, 16,0,0},
	{CHARSET_WEST, 12, 24, 0,0},
};
#endif

// end of file

