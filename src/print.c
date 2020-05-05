
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
enum{
	PRN_6x8=0,
	PRN_8x16,
	PRN_16x16,
	PRN_12x24,
	PRN_24x24,
	PRN_6x12,
	PRN_12x12,
	PRN_NULL=0xFF
};





static int  PrnFontSetNew(uchar ucEnType, uchar ucSQType);
//static void PrnSmallConstStr(uchar *str);

static int  PrintReceipt_FreeFmat(uchar ucPrnFlag);
static int  PrintReceipt_T(uchar ucPrnFlag);
static int  PrnCurAcqTransList_T(void);
static void PrnHead(uchar ucFreeFmat);
static int  PrnCustomLogo_T(void);
static void PrnHead_T(void);
static void PrnAmount(const uchar *pszIndent, uchar ucNeedSepLine);
static void PrnDescriptor(void);
static void PrnAdditionalPrompt(void);
static void PrnStatement(void);
static void PrnTotalInfo(const void *pstInfo);
static int  PrnParaAcq(uchar ucAcqIndex);
static void PrnParaIssuer(uchar ucAcqIndex);
static void PrnParaIssuerSub(uchar ucIssuerKey);
static void PrnIssuerOption(const uchar *psOption);
static void PrnCardTable(uchar ucIssuerKey);
static int  PrnInstalmentPara(void);
static int  PrnEmvPara(void);
static void PrnHexString(const char *pszTitle, const uchar *psHexStr, int iLen, uchar bNewLine);
static void MultiLngPrnStr(const uchar *str, uchar mode);

static void  PrnEngTime(void);

/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// For thermal, small={8x16,16x16}
// For sprocket, small=normal={6x8,16x16}
// Modified by Kim_LinHB 2014-6-8
void PrnSetSmall(void)
{
	if (ChkIfThermalPrinter())
	{
#ifdef AREA_Arabia
		if(0 == strcmp(LANGCONFIG, "Arabic"))
		{
			if (ArPrnFontSelect(gl_AR_FONT_ID, 4)!=AR_SUCCESS)
			{
				while(1)
				{
					ScrCls();
					ScrPrint(0,0,0,"Error:PAX_ARFA.FONT error\nPls download ParamFile");
					getkey();
				}
			}
		}
		else
#endif
		{
#if defined(_Sxx_) || defined(_SP30_) || defined(_Sxxx_)
			PrnFontSetNew(PRN_8x16, PRN_16x16);
			PrnSpaceSet(1, 2);
#else
			PrnFontSet(0, 0);
			PrnSpaceSet(1, 2);
#endif
		}
	}
	else
	{
		PrnSetNormal();
	}
}

// For thermal, normal={12x24,24x24}
// For sprocket, normal={6x8,16x16}
// Modified by Kim_LinHB 2014-6-8
void PrnSetNormal(void)
{
	if (ChkIfThermalPrinter())
	{
#ifdef AREA_Arabia
		if(0 == strcmp(LANGCONFIG, "Arabic"))
		{
			if (ArPrnFontSelect(gl_AR_FONT_ID, 6)!=AR_SUCCESS)
			{
				while(1)
				{
					ScrCls();
					ScrPrint(0,0,0,"Error:PAX_ARFA.FONT error\nPls download ParamFile");
					getkey();
				}
			}
		}
		else
#endif
		{
#if defined(_Sxx_) || defined(_SP30_) || defined(_Sxxx_)
		PrnFontSetNew(PRN_12x24, PRN_24x24);
#else
		PrnFontSet(1, 1);
#endif
		PrnSpaceSet(1, 3);
		}
	}
	else
	{
#if defined(_Sxx_) || defined(_SP30_) || defined(_Sxxx_)
		PrnFontSetNew(PRN_6x8, PRN_16x16);
#else
		PrnFontSet(0, 0);
#endif
		PrnSpaceSet(0, 2);
	}
}

// For thermal, normal=big={12x24,24x24}
// For sprocket, big={8x16,16x16}
// Modified by Kim_LinHB 2014-6-8
void PrnSetBig(void)
{
	if (ChkIfThermalPrinter())
	{
#ifdef AREA_Arabia
		if(0 == strcmp(LANGCONFIG, "Arabic"))
		{
			if (ArPrnFontSelect(gl_AR_FONT_ID, 8)!=AR_SUCCESS)
			{
				while(1)
				{
					ScrCls();
					ScrPrint(0,0,0,"Error:PAX_ARFA.FONT error\nPls download ParamFile");
					getkey();
				}
			}
		}
		else
#endif
		{
			PrnSetNormal();
		}
	}
	else
	{
#if defined(_Sxx_) || defined(_SP30_) || defined(_Sxxx_)
		PrnFontSetNew(PRN_8x16, PRN_16x16);
#else
		PrnFontSet(1, 1);
#endif
		PrnSpaceSet(0, 2);
	}
}

int PrintReceipt(uchar ucPrnFlag)
{
	uchar	szBuff[100];
	uchar	szIssuerName[10+1];


	if (ChkIfIrDAPrinter())
	{
		SetOffBase(OffBaseDisplay);
		ChkOnBase();
	}
	

	DispPrinting();
	if( ChkIfThermalPrinter() )
	{
		return PrintReceipt_T(ucPrnFlag);
	}
	
	if( ChkEdcOption(EDC_FREE_PRINT) )	// Free format print
	{
		return PrintReceipt_FreeFmat(ucPrnFlag);
	}

	PrnInit();
	PrnSetNormal();

	PrnStr("\n\n\n");
	PrnHead(FALSE);


	//	memcpy(szBuff, glProcInfo.stTranLog.szPan, sizeof(glProcInfo.stTranLog.szPan));
	if (ChkIfTransMaskPan(1))
	{
		MaskPan(glProcInfo.stTranLog.szPan, szBuff);
	}
	else
	{
		strcpy(szBuff, glProcInfo.stTranLog.szPan);
	}
	if( glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT )
	{
		PrnStr("%s S\n", szBuff);
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
		PrnStr("%s C\n", szBuff);
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		PrnStr("%s F\n", szBuff);
	}
    else if(glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS)//bug806
    {
        PrnStr("%s T\n", szBuff);
    }
	else
	{
		PrnStr("%s M\n", szBuff);
	}
	PrnStr("%s\n", glProcInfo.stTranLog.szHolderName);

	// print txn name & expiry
	if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
	{		
		sprintf((char *)szBuff, "%s(%s)", _T("VOID"), _T(glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel));
	}
	else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
	{
		sprintf((char *)szBuff, "%s(%s)", _T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel), _T("ADJ"));
	}
	else
	{
		sprintf((char *)szBuff, "%s", _T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel));
	}
	if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
	{
		if( ChkIssuerOption(ISSUER_MASK_EXPIRY) )
		{
			PrnSetBig();
			PrnStr(" %-16.16s", szBuff);
			PrnSetNormal();
			PrnStr("**/**\n");
		}
		else
		{
			PrnSetBig();
			PrnStr(" %-16.16s", szBuff);
			PrnSetNormal();
			PrnStr("%2.2s/%2.2s\n", &glProcInfo.stTranLog.szExpDate[2],
					glProcInfo.stTranLog.szExpDate);
		}
	}
	else
	{
		PrnSetBig();
		PrnStr(" %s\n", szBuff);
		PrnSetNormal();
	}

	// Batch NO & invoice #
	PrnStep(6);
	PrnStr("   %06ld%14s%06ld\n", glPosParams.batchNo, "", glProcInfo.stTranLog.ulInvoiceNo);
	Conv2EngTime(glProcInfo.stTranLog.szDateTime, szBuff);  //DATE/TIME
	PrnStr("%22s\n", szBuff);
	PrnStr("%15.12s%8s%-6.6s\n", glProcInfo.stTranLog.szRRN, "", glProcInfo.stTranLog.szAuthCode);

	PrnStep(2);
	if( glProcInfo.stTranLog.ucInstalment!=0 )
	{
		PrnStr("   NO. OF INSTALMENT:%02d\n", glProcInfo.stTranLog.ucInstalment);
	}

	if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
        if (strlen(glProcInfo.stTranLog.szAppPreferName)!=0)
        {
		    PrnStr("   APP: %.16s\n", glProcInfo.stTranLog.szAppPreferName);
        } 
        else
        {
		    PrnStr("   APP: %.16s\n", glProcInfo.stTranLog.szAppLabel);
        }
		PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff);
		PubTrimTailChars(szBuff, 'F');
		PrnStr("   AID: %s\n", szBuff);
		PubBcd2Asc0(glProcInfo.stTranLog.sAppCrypto, 8, szBuff);
		PrnStr("   TC : %s\n", szBuff);
#ifdef ENABLE_EMV
#ifdef EMV_TEST_VERSION
// 		PubBcd2Asc0(glProcInfo.stTranLog.sTSI, 2, szBuff);
// 		PrnStr("   TSI: %s\n", szBuff);
// 		PubBcd2Asc0(glProcInfo.stTranLog.sTVR, 5, szBuff);
// 		PrnStr("   TVR: %s\n", szBuff);
#endif
#endif
		if( glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN )
		{
			PrnStr("   PIN VERIFIED\n");
		}
		else
		{
			PrnStr("\n");
		}
	}

	PrnDescriptor();

	// amount
	PrnAmount((uchar *)"   ", TRUE);

	PrnAdditionalPrompt();

	PrnStatement();

	if( ucPrnFlag==PRN_REPRINT )
	{
		PrnSetBig();
		PrnStr("       REPRINT\n");
	}
#ifdef ENABLE_EMV
#ifdef EMV_TEST_VERSION
		PrnStr("* FOR EMV TEST ONLY *");
#endif
#endif
	PrnStr("\f");

	StartPrinter();
	return 0;
}

// Print the list of all transaction
int PrnAllList(void)
{
	int		iRet;
	uchar	ucIndex;

	SetCurrTitle(_T("PRINT LOG")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493

	iRet = SelectAcq(FALSE, GetCurrTitle(), &ucIndex);
	if( iRet!=0 )
	{
		return ERR_NO_DISP;
	}

	if( ucIndex!=ACQ_ALL )
	{
//		SetCurAcq(ucIndex);  //ALEX ADD
		PrnCurAcqTransList();
		return ERR_NO_DISP;
	}

	return 0;
}

// print list of transaction of current acquirer
int PrnCurAcqTransList(void)
{
	return 0;
}

int PrnCurAcqTransList_T(void)
{
	return 0;
}

int PrintReceipt_FreeFmat(uchar ucPrnFlag)
{
	uchar	szBuff[50];
	uchar	szIssuerName[10+1];

	PrnInit();
	PrnSetNormal();

	PrnStr("\n\n\n");
	PrnHead(TRUE);

	// issuer Name
	PrnStr("CARD TYPE: %-10.10s    ", szIssuerName);

	// Expiry date
	if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
	{
		if( ChkIssuerOption(ISSUER_MASK_EXPIRY) )
		{
			PrnStr("**/**");
		}
		else
		{
			PrnStr("%2.2s/%2.2s", &glProcInfo.stTranLog.szExpDate[2],
					glProcInfo.stTranLog.szExpDate);
		}
	}

	PrnStr("\n");

	//	PAN
	if (ChkIfTransMaskPan(1))
	{
		MaskPan(glProcInfo.stTranLog.szPan, szBuff);
	}
	else
	{
		strcpy(szBuff, glProcInfo.stTranLog.szPan);
	}
	if( glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT )
	{
		PrnStr("CARD NO: %s S\n", szBuff);
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
		PrnStr("CARD NO: %s C\n", szBuff);
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		PrnStr("CARD NO: %s F\n", szBuff);
	}
	else
	{
		PrnStr("CARD NO: %s M\n", szBuff);
	}

	// Holder
	PrnStr("HOLDER: %s\n", glProcInfo.stTranLog.szHolderName);

	// print txn name & expiry
	if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
	{		
		sprintf((char *)szBuff, "%s(%s)", _T("VOID"), _T(glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel));
	}
	else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
	{
		sprintf((char *)szBuff, "%s(%s)", _T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel), _T("ADJ"));
	}
	else
	{
		sprintf((char *)szBuff, "%s", _T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel));
	}
	PrnStr("%s\n", szBuff);

	// Batch NO & invoice #
	PrnStep(6);
	PrnStr("BATCH NO: %06ld   REF: %06ld\n", glPosParams.batchNo, glProcInfo.stTranLog.ulInvoiceNo);

	// RRN, AuthCode
	PrnStr("RRN: %-12.12s AUTH: %-6.6s\n", glProcInfo.stTranLog.szRRN, glProcInfo.stTranLog.szAuthCode);

	PrnStep(2);
	if( glProcInfo.stTranLog.ucInstalment!=0 )
	{
		PrnStr("   NO. OF INSTALMENT:%02d\n", glProcInfo.stTranLog.ucInstalment);
	}

	if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
        if (strlen(glProcInfo.stTranLog.szAppPreferName)!=0)
        {
		    PrnStr("   APP: %.16s\n", glProcInfo.stTranLog.szAppPreferName);
        } 
        else
        {
		    PrnStr("   APP: %.16s\n", glProcInfo.stTranLog.szAppLabel);
        }
		PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff);
		PubTrimTailChars(szBuff, 'F');
		PrnStr("   AID: %s\n", szBuff);
		PubBcd2Asc0(glProcInfo.stTranLog.sAppCrypto, 8, szBuff);
		PrnStr("   TC : %s\n", szBuff);
#ifdef ENABLE_EMV
#ifdef EMV_TEST_VERSION
// 		PubBcd2Asc0(glProcInfo.stTranLog.sTSI, 2, szBuff);
// 		PrnStr("   TSI: %s\n", szBuff);
// 		PubBcd2Asc0(glProcInfo.stTranLog.sTVR, 5, szBuff);
// 		PrnStr("   TVR: %s\n", szBuff);
#endif
#endif
		if( glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN )
		{
			PrnStr("   PIN VERIFIED\n");
		}
		else
		{
			PrnStr("\n");
		}
	}

	PrnDescriptor();

	PrnAmount((uchar *)"   ", TRUE);

	PrnAdditionalPrompt();

	PrnStatement();

	if( ucPrnFlag==PRN_REPRINT )
	{
		PrnFontSetNew(PRN_8x16, PRN_16x16);
		PrnStr("       REPRINT\n");
	}

	PrnStr("\f");

	StartPrinter();
	return 0;
}

int PrintReceipt_T(uchar ucPrnFlag)
{	
	uchar	ucNum;
	uchar	szBuff[50],szBuf1[50];
	uchar	szIssuerName[10+1], szTranName[16+1];

	for(ucNum=0; ucNum<NumOfReceipt(); ucNum++)
	{
		PrnInit();
		PrnSetNormal();

		PrnCustomLogo_T();

		PrnHead_T();

		// issuer Name
		memset(szBuff, 0, sizeof(szBuff));
		sprintf((char *)szBuff, "%s\n", _T("CARD TYPE:"));
		PrnSetSmall();
		MultiLngPrnStr(szBuff,GUI_ALIGN_LEFT);

		PrnSetNormal();
		memset(szBuff, 0, sizeof(szBuff));
		sprintf((char *)szBuff, "%s\n", szIssuerName);
		MultiLngPrnStr(szBuff,GUI_ALIGN_LEFT);

		// PAN
		sprintf((char *)szBuff, "%s\n", _T("CARD NO./EXP. DATE"));
		PrnSetSmall();
		MultiLngPrnStr(szBuff,GUI_ALIGN_LEFT);
		//memcpy(szBuff, glProcInfo.stTranLog.szPan, sizeof(glProcInfo.stTranLog.szPan));

		if (ChkIfTransMaskPan(ucNum))
		{
			MaskPan(glProcInfo.stTranLog.szPan, szBuff);
		}
		else
		{
			strcpy((char *)szBuff, (char *)glProcInfo.stTranLog.szPan);
		}

		if( glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT )
		{
			sprintf(szBuf1, "%s (S)", szBuff);
		}
		else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
		{
			sprintf(szBuf1, "%s (C)", szBuff);
		}
		else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
				 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
		{
			sprintf(szBuf1, "%s (F)", szBuff);
		}
		else if(glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS)//bug806
		{
		    sprintf(szBuf1, "%s (T)", szBuff);
		}
		else
		{
			sprintf(szBuf1, "%s (M)", szBuff);
		}
		PrnSetNormal();
		memset(szBuff, 0, sizeof(szBuff));
		sprintf((char *)szBuff, "%-23.23s", szBuf1);
		MultiLngPrnStr(szBuff,GUI_ALIGN_LEFT);

		// print expiry
		if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
		{
			if( ChkIssuerOption(ISSUER_MASK_EXPIRY) )
			{
				PrnStr(" **/**");
			}
			else
			{
				memset(szBuff, 0, sizeof(szBuff));
				sprintf((char *)szBuff, " %2.2s/%2.2s", &glProcInfo.stTranLog.szExpDate[2],
						glProcInfo.stTranLog.szExpDate);
				MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
			}
		}

		PrnStr("\n");

		// Holder name
		sprintf((char *)szBuff, "%s\n", _T("HOLDER"));
		PrnSetSmall();
		MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
		PrnSetNormal();
		memset(szBuff, 0, sizeof(szBuff));
		sprintf((char *)szBuff, "%-23.23s\n", glProcInfo.stTranLog.szHolderName);
		MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);

		// Transaction type
		sprintf((char *)szBuff, "%s\n", _T("TRANS. TYPE"));
		PrnSetSmall();
		MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);

		sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel);
		if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
		{
			sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);
			sprintf((char *)szBuff, "%s(%s)", _T(szTranName), _T("VOID"));
		}
		else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
		{
			sprintf((char *)szBuff, "%s(%s)", _T(szTranName), _T("ADJ"));
		}
		else
		{
			sprintf((char *)szBuff, "%s", szTranName);
		}
		PrnSetNormal();
		MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
		PrnStr("\n");

		// Batch, Invoice
		sprintf((char *)szBuff, "%s\n", _T("BATCH NO.              TRACE NO."));
		PrnSetSmall();
		MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);

		sprintf((char *)szBuff, "%06lu          %06lu\n", glPosParams.batchNo, glProcInfo.stTranLog.ulInvoiceNo);
		PrnSetNormal();
		MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);

		// Date, time
		sprintf((char *)szBuff, "%s\n", _T("DATE/TIME                       "));
		PrnSetSmall();
		MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);

		Conv2EngTime(glProcInfo.stTranLog.szDateTime, szBuff);
		PrnSetNormal();
		memset(szBuf1, 0, sizeof(szBuf1));
		sprintf((char *)szBuf1, "%-22.22s\n", szBuff);
		MultiLngPrnStr(szBuf1, GUI_ALIGN_LEFT);

		// REF, APPV
		sprintf((char *)szBuff, "%s\n", _T("REF. NO.               APP. CODE"));
		PrnSetSmall();
		MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
		PrnSetNormal();
		memset(szBuf1, 0, sizeof(szBuf1));
		sprintf((char *)szBuf1, "%-14.14s  %-12.12s\n", glProcInfo.stTranLog.szRRN, glProcInfo.stTranLog.szAuthCode);
		MultiLngPrnStr(szBuf1, GUI_ALIGN_LEFT);

		PrnStr("\n");

		if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
		{
			memset(szBuf1, 0, sizeof(szBuf1));
			sprintf((char *)szBuf1, "APP:%.16s\n", glProcInfo.stTranLog.szAppLabel);
			MultiLngPrnStr(szBuf1, GUI_ALIGN_LEFT);

			PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff);
			PubTrimTailChars(szBuff, 'F');
			memset(szBuf1, 0, sizeof(szBuf1));
			sprintf((char *)szBuf1, "AID:%.32s\n", szBuff);
			MultiLngPrnStr(szBuf1, GUI_ALIGN_LEFT);

			PubBcd2Asc0(glProcInfo.stTranLog.sAppCrypto, 8, szBuff);
			memset(szBuf1, 0, sizeof(szBuf1));
			sprintf((char *)szBuf1, "TC: %.16s\n", szBuff);
			MultiLngPrnStr(szBuf1, GUI_ALIGN_LEFT);

#ifdef ENABLE_EMV
#ifdef APP_DEBUG
 			PubBcd2Asc0(glProcInfo.stTranLog.sTSI, 2, szBuff);
 			PrnStr("   TSI: %s\n", szBuff);
 			PubBcd2Asc0(glProcInfo.stTranLog.sTVR, 5, szBuff);
 			PrnStr("   TVR: %s\n", szBuff);
#endif
#endif
			if( glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN )
			{
				MultiLngPrnStr("   PIN VERIFIED\n", GUI_ALIGN_LEFT);
			}
			else
			{
				PrnStr("\n");
			}
		}

		PrnDescriptor();

		// amount
		PrnAmount((uchar *)"", TRUE);  //Print amount

		PrnAdditionalPrompt();

		PrnStatement();

		if( ucPrnFlag==PRN_REPRINT )
		{
			sprintf((char *)szBuff, "%s\n", _T("         * REPRINT *"));
			MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
		}

		if( ucNum==0 )
		{
		    if(HasE_Signature()){
				unsigned char* gSigToPrn = malloc(20000 * sizeof(unsigned char));
		        int ret = PrnBmp(glProcInfo.stTranLog.szSignPath, 0, 0, gSigToPrn);
				free(gSigToPrn);
                /*if(ret == 0){
                    PrnLogo(gSigToPrn);
                }*/
		    }
		    else {
				sprintf((char *)szBuff, "%s\n", _T("CARDHOLDER SIGNATURE"));
				PrnSetSmall();
				MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
				PrnSetNormal();
				PrnStr("\n\n\n\n");
				PrnStr("-----------------------------\n");
				PrnSetSmall();
				MultiLngPrnStr("I ACKNOWLEDGE SATISFACTORY RECEIPT OF RELATIVE  GOODS/SERVICE\n", GUI_ALIGN_LEFT);
			}
		}

#ifdef ENABLE_EMV
#ifdef EMV_TEST_VERSION
			sprintf((char *)szBuff, "%s\n", _T("  ** FOR EMV TEST ONLY **"));
			MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
#endif
#endif

        PrnSetNormal();
		if( ucNum==0 )
		{
			MultiLngPrnStr(_T("  **** MERCHANT COPY ****  "), GUI_ALIGN_LEFT);
		}
		else if( ucNum==1 )
		{
			MultiLngPrnStr(_T("  **** CUSTOMER COPY ****  "), GUI_ALIGN_LEFT);
		}
		else if( ucNum==2 )
		{
			MultiLngPrnStr(_T("  **** BANK COPY ****  "), GUI_ALIGN_LEFT);
		}
		PrnStr("\n\n\n\n\n\n\n\n");

		StartPrinter();

		if( ucNum==0 && NumOfReceipt() != 1)
		{
            kbflush();

			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PRESS ANY KEY"), gl_stCenterAttr, GUI_BUTTON_NONE, USER_OPER_TIMEOUT, NULL);
		}
	}

	return 0;
}

void PrnHead(uchar ucFreeFmat)
{
	uchar	szBuff[32];

	if (true)
	{
		PrnStr((uchar *)_T("DEMONSTRATE ONLY\n"));
		sprintf(szBuff, "%.30s", _T("NOT FOR PAYMENT PROOF"));
		if( szBuff[0]>=0xA0 )
		{
			PrnStr("%.30s\n", szBuff);
		}
		else
		{
			PrnStr("%.30s\n\n", szBuff);
		}
	} 
	else
	{
		PrnStr("%.23s\n", glSysParam.stEdcInfo.szMerchantName);

		if( glSysParam.stEdcInfo.szMerchantAddr[0]>=0xA0 )
		{
			PrnStr("%.23s\n", glSysParam.stEdcInfo.szMerchantAddr);
		}
		else
		{
			PrnStr("%.23s\n%.23s\n", glSysParam.stEdcInfo.szMerchantAddr,
					&glSysParam.stEdcInfo.szMerchantAddr[23]);
		}
	}

	PrnStep(15);

	if (ucFreeFmat)
	{
		GetEngTime(szBuff);
		PrnStr("MID:%-15.15s      %5.5s\n",   glPosParams.merchantId, szBuff+11);
		PrnStr("TID:%-8.8s        %10.10s\n", glPosParams.terminalId, szBuff);
	}
	else
	{
		PrnStr("%14.8s\n%21.15s\n", glPosParams.terminalId, glPosParams.merchantId);
	}
	PrnStep(15);
}


int PrnFontSetNew(uchar ucEnType, uchar ucSQType)
{
#if defined(_Sxx_) || defined(_Sxxx_)
	int	iRet;
	ST_FONT font1,font2;

	font1.CharSet = CHARSET_WEST;
	font1.Width   = 8;
	font1.Height  = 16;
	font1.Bold    = 0;
	font1.Italic  = 0;

	font2.CharSet = glSysParam.stEdcInfo.stLangCfg.ucCharSet;
	font2.Width   = 16;
	font2.Height  = 16;
	font2.Bold    = 0;
	font2.Italic  = 0;

	if (ucEnType==PRN_6x8)
	{
		font1.Width   = 6;
		font1.Height  = 8;
	}
	if (ucEnType==PRN_6x12)
	{
		font1.Width   = 6;
		font1.Height  = 12;
	}
	if (ucEnType==PRN_12x24)
	{
		font1.Width   = 12;
		font1.Height  = 24;
	}

	if (ucSQType==PRN_12x12)
	{
		font2.Width   = 12;
		font2.Height  = 12;
	}
	if (ucSQType==PRN_24x24)
	{
		font2.Width   = 24;
		font2.Height  = 24;
	}

	// in WIN32 do not allow NULL
#ifndef WIN32
	if (font1.CharSet==font2.CharSet)
	{
		iRet = PrnSelectFont(&font1,NULL);
	} 
	else
#endif
	{
		iRet = PrnSelectFont(&font1, NULL);
		if(0 == iRet)
			iRet = PrnSelectFont(NULL, &font2);
	}
	PrnDoubleWidth(0, 0);
	PrnDoubleHeight(0, 0);

	if ((iRet!=0) && (font1.Width>=12) && (font2.Width>=12))
	{
		font1.Width /= 2;
		font1.Height /= 2;
		font2.Width /= 2;
		font2.Height /= 2;
		iRet = PrnSelectFont(&font1, NULL);
		if(0 == iRet)
			iRet = PrnSelectFont(NULL, &font2);

		if (iRet==0)
		{
			PrnDoubleWidth(1, 1);
			PrnDoubleHeight(1, 1);
		}		
	}

	return iRet;

#else
	if (ucEnType==PRN_6x8)
	{
		PrnFontSet(0, 0);
	}
	else
	{
		PrnFontSet(1, 1);
	}

	return 0;
	
#endif
}

// for thermal only
// void PrnSmallConstStr(uchar *str)
// {
// 	PrnSetSmall();
// 	PrnStr(str);
// 	PrnSetNormal();
// }

int PrnCustomLogo_T(void)
{
	uchar	*psLogoData;
	int		iWidth, iHeigh;

	psLogoData = NULL;
	GetNowPrnLogo(&psLogoData);
	if (psLogoData!=NULL)
	{
		iWidth = 0;
		iHeigh = 0;
		GetLogoWidthHeigh(psLogoData, &iWidth, &iHeigh);
		if (iWidth<384)
		{
			iWidth = (384-iWidth)/2;	// let logo be printed at center
		}
		else
		{
			iWidth = 0;
		}
		PrnLeftIndent(100);
		PrnSetNormal();
		PrnLogo(psLogoData);
		PrnLeftIndent(0);
		PrnStr("\n");
		return 0;
	}

	return -1;
}

void PrnHead_T(void)
{
	uchar	szBuff[32];

	PrnStep(30);
	PrnSetNormal();

	if (true)
	{
		MultiLngPrnStr(_T("* DEMONSTRATE ONLY *"), GUI_ALIGN_LEFT);
		PrnStr("\n");
		PrnStep(10);
		
		memset(szBuff, 0, sizeof(szBuff));
		sprintf(szBuff, "%.30s", _T("* NOT FOR PAYMENT PROOF *"));
		if( szBuff[0]>=0xA0 )
		{
			MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
			PrnStr("\n");
		}
		else
		{
			MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
			PrnStr("\n\n");
		}
		PrnStep(15);
	}
	else
	{
		memset(szBuff, 0, sizeof(szBuff));
		sprintf(szBuff, "%.23s\n", glSysParam.stEdcInfo.szMerchantName);
		MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
		PrnStep(10);

		if (strlen(glSysParam.stEdcInfo.szMerchantAddr)>23)
		{
			memset(szBuff, 0, sizeof(szBuff));
			sprintf(szBuff, "%.23s\n", glSysParam.stEdcInfo.szMerchantAddr);
			PrnSetSmall();
			MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
			sprintf(szBuff, "%.23s", glSysParam.stEdcInfo.szMerchantAddr+23);
			MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);
		}
		else
		{
			PrnSetSmall();
			PrnStr(glSysParam.stEdcInfo.szMerchantAddr);
			PrnStr("\n");
		}
		PrnSetNormal();
		PrnStr("\n");
		PrnStep(15);
	}


	PrnSetSmall();
	MultiLngPrnStr(_T("MERCHANT ID.        "), GUI_ALIGN_LEFT);
	PrnSetNormal();
	memset(szBuff, 0, sizeof(szBuff));
	sprintf(szBuff,"%15.15s\n", glPosParams.merchantId);
	MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);

	PrnSetSmall();
	MultiLngPrnStr(_T("TERMINAL ID.        "), GUI_ALIGN_LEFT);
	PrnSetNormal();
	memset(szBuff, 0, sizeof(szBuff));
	sprintf(szBuff,"%15.15s\n", glPosParams.terminalId);
	MultiLngPrnStr(szBuff, GUI_ALIGN_LEFT);

	MultiLngPrnStr("=============================\n", GUI_ALIGN_LEFT);
	PrnStep(15);
}

void PrnAmount(const uchar *pszIndent, uchar ucNeedSepLine)
{
	uchar	szBuff[50], szTotalAmt[12+1];
	uchar   szTempBuff[100];

    if (ChkIfNeedTip() && !(glProcInfo.stTranLog.uiStatus & TS_VOID))
	{
		//-------------------------------- BASE --------------------------------
		memset(szTempBuff, 0, sizeof(szTempBuff));
		App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, 0);
		if (ChkIfThermalPrinter())
		{
			sprintf((char *)szTempBuff, "%s%s      %s\n", _T(pszIndent), _T("BASE"), szBuff);
		}
		else
		{
			sprintf((char *)szTempBuff, "%sBASE      %s\n", pszIndent, szBuff);
		}
		MultiLngPrnStr(szTempBuff, GUI_ALIGN_LEFT);

		//-------------------------------- TIPS --------------------------------
		memset(szTempBuff, 0, sizeof(szTempBuff));
		if( !ChkIfZeroAmt(glProcInfo.stTranLog.szOtherAmount) )
		{
			App_ConvAmountTran(glProcInfo.stTranLog.szOtherAmount, szBuff, 0);
			if (ChkIfThermalPrinter())
			{
				sprintf((char *)szTempBuff, "%s%s      %s\n", _T(pszIndent), _T("TIPS"), szBuff);
			}
			else
			{
				sprintf((char *)szTempBuff, "%sTIPS      %s\n", pszIndent, szBuff);
			}
		}
		else
		{
			if (ChkIfThermalPrinter())
			{
				sprintf((char *)szTempBuff, "%s%s\n", _T(pszIndent), _T("TIPS"), szBuff);
			}
			else
			{
				sprintf((char *)szTempBuff, "%sTIPS\n", pszIndent);
			}
		}
		MultiLngPrnStr(szTempBuff, GUI_ALIGN_LEFT);

		//-------------------------------- TOTAL --------------------------------
		memset(szTempBuff, 0, sizeof(szTempBuff));
		if( ucNeedSepLine )
		{
			sprintf((char *)szTempBuff, "%s          -----------------\n", pszIndent);
			MultiLngPrnStr(szTempBuff, GUI_ALIGN_LEFT);
		}

		memset(szTempBuff, 0, sizeof(szTempBuff));
		if( !ChkIfZeroAmt(glProcInfo.stTranLog.szOtherAmount) )
		{
			PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szOtherAmount, 12, szTotalAmt);
			App_ConvAmountTran(szTotalAmt, szBuff, 0);
			if (ChkIfThermalPrinter())
			{
				sprintf((char *)szTempBuff, "%s%s     %s\n", _T(pszIndent), _T("TOTAL"), szBuff);
			}
			else
			{
				sprintf((char *)szTempBuff, "%sTOTAL     %s\n", pszIndent, szBuff);
			}
		}
		else
		{
			if (ChkIfThermalPrinter())
			{
				sprintf((char *)szTempBuff, "%s%s\n", _T(pszIndent), _T("TOTAL"), szBuff);
			} 
			else
			{
				sprintf((char *)szTempBuff, "%sTOTAL\n", pszIndent);
			}
		}
		MultiLngPrnStr(szTempBuff, GUI_ALIGN_LEFT);

		if( ucNeedSepLine )
		{
			memset(szTempBuff, 0, sizeof(szTempBuff));
			sprintf((char *)szTempBuff, "%s          =================\n", pszIndent);
			MultiLngPrnStr(szTempBuff, GUI_ALIGN_LEFT);
		}
	}
	else
	{
		App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));

		memset(szTempBuff, 0, sizeof(szTempBuff));
		if (ChkIfThermalPrinter())
		{
			sprintf((char *)szTempBuff, "%s%s     %s\n", _T(pszIndent), _T("TOTAL"), szBuff);
		}
		else
		{
			sprintf((char *)szTempBuff, "%sTOTAL     %s\n", pszIndent, szBuff);
		}
		MultiLngPrnStr(szTempBuff, GUI_ALIGN_LEFT);

		if( ucNeedSepLine )
		{
			memset(szTempBuff, 0, sizeof(szTempBuff));
			sprintf((char *)szTempBuff, "%s          =================\n", pszIndent);
			MultiLngPrnStr(szTempBuff, GUI_ALIGN_LEFT);
		}
	}
}

// print product descriptor
void PrnDescriptor(void)
{
	uchar	ucCnt, ucMaxNum, ucTemp;
	uchar	szBuf[50];

	if( ChkIfDccAcquirer() )
	{
		return;
	}

	ucMaxNum = (uchar)MIN(MAX_GET_DESC, glProcInfo.stTranLog.ucDescTotal);
	for(ucCnt=0; ucCnt<ucMaxNum; ucCnt++)
	{
		ucTemp = glProcInfo.stTranLog.szDescriptor[ucCnt] - '0';
		PubASSERT( ucTemp<MAX_DESCRIPTOR );
		memset(szBuf, 0, sizeof(szBuf));
		sprintf((char *)szBuf, "   %-20.20s\n", glSysParam.stDescList[ucTemp].szText);
		MultiLngPrnStr(szBuf, GUI_ALIGN_LEFT);
	}

	if (!ChkIfThermalPrinter())
	{
		if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) || ChkEdcOption(EDC_FREE_PRINT) )
		{
			PrnStr("\n");
		}
		else
		{
			for(; ucCnt<MAX_GET_DESC; ucCnt++)
			{
				PrnStr("\n");
			}
		}
	}
}

void PrnAdditionalPrompt(void)
{
	uchar	szBuf[50];

	memset(szBuf, 0, sizeof(szBuf));
	sprintf((char *)szBuf, "%-14.14s%16.16s\n", glSysParam.stEdcInfo.szAddlPrompt, glProcInfo.stTranLog.szAddlPrompt);
	MultiLngPrnStr(szBuf, GUI_ALIGN_LEFT);
}

void PrnStatement(void)
{

}

// print total information of ucIssuerKey
int PrnTotalIssuer(uchar ucIssuerKey)
{

	return 0;
}

// print total information
void PrnTotalInfo(const void *pstInfo)
{
	uchar		szBuff[50], szBaseAmt[20];
	TOTAL_INFO	*pstTotal;

	pstTotal = (TOTAL_INFO *)pstInfo;
	PubAscSub(pstTotal->szSaleAmt, pstTotal->szTipAmt, 12, szBaseAmt);
	App_ConvAmountTran(szBaseAmt, szBuff, 0);
	PrnStr("BASE :%-03d %17s\n", pstTotal->uiSaleCnt, szBuff);

	App_ConvAmountTran(pstTotal->szTipAmt, szBuff, 0);
	PrnStr("TIPS :%-03d %17s\n", pstTotal->uiTipCnt, szBuff);

	App_ConvAmountTran(pstTotal->szSaleAmt, szBuff, 0);
	PrnStr("SALES:%-03d %17s\n", pstTotal->uiSaleCnt, szBuff);

	App_ConvAmountTran(pstTotal->szRefundAmt, szBuff, GA_NEGATIVE);
	PrnStr("REFND:%-03d %17s\n", pstTotal->uiRefundCnt, szBuff);

	App_ConvAmountTran(pstTotal->szVoidSaleAmt, szBuff, GA_NEGATIVE);
	PrnStr("VOIDED SALES :%-03d\n%27s\n", pstTotal->uiVoidSaleCnt, szBuff);

	App_ConvAmountTran(pstTotal->szVoidRefundAmt, szBuff, 0);
	PrnStr("VOIDED REFUND:%-03d\n%27s\n\n", pstTotal->uiVoidRefundCnt, szBuff);
}

int PrnTotalAcq(void)
{
	uchar	ucCnt;
	int		iRet;

	PrnInit();
	PrnSetNormal();
	
	PrnStep(30);
	PrnStr("  TRANS TOTALS BY CARD\n");
	PrnEngTime();
	PrnStep(20);
	if( ChkIfThermalPrinter() )
	{
		PrnStr("TID: %s\nMID: %s\n", glPosParams.terminalId, glPosParams.merchantId);
		PrnStep(15);				
	}
	else
	{
		PrnStr("%14.8s\n%21.15s\n", glPosParams.terminalId, glPosParams.merchantId);
		PrnStep(15);
	}

	PrnStr("   END  OF  TOTAL\n");
	PrnStr("%s", (ChkIfThermalPrinter() ? "\n\n\n\n\n\n" : "\f"));

	StartPrinter();
	return 0;
}

int PrintSettle(uchar ucPrnFlag)
{
	return 0;
}

void PrnEngTime(void)
{
	uchar	szDateTime[14+1], szBuff[30];

	GetDateTime(szDateTime);
	Conv2EngTime(szDateTime, szBuff);
	PrnStr("%s\n", szBuff);
}

// Print parameter
int PrintParam(void)
{
	uchar	ucCnt;

	SetCurrTitle(_T("PRINT PARAMETER"));
	if( PasswordTerm()!=0 )
	{
		return ERR_NO_DISP;
	}

	SetOffBase(OffBaseCheckPrint);

	Gui_ClearScr();
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	PrnInit();
	PrnSetNormal();

	PrnStr("\n\n%.23s\n", glSysParam.stEdcInfo.szMerchantName);
	PrnStr("%.23s\n",     glSysParam.stEdcInfo.szMerchantAddr);
	PrnStr("%.23s\n",     glSysParam.stEdcInfo.szMerchantAddr+23);
	PrnStr("INIT TID: %.8s\n", glSysParam.stEdcInfo.szDownLoadTID);
	PrnHexString("EDC OPTION:", glSysParam.stEdcInfo.sOption, 5, TRUE);

	PrnEngTime();
	PrnStr("APP VERSION: %s\n", EDC_VER_PUB);

	if (glSysParam.ucDescNum)
	{
		PrnStr("DESCRIPTION:\n");
		for(ucCnt=0; ucCnt<glSysParam.ucDescNum; ucCnt++)
		{
			PrnStr("   %.2s:%.20s\n", glSysParam.stDescList[ucCnt].szCode,
					glSysParam.stDescList[ucCnt].szText);
		}
	}

	PrnInstalmentPara();

	if( StartPrinter()!=0 )
	{
		return ERR_NO_DISP;
	}

	if (!ChkIfEmvEnable())
	{
		return 0;
	}

#ifdef ENABLE_EMV
	Gui_ClearScr();
	if(GUI_OK == Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PRN EMV PARA ?"), gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL)){
		PrnEmvPara();
	}
#endif
	return 0;
}

int PrnParaAcq(uchar ucAcqIndex)
{

	return 0;
}

void PrnParaIssuer(uchar ucAcqIndex)
{

}

void PrnParaIssuerSub(uchar ucIssuerKey)
{

}

void PrnIssuerOption(const uchar *psOption)
{
	static	OPTION_INFO	stIssuerOptList[] =
	{
// 		{"ENABLE BALANCE",		ISSUER_EN_BALANCE},
		{"ENABLE ADJUST",		ISSUER_EN_ADJUST,			FALSE,	PM_MEDIUM},
		{"ENABLE OFFLINE",		ISSUER_EN_OFFLINE,			FALSE,	PM_MEDIUM},
		{"EN. (PRE)AUTH",		ISSUER_NO_PREAUTH,			TRUE,	PM_MEDIUM},
		{"EN. REFUND",			ISSUER_NO_REFUND,			TRUE,	PM_MEDIUM},
		{"EN. VOID",			ISSUER_NO_VOID,				TRUE,	PM_MEDIUM},
// 		{"ENABLE EXPIRY",		ISSUER_EN_EXPIRY,			FALSE,	PM_MEDIUM},
// 		{"CHECK EXPIRY",		ISSUER_CHECK_EXPIRY,		FALSE,	PM_MEDIUM},
// 		{"CHKEXP OFFLINE",		ISSUER_CHECK_EXPIRY_OFFLINE,FALSE,	PM_MEDIUM},
// 		{"CHECK PAN",			ISSUER_CHKPAN_MOD10,		FALSE,	PM_MEDIUM},
// 		{"EN DISCRIPTOR",		ISSUER_EN_DISCRIPTOR,		FALSE,	PM_MEDIUM},
		{"ENABLE MANUAL",		ISSUER_EN_MANUAL,			FALSE,	PM_MEDIUM},
// 		{"ENABLE PRINT",		ISSUER_EN_PRINT,			FALSE,	PM_MEDIUM},
		{"VOICE REFERRAL",		ISSUER_EN_VOICE_REFERRAL,	FALSE,	PM_MEDIUM},
// 		{"PIN REQUIRED",		ISSUER_EN_PIN,				FALSE,	PM_MEDIUM},
// 		{"ACCOUNT SELECT",		ISSUER_EN_ACCOUNT_SELECTION,FALSE,	PM_MEDIUM},
// 		{"ROC INPUT REQ",		ISSUER_ROC_INPUT_REQ,		FALSE,	PM_MEDIUM},
// 		{"DISP AUTH CODE",		ISSUER_AUTH_CODE,			FALSE,	PM_MEDIUM},
// 		{"ADDTIONAL DATA",		ISSUER_ADDTIONAL_DATA,		FALSE,	PM_MEDIUM},
		{"SECURITY CODE",		ISSUER_SECURITY_SWIPE,		FALSE,	PM_MEDIUM},
		{"SECU. CODE MANUL",	ISSUER_SECURITY_MANUL,		FALSE,	PM_MEDIUM},
		{NULL, 0, FALSE, PM_MEDIUM},
	};
	uchar	ucCnt;

	for(ucCnt=0; stIssuerOptList[ucCnt].pText!=NULL; ucCnt++)
	{
		if( (!stIssuerOptList[ucCnt].ucInverseLogic && ChkOptionExt(psOption, stIssuerOptList[ucCnt].uiOptVal)) ||
			(stIssuerOptList[ucCnt].ucInverseLogic && !ChkOptionExt(psOption, stIssuerOptList[ucCnt].uiOptVal)) )
		{
			PrnStr("    %-16.16s[ on]\n", stIssuerOptList[ucCnt].pText);
		}
		else
		{
			PrnStr("    %-16.16s[off]\n", stIssuerOptList[ucCnt].pText);
		}
	}
}

void PrnCardTable(uchar ucIssuerKey)
{
	uchar	ucCnt, szBuff[30];

	if( ucIssuerKey==INV_ISSUER_KEY )
	{
		return;
	}

	for(ucCnt=0; ucCnt<glSysParam.ucCardNum; ucCnt++)
	{
		if( glSysParam.stCardTable[ucCnt].ucIssuerKey==ucIssuerKey )
		{
			PubBcd2Asc0(glSysParam.stCardTable[ucCnt].stCardRange[0].sPanRangeLow, 5, szBuff);
			szBuff[10] = '~';
			PubBcd2Asc0(glSysParam.stCardTable[ucCnt].stCardRange[0].sPanRangeHigh, 5, &szBuff[11]);
			PrnStr("%s\n", szBuff);
			if ( strlen(glSysParam.stCardTable[ucCnt].stCardRange[1].sPanRangeLow) != 0 )
			{
				memset(szBuff, 0, sizeof(szBuff));
				PubBcd2Asc0(glSysParam.stCardTable[ucCnt].stCardRange[1].sPanRangeLow, 5, szBuff);
				szBuff[10] = '~';
				PubBcd2Asc0(glSysParam.stCardTable[ucCnt].stCardRange[1].sPanRangeHigh, 5, &szBuff[11]);
				PrnStr("%s\n", szBuff);
			}
		}
	}
}

int PrnInstalmentPara(void)
{
	return 0;
}

#ifdef ENABLE_EMV

// Print EMV parameter
int PrnEmvPara(void)
{
	int			iRet, iCnt;
	EMV_APPLIST	stEmvApp;
	EMV_CAPK	stEmvCapk;

	PrnInit();
	PrnSetNormal();

	PrnStr("\n=========EMV PARAMETER=======\n");
	EMVGetParameter(&glEmvParam);
	PrnStr("TERMINAL TYPE: %02X\n", glEmvParam.TerminalType);
	PrnHexString("TERMINAL CAPA:",  glEmvParam.Capability, 3, TRUE);
	PrnHexString("TERM EX-CAPA :",  glEmvParam.ExCapability, 5, TRUE);
	PrnStr("TXN CURR EXP : %02X\n", glEmvParam.TransCurrExp);
	PrnStr("REF CURR EXP : %02X\n", glEmvParam.ReferCurrExp);
	PrnHexString("REF CURR CODE:", glEmvParam.ReferCurrCode, 2, TRUE);
	PrnHexString("COUNTRY CODE :", glEmvParam.CountryCode, 2, TRUE);
	PrnHexString("TXN CURR CODE:", glEmvParam.TransCurrCode, 2, TRUE);
	PrnStr("REF CURR CON : %ld\n", glEmvParam.ReferCurrCon);
	PrnStr("SELECT PSE   : %s\n",  glEmvParam.SurportPSESel ? "YES" : "NO");

	PrnStr("\n\n\n========EMV APP LIST=========\n");
	for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
	{
		memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
		iRet = EMVGetApp(iCnt, &stEmvApp);
		if( iRet!=EMV_OK )
		{
			continue;
		}
		PrnHexString("AID:",  stEmvApp.AID, (int)stEmvApp.AidLen, TRUE);
		PrnHexString("VERSION:",  stEmvApp.Version, 2, TRUE);
		PrnStr("SELECT FLAG   : %s MATCH\n", stEmvApp.SelFlag==FULL_MATCH ? "FULL" : "PARTIAL");
		PrnStr("PRIORITY      : %d\n", stEmvApp.Priority);
		PrnStr("TARGET PER    : %d\n", stEmvApp.TargetPer);
		PrnStr("MAX TARGET PER: %d\n", stEmvApp.MaxTargetPer);
		PrnStr("CHECK FLOOR   : %s\n", stEmvApp.FloorLimitCheck ? "YES" : "NO");
		PrnStr("RANDOM SELECT : %s\n", stEmvApp.RandTransSel    ? "YES" : "NO");
		PrnStr("CHECK VELOCITY: %s\n", stEmvApp.VelocityCheck   ? "YES" : "NO");
		PrnStr("FLOOR LIMIT   : %lu\n", stEmvApp.FloorLimit);
		PrnStr("THRESHOLD     : %lu\n", stEmvApp.Threshold);
		PrnHexString("TAC DENIAL :",  stEmvApp.TACDenial,  5, TRUE);
		PrnHexString("TAC ONLINE :",  stEmvApp.TACOnline,  5, TRUE);
		PrnHexString("TAC DEFAULT:",  stEmvApp.TACDefault, 5, TRUE);
		PrnStr("-----------------------------\n");
		if( (iCnt%5)==0 )
		{
			if( StartPrinter()!=0 )
			{
				return 1;
			}
			PrnInit();
			PrnSetNormal();
		}
	}

	PrnStr("\n\n=========EMV CAPK LIST========\n");
	for(iCnt=0; iCnt<MAX_KEY_NUM; iCnt++)
	{
		memset(&stEmvCapk, 0, sizeof(EMV_CAPK));
		iRet = EMVGetCAPK(iCnt, &stEmvCapk);
		if( iRet!=EMV_OK )
		{
			continue;
		}
		PrnHexString("RID:",  stEmvCapk.RID, 5, FALSE);
		PrnStr(" ID: %02X\n",  stEmvCapk.KeyID);
// 		PrnStr("HASH   : %02X\n",  stEmvCapk.HashInd);
// 		PrnStr("ARITH  : %02X\n",  stEmvCapk.ArithInd);
		PrnHexString("EXP DATE:",  stEmvCapk.ExpDate, 3, TRUE);
		PrnStr("MOD LEN: %d ",  (int)(8 * stEmvCapk.ModulLen));
		PrnHexString("EXPONENT:",  stEmvCapk.Exponent, (int)stEmvCapk.ExponentLen, TRUE);
		PrnStr("-----------------------------\n");
		if( (iCnt%5)==0 )
		{
// 			PrnStr("\f");
			if( StartPrinter()!=0 )
			{
				return 1;
			}
			PrnInit();
			PrnSetNormal();
		}
	}
	PrnStr("\f");
	return StartPrinter();
}


void PrintEmvErrLogSub(void)
{
	ushort			uiCnt, uiActNum, uiTemp;
	uchar			szBuff[50];
	EMV_ERR_LOG		stErrLog;

	PrnInit();
	PrnSetNormal();
	MultiLngPrnStr(_T("EMV ERROR LOG"), GUI_ALIGN_CENTER);
	PrnStr("\n\n");

	for(uiActNum=uiCnt=0; uiCnt<MAX_ERR_LOG; uiCnt++)
	{
		memset(&stErrLog, 0, sizeof(EMV_ERR_LOG));
		LoadErrLog(uiCnt, &stErrLog);
		if( !stErrLog.bValid )
		{
			continue;
		}

		uiActNum++;
		PrnStr("\nSTAN: %06lu\n", stErrLog.ulSTAN);
		PubBcd2Asc0(stErrLog.sAID, stErrLog.ucAidLen, szBuff);
		PrnStr("AID: %s\n", szBuff);
		PrnStr("PAN: %s\n", stErrLog.szPAN);
		PrnStr("PAN SEQ #: %02X\n", stErrLog.ucPANSeqNo);
		PrnStr("AMT: %.12s\n", stErrLog.szAmount);
		PrnStr("TIP: %.12s\n", stErrLog.szTipAmt);
		PrnStr("RSP: %.2s\n",  stErrLog.szRspCode);
		PrnStr("RRN: %.12s\n", stErrLog.szRRN);
		PrnStr("AUT: %.6s\n",  stErrLog.szAuthCode);
		PrnStr("TVR: %02X %02X %02X %02X %02X\n", stErrLog.sTVR[0], stErrLog.sTVR[1],
					   stErrLog.sTVR[2], stErrLog.sTVR[3], stErrLog.sTVR[4]);
		PrnStr("TSI: %02X %02X\n", stErrLog.sTSI[0], stErrLog.sTSI[1]);

		PrnStr("REQ BIT 55:\n");
		for(uiTemp=0; uiTemp<stErrLog.uiReqICCDataLen; uiTemp++)
		{
			PrnStr("%02X %s", stErrLog.sReqICCData[uiTemp], (uiTemp%8)==7 ? "\n" : "");
		}
		if(uiTemp>0)
		{
			PrnStr("\n");
		}

		PrnStr("REQ BIT 56:\n");
		for(uiTemp=0; uiTemp<stErrLog.uiReqField56Len; uiTemp++)
		{
			PrnStr("%02X %s", stErrLog.sReqField56[uiTemp], (uiTemp%8)==7 ? "\n" : "");
		}
		if(uiTemp>0)
		{
			PrnStr("\n");
		}

		PrnStr("RSP BIT 55:\n");
		for(uiTemp=0; uiTemp<stErrLog.uiRspICCDataLen; uiTemp++)
		{
			PrnStr("%02X %s", stErrLog.sRspICCData[uiTemp], (uiTemp%8)==7 ? "\n" : "");
		}
		if(uiTemp>0)
		{
			PrnStr("\n");
		}

		if( (uiActNum%5)==4 )
		{
			if( StartPrinter()!=0 )
			{
				return;
			}

			PrnInit();
			PrnSetNormal();
		}
	}

	if (uiActNum>0)
	{
		PrnStr("%s", (ChkIfThermalPrinter() ? "\n" : "\f"));
	}
	else
	{
		PrnStr("\n  ( NO RECORD )");
	}

	StartPrinter();
}
#endif


void PrnHexString(const char *pszTitle, const uchar *psHexStr, int iLen, uchar bNewLine)
{
	int		iCnt;

	PrnStr("%s", pszTitle);
	for(iCnt=0; iCnt<iLen; iCnt++)
	{
		PrnStr(" %02X", psHexStr[iCnt]);
	}
	if (bNewLine)
	{
		PrnStr("\n");
	}
}

// ´òÓ¡´íÎóÌáÊ¾
// Start-up printer, and show error if any.
// Modified by Kim_LinHB 2014-6-8
int StartPrinter(void)
{
	uchar	ucRet;

	if (!ChkIfIrDAPrinter())
	{
		int iRet;
		while( 1 )
		{
			DispPrinting();
			PrintOne();
			ucRet = PrnStart();
			if( ucRet==PRN_OK )
			{
				return 0;	// print success!
			}

			iRet = DispPrnError(ucRet);
			if( ucRet!=ERR_PRN_PAPEROUT )
			{
				break;
			}

			if( GUI_ERR_USERCANCELLED == iRet||
				GUI_ERR_TIMEOUT == iRet)
			{
				Gui_ClearScr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLEASE REPRINT"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
				break;
			}
		}
	}
	else
	{
		int iRet;
		SetOffBase(OffBaseCheckPrint);	//????

		DispPrinting();
		PrnStart();
		PrintOne();
		while( 1 )
		{
			ucRet = PrnStatus();
			if( ucRet==PRN_OK)
			{
				return PRN_OK;
			}
			else if( ucRet==PRN_BUSY )
			{
				DelayMs(500);
				continue;
			}
			
			iRet = DispPrnError(ucRet);
			if( ucRet!=ERR_PRN_PAPEROUT )
			{
				break;
			}

			if( GUI_OK != iRet)
			{
				break;
			}
			DispPrinting();
			PrnStart();
			PrintOne();
		}
	}

	return ERR_NO_DISP;
}

// Modified by Kim_LinHB 2014-6-8
int DispPrnError(int iErrCode)
{
	unsigned char szBuff[100];
	Gui_ClearScr();
	PubBeepErr();
	switch( iErrCode )
	{
	case ERR_PRN_BUSY:
		strcpy(szBuff, _T("PRINTER BUSY"));
		break;

	case ERR_PRN_PAPEROUT:
		strcpy(szBuff, _T("OUT OF PAPER"));
		return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL);
		break;

	case ERR_PRN_WRONG_PACKAGE:
		strcpy(szBuff, _T("PRINTER DATA ERROR"));
		break;

	case ERR_PRN_OVERHEAT:
		strcpy(szBuff, _T("PRINTER OVERHEAT"));
		break;

	case ERR_PRN_OUTOFMEMORY:
		strcpy(szBuff, _T("PRINTER OVERFLOW"));
		break;

	case PRN_NO_FONT:
		strcpy(szBuff, _T("PLEASE LOAD FONT"));
		break;

	default:
		strcpy(szBuff, _T("PRINTER ERROR"));
		break;
	}
	return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
}

/*
int PrnTest(void)
{
	PrnInit();
	PrnSetNormal();

	//...

	StartPrinter();
	return 0;
}
*/

void MultiLngPrnStr(const uchar *str, uchar mode)
{
#ifdef AREA_Arabia
	if(strcmp(LANGCONFIG, "Arabia") == 0)
	{
		if (mode == GUI_ALIGN_LEFT)
		{
			ArPrnAlign(AR_ALIGN_RIGHT);
		}
		else if (mode == GUI_ALIGN_RIGHT)
		{
			ArPrnAlign(AR_ALIGN_LEFT);
		}
		else if (mode == GUI_ALIGN_CENTER)
		{
			ArPrnAlign(AR_ALIGN_CENTER);
		}
		ArPrnPrint((uchar *)str);
	}
	else
	{
		PrnStr((uchar *)str);
	}
#else
	PrnStr((uchar *)str);
#endif
}

// end of file

