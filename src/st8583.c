
#include "pack8583.h"
#include "st8583.h"

//ISO8583使用说明
//Step1: 根据收单行的对于报文中每个域的定义，修改glEdcDataDef结构

// Usage of this module
// Step1: modify the "glEdcDataDef", according to the definition of each bit that the bank requires.
// For step 2, see in st8583.h;

// 说明：FIELD_ATTR结构定义请参见ISO8583处理模块定义
// Check the "FIELD_ATTR" definition first.
// This definition is specified by bank.
FIELD_ATTR glEdcDataDef[] =
{
	{Attr_n, Attr_fix,  LEN_MSG_CODE},		// message code(MTI)
	{Attr_b, Attr_fix,  LEN_BITMAP},		// 1 - bitmap
	{Attr_n, Attr_var1, LEN_PAN},			// 2 - PAN
	{Attr_n, Attr_fix,  LEN_PROC_CODE},		// 3 - process code
	{Attr_n, Attr_fix,  LEN_TRAN_AMT},		// 4 - txn amount
	{Attr_n, Attr_fix,  LEN_FRN_AMT},		// 5 - Foreign Amt(DCC)
	{Attr_UnUsed, Attr_fix, 0},				// 6 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 7 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 8 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 9 - not used
	{Attr_n, Attr_fix, LEN_DCC_RATE},		// 10 - DCC rate
	{Attr_n, Attr_fix, LEN_STAN},			// 11 - STAN
	{Attr_n, Attr_fix, LEN_LOCAL_TIME},		// 12 - local time
	{Attr_n, Attr_fix, LEN_LOCAL_DATE},		// 13 - local date
	{Attr_n, Attr_fix, LEN_EXP_DATE},		// 14 - expire
	{Attr_UnUsed, Attr_fix, 0},				// 15 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 16 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 17 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 18 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 19 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 20 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 21 - not used
	{Attr_n, Attr_fix, LEN_ENTRY_MODE},		// 22 - entry modes
	{Attr_n, Attr_fix, LEN_PAN_SEQ_NO},		// 23 - PAN Seq #
	{Attr_n, Attr_fix, LEN_NII},			// 24 - NII
	{Attr_n, Attr_fix, LEN_COND_CODE},		// 25 - condition code
	{Attr_UnUsed, Attr_fix, 0},				// 26 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 27 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 28 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 29 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 30 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 31 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 32 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 33 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 34 - not used
	{Attr_z, Attr_var1, LEN_TRACK2},		// 35 - track 2
	{Attr_z, Attr_var2, LEN_TRACK3},		// 36 - track 3
	{Attr_a, Attr_fix,  LEN_RRN},			// 37 - RRN
	{Attr_a, Attr_fix,  LEN_AUTH_CODE},		// 38 - auth. code
	{Attr_a, Attr_fix,  LEN_RSP_CODE},		// 39 - response code
	{Attr_UnUsed, Attr_fix, 0},				// 40 - not uesed
	{Attr_a, Attr_fix, LEN_TERM_ID},		// 41 - TID
	{Attr_a, Attr_fix, LEN_MERCHANT_ID},	// 42 - MID
	{Attr_UnUsed, Attr_fix, 0},				// 43 - not used
	{Attr_a, Attr_var1, LEN_ADDL_RSP},		// 44 - Add'l rsp data
	{Attr_a, Attr_var1, LEN_TRACK1},		// 45 - track 1
	{Attr_UnUsed, Attr_fix, 0},				// 46 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 47 - not used
	{Attr_b, Attr_var2, LEN_FIELD48},		// 48 - add'l req data
	{Attr_a, Attr_fix,  LEN_CURCY_CODE},	// 49 - Transaction Currency code
	{Attr_UnUsed, Attr_fix, 0},				// 50 - not used
	{Attr_n, Attr_fix,  LEN_CURCY_CODE},	// 51 - Card-holder Currency code
	{Attr_b, Attr_fix, LEN_PIN_DATA},		// 52 - PIN data
	{Attr_UnUsed, Attr_fix, 0},				// 53 - not used
	{Attr_a, Attr_var2, LEN_EXT_AMOUNT},	// 54 - Extra Amount
	{Attr_b, Attr_var2, LEN_ICC_DATA},		// 55 - ICC data
	{Attr_b, Attr_var2, LEN_ICC_DATA2},		// 56 - ICC data 2(HK)
	{Attr_UnUsed, Attr_fix, 0},				// 57 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 58 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 59 - not used
	{Attr_a, Attr_var2, LEN_FIELD60},		// 60 - Private used
	{Attr_a, Attr_var2, LEN_FIELD61},		// 61 - POS SN/desc code
//	{Attr_a, Attr_var2, LEN_FIELD62},		// 62 - ROC/SOC
	{Attr_b, Attr_var2, LEN_FIELD62},		// 62 - ROC/SOC
	{Attr_b, Attr_var2, LEN_FIELD63},		// 63 - Private used
	{Attr_b, Attr_fix,  LEN_MAC},			// 64 - MAC(not used)
	{Attr_Over,   Attr_fix, 0}
};

// TMS消息定义
// This definition is specified by Protims.
FIELD_ATTR glTMSDataDef[] =
{
	{Attr_n, Attr_fix,  LEN_MSG_CODE},		// message code(MTI)
	{Attr_b, Attr_fix,  LEN_BITMAP},		// 1 - bitmap
	{Attr_UnUsed, Attr_fix, 0},				// 2 - PAN
	{Attr_n, Attr_fix,  LEN_PROC_CODE},		// 3 - process code
	{Attr_UnUsed, Attr_fix, 0},				// 4 - txn amount
	{Attr_UnUsed, Attr_fix, 0},				// 5 - Foreign Amt(DCC)
	{Attr_UnUsed, Attr_fix, 0},				// 6 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 7 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 8 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 9 - DCC rate
	{Attr_UnUsed, Attr_fix, 0},				// 10 - not used
	{Attr_n, Attr_fix, LEN_STAN},			// 11 - STAN
	{Attr_n, Attr_fix, LEN_LOCAL_TIME},		// 12 - local time
	{Attr_n, Attr_fix, LEN_LOCAL_DATE},		// 13 - local date
	{Attr_UnUsed, Attr_fix, 0},				// 14 - expire
	{Attr_UnUsed, Attr_fix, 0},				// 15 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 16 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 17 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 18 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 19 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 20 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 21 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 22 - entry modes
	{Attr_UnUsed, Attr_fix, 0},				// 23 - PAN Seq #
	{Attr_n, Attr_fix, LEN_NII},			// 24 - NII
	{Attr_UnUsed, Attr_fix, 0},				// 25 - condition code
	{Attr_UnUsed, Attr_fix, 0},				// 26 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 27 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 28 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 29 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 30 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 31 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 32 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 33 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 34 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 35 - track 2
	{Attr_UnUsed, Attr_fix, 0},				// 36 - track 3
	{Attr_UnUsed, Attr_fix, 0},				// 37 - RRN
	{Attr_UnUsed, Attr_fix, 0},				// 38 - auth. code
	{Attr_a, Attr_fix,  LEN_RSP_CODE},		// 39 - response code
	{Attr_UnUsed, Attr_fix, 0},				// 40 - not uesed
	{Attr_a, Attr_fix, LEN_TERM_ID},		// 41 - TID
	{Attr_a, Attr_fix, LEN_MERCHANT_ID},	// 42 - MID
	{Attr_UnUsed, Attr_fix, 0},				// 43 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 44 - Add'l rsp data
	{Attr_UnUsed, Attr_fix, 0},				// 45 - track 1
	{Attr_UnUsed, Attr_fix, 0},				// 46 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 47 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 48 - add'l req data
	{Attr_UnUsed, Attr_fix, 0},				// 49 - FRN Curcy code
	{Attr_UnUsed, Attr_fix, 0},				// 50 - Currency code
	{Attr_UnUsed, Attr_fix, 0},				// 51 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 52 - PIN data
	{Attr_UnUsed, Attr_fix, 0},				// 53 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 54 - Extra Amount
	{Attr_UnUsed, Attr_fix, 0},				// 55 - ICC data
	{Attr_UnUsed, Attr_fix, 0},				// 56 - ICC data 2(HK)
	{Attr_UnUsed, Attr_fix, 0},				// 57 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 58 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 59 - not used
	{Attr_b, Attr_var2, LEN_TMSFIELD60},	// 60 - Private used
	{Attr_a, Attr_var2, LEN_FIELD61},		// 61 - POS SN/desc code
	{Attr_UnUsed, Attr_fix, 0},				// 62 - ROC/SOC
	{Attr_UnUsed, Attr_fix, 0},				// 63 - Private used
	{Attr_UnUsed, Attr_fix, 0},				// 64 - MAC(not used)
	{Attr_Over,   Attr_fix, 0}
};

// end of file
