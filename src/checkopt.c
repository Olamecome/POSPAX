
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal constants declaration *********************/
/********************** Internal functions declaration *********************/
static uchar ChkIfAcqAvail(uchar ucIndex);

/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/


/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// Check Term Model
uchar ChkTerm(uchar ucTermType)
{
	char sTermInfo[HWCFG_END + 1];
	return (sTermInfo[HWCFG_MODEL]==ucTermType);
}

// Check Term Hardware Config, by Checking the info returned by GetTermInfo() (buffered in glSysParam.sTermInfo[])
// Modified by Kim_LinHB 2014-08-21 v1.01.0004
uchar ChkHardware(uchar ucChkType, uchar ucValue)
{
	PubASSERT(ucChkType<HWCFG_END);
	char sTermInfo[HWCFG_END + 1];
	GetTermInfo(sTermInfo);

#if !defined(_Dxxx_) || defined(_MIPS_)
	return (sTermInfo[ucChkType]==ucValue);	// return value: TRUE/FALSE
#else
	if(HWCFG_BLTH != ucChkType)
	{
		return (sTermInfo[ucChkType]==ucValue);	// return value: TRUE/FALSE
	}
	else
	{
		return ((sTermInfo[19] & 0x08) == ucValue);
	}
#endif
}

// Check whether it is an IrDA-communication printer
uchar ChkIfIrDAPrinter(void)
{
	return (ChkTerm(_TERMINAL_S60_));
}

// 1.00.0009 delete
//uchar ChkIfSupportCommType(uchar ucCommType)

// check if terminal is using the specific PED mode
uchar ChkTermPEDMode(uchar ucMode)
{
	return (glPosParams.ucPedMode==ucMode);
}

// Scan all Acquirer and check whether one of them support EMV
uchar ChkIfEmvEnable(void)
{
	return TRUE;
}

// Check EDC option
uchar ChkEdcOption(ushort uiOption)
{
	return FALSE;
}

// Extension of option checking
uchar ChkOptionExt(const uchar *psExtOpt, ushort uiOption)
{
	return (psExtOpt[uiOption>>8] & (uiOption & 0xFF));
}

void SetOptionExt(uchar *psExtOptInOut, ushort uiOption)
{
	psExtOptInOut[uiOption>>8] |= (uiOption & 0xFF);
}

// Check whether need PIN for current Issuer TRUE need FALSE no need
uchar ChkIfNeedPIN(void)
{
	return TRUE;
}

uchar ChkIfPinReqdAllIssuer(void)
{
	return TRUE;
}

uchar ChkIfAmex(void)
{
	return ChkAcqOption(ACQ_AMEX_SPECIFIC_FEATURE);
}

uchar ChkAcqOption(uchar option) {
	return TRUE;
}

uchar ChkIssuerOption(uchar option) {
	return TRUE;
}


// add by lirz v1.01.0007 
uchar ChkIfAmexName(void)
{
	return ChkCurAcqName("AMEX", FALSE);
}

uchar ChkIfBoc(void)
{
	return ChkCurAcqName("BOC", TRUE);
}

// Citibank HK
uchar ChkIfCiti(void)
{
	return ChkCurAcqName("CITI", TRUE);
}

// Fubon Bank
uchar ChkIfFubon(void)
{
	return ChkCurAcqName("FUBO", TRUE);
}

// If now acquirer is DahSing bank
uchar ChkIfDah(void)
{
	return (ChkCurAcqName("DAHSING", TRUE) || ChkCurAcqName("DSB", TRUE));
}

// Bank of East Asia
uchar ChkIfBea(void)
{
	return ChkCurAcqName("BEA", TRUE);
}

// Standard Chartered
uchar ChkIfScb(void)
{
	return ChkCurAcqName("SCB", TRUE);
}

uchar ChkIfWordCard(void)
{
	return ChkCurAcqName("WORLDCARD", TRUE);
}

uchar ChkIfUob(void)
{
	return false;
//	return ChkCurAcqName("UOB", TRUE);
}

uchar ChkIfUobIpp(void)
{
	return ChkCurAcqName("UOB-IPP", TRUE);
}

// Diners for non-specific bank
uchar ChkIfDiners(void)
{
	return ChkCurAcqName("DINERS", FALSE);
}

// JCB for non-specific bank
uchar ChkIfJCB(void)
{
	return ChkCurAcqName("JCB", FALSE);
}

// ICBC(ASIA) in HK
uchar ChkIfICBC(void)
{
	return ChkCurAcqName("ICBC_HK", FALSE);
}

// ICBC Macau branch
uchar ChkIfICBC_MACAU(void)
{
	return ChkCurAcqName("ICBC_MACAU", FALSE);
}

// Wing Hang Bank
uchar ChkIfWingHang(void)
{
    return ChkCurAcqName("WINGHANG", FALSE);
}

// Shanghai Commercial Bank (HK)
uchar ChkIfShanghaiCB(void)
{
    return ChkCurAcqName("SHCB", TRUE);
}

uchar ChkIfHSBC(void)
{
	if( ChkIfAmex()   || ChkIfBoc()  || ChkIfBea() || ChkIfFubon() ||
		ChkIfDah()    || ChkIfCiti() || ChkIfScb() || ChkIfUob()   ||
		ChkIfUobIpp() || ChkIfWordCard() || ChkIfDiners() || ChkIfJCB() ||
		ChkIfICBC() || ChkIfICBC_MACAU() || ChkIfWingHang() || ChkIfShanghaiCB())
	{
		return FALSE;
	}

	return TRUE;
}

// 检查磁卡交易金额是否低于Floor Limit
// Check whether below floor limit. Only for Non-EMV.
uchar ChkIfBelowMagFloor(void)
{

	return FALSE;
}


uchar ChkInstalmentAllAcq(void)
{
	return TRUE;
}

uchar ChkIfDispMaskPan2(void)
{
	return TRUE;
}

uchar ChkIfInstalmentPara(void)
{
	if( !ChkEdcOption(EDC_ENABLE_INSTALMENT) )
	{
		return FALSE;
	}

	return FALSE;
}

uchar ChkIfTransMaskPan(uchar ucCurPage)
{
	PubASSERT(ucCurPage<4);
	//PubASSERT(ucCurPage>=0 && ucCurPage<4);
	
	return TRUE;
}

// compare acquirer name in upper case
uchar ChkCurAcqName(const void *pszKeyword, uchar ucPrefix)
{

	return FALSE;
}

// Modified by Kim_LinHB 2014-6-8 v1.01.0000
uchar ChkIfTranAllow(uchar ucTranType)
{

	if( GetTranLogNum(ACQ_ALL)>=MAX_TRANLOG )
	{
		// Modified by Kim_LinHB 2014-5-31
		Gui_ClearScr();
		Gui_ShowMsgBox(_T("BATCH FULL"), gl_stTitleAttr, _T("PLS SETTLE BATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return FALSE;
	}

	return TRUE;
}

// Modified by Kim_LinHB 2014-5-31
uchar ChkIfZeroAmt(const uchar *pszIsoAmountStr)
{
	if(!pszIsoAmountStr)
		return TRUE;
	return (memcmp(pszIsoAmountStr, "000000000000",strlen(pszIsoAmountStr))==0);
}

uchar ChkIfBatchEmpty(void)
{
	int	ii;
	for (ii=0; ii<MAX_TRANLOG; ii++)
	{
		return FALSE;
	}
	return TRUE;
}

uchar ChkIfZeroTotal(const void *pstTotal)
{
	TOTAL_INFO	*pTotal = (TOTAL_INFO *)pstTotal;

	if ( pTotal->uiSaleCnt==0 && pTotal->uiRefundCnt==0 &&
		pTotal->uiVoidSaleCnt==0 && pTotal->uiVoidRefundCnt==0 )
	{
		return TRUE;
	}
	return FALSE;
}

uchar ChkSettle(void)
{
	return TRUE;
}

uchar ChkIfNeedTip(void)
{

	return FALSE;
}

uchar ChkIfAcqAvail(uchar ucIndex)
{
	return FALSE;
}

uchar ChkIfDccBOC(void)	// BOC DCC acquirer
{
	return 0;
}

uchar ChkIfDccAcquirer(void)
{
	return ChkCurAcqName("DCC", FALSE);
}

uchar ChkIfDccBocOrTas(void)	// !!!! to be applied.
{
	return (ChkCurAcqName("DCC_BOC", TRUE));
}

uchar ChkIfIccTran(ushort uiEntryMode)
{
	if( (uiEntryMode & MODE_CHIP_INPUT) ||
		(uiEntryMode & MODE_FALLBACK_SWIPE) ||
		(uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		return TRUE;
	}

	return FALSE;
}

uchar ChkIfPrnReceipt(void)
{
	PubASSERT( glProcInfo.stTranLog.ucTranType<MAX_TRANTYPE );
	return (glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct & PRN_RECEIPT);
}

uchar ChkIfNeedReversal(void)
{
	PubASSERT( glProcInfo.stTranLog.ucTranType<MAX_TRANTYPE );
	return (glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct & NEED_REVERSAL);
}

uchar ChkIfSaveLog(void)
{
	PubASSERT( glProcInfo.stTranLog.ucTranType<MAX_TRANTYPE );
	return (glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct & WRT_RECORD);
}

uchar ChkIfThermalPrinter(void)
{
	return TRUE;
}

uchar ChkIfNeedSecurityCode(void)
{
	if( glProcInfo.stTranLog.ucTranType==VOID )
	{
		return FALSE;
	}

	if( ChkIfAmex() )
	{
		// add by lirz v1.01.0007
		if( glProcInfo.stTranLog.ucTranType==REFUND && !ChkAcqOption(ACQ_ONLINE_REFUND) )
		{
			return FALSE;
		}
	   // end add
	}
	else if( glProcInfo.stTranLog.ucTranType==REFUND )
	{
		return FALSE;
	}

	if( (glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT) ||
		(glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) )
	{
		if( ChkIssuerOption(ISSUER_SECURITY_SWIPE) )
		{
			return TRUE;
		}
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_MANUAL_INPUT) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		if( ChkIssuerOption(ISSUER_SECURITY_MANUL) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

uchar ChkIfNeedMac(void)
{
	return FALSE;
}

uchar ChkIfAcqNeedTC(void)
{
	if (ChkIfICBC_MACAU() || ChkIfDah() || ChkIfWingHang())
	{
		return TRUE;
	}
	
	// add by lirz v1.01.0007
	if(ChkIfAmex())
	{
		return TRUE;
	}
	// end add by lirz

	// more banks may need to add in later
	return FALSE;
}

uchar ChkIfAcqNeedDE56(void)
{
    if (ChkIfAcqNeedTC())
    {
        return FALSE;
    }
    return TRUE;
}

// check if allow to exit
uchar ChkIfAllowExit(void)
{
#ifdef _WIN32
	return FALSE;
#else
	APPINFO	stTempAppInf;

	if (ReadAppInfo(0, &stTempAppInf)==0)
	{
		return TRUE;
	}
	return FALSE;
#endif
}




// end of file

