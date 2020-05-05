
/****************************************************************************
NAME
    checkopt.h - 

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _CHECKOPT_H
#define _CHECKOPT_H

#define HW_NONE			0
// offsets in the string returned by GetTermInfo()
#define HWCFG_MODEL		0
#define HWCFG_PRINTER	1
#define HWCFG_MODEM		2
#define HWCFG_M_SYNC	3
#define HWCFG_M_ASYNC	4
#define HWCFG_PCI		5
#define HWCFG_USBHOST	6
#define HWCFG_USBDEV	7
#define HWCFG_LAN		8
#define HWCFG_GPRS		9
#define HWCFG_CDMA		10
#define HWCFG_WIFI		11
#define HWCFG_CONTACT	12
#define HWCFG_CFONT		13
#define HWCFG_FONTVER	14
#define HWCFG_ICCREAD	15
#define HWCFG_MSR		16
#define HWCFG_BLTH		20 //offset of buffer for Buletooth
#define HWCFG_WCDMA		21
// 17--29 reserved
#define HWCFG_END		30

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uchar ChkTerm(uchar ucTermType);	// check the current terminal type
uchar ChkHardware(uchar ucChkType, uchar ucValue);
uchar ChkIfIrDAPrinter(void);
uchar ChkTermPEDMode(uchar ucMode);
uchar ChkIfEmvEnable(void);// check if supports EMV
uchar ChkEdcOption(ushort uiOption);
uchar ChkOptionExt(const uchar *psExtOpt, ushort uiOption);
void  SetOptionExt(uchar *psExtOptInOut, ushort uiOption);
uchar ChkIfNeedPIN(void);
uchar ChkIfPinReqdAllIssuer(void);
uchar ChkIfAmex(void);
uchar ChkIfAmexName(void);
uchar ChkIfBoc(void);
uchar ChkIfCiti(void);
uchar ChkIfFubon(void);
uchar ChkIfDah(void);
uchar ChkIfBea(void);
uchar ChkIfScb(void);
uchar ChkIfWordCard(void);
uchar ChkIfUob(void);
uchar ChkIfUobIpp(void);
uchar ChkIfDiners(void);
uchar ChkIfJCB(void);
uchar ChkIfICBC(void);
uchar ChkIfICBC_MACAU(void);
uchar ChkIfWingHang(void);
uchar ChkIfShanghaiCB(void);
uchar ChkIfHSBC(void);
uchar ChkIfBelowMagFloor(void);
uchar ChkInstalmentAllAcq(void);
uchar ChkIfDispMaskPan2(void);
uchar ChkIfInstalmentPara(void);
uchar ChkIfTransMaskPan(uchar ucCurPage);
uchar ChkCurAcqName(const void *pszKeyword, uchar ucPrefix);
uchar ChkIfTranAllow(uchar ucTranType);
uchar ChkIfZeroAmt(const uchar *pszIsoAmountStr);
uchar ChkIfBatchEmpty(void);
uchar ChkIfZeroTotal(const void *pstTotal);
uchar ChkSettle(void);
uchar ChkIfNeedTip(void);
uchar ChkIfDccBOC(void);
uchar ChkIfDccAcquirer(void);
uchar ChkIfDccBocOrTas(void);
uchar ChkIfIccTran(ushort uiEntryMode);
uchar ChkIfPrnReceipt(void);
uchar ChkIfNeedReversal(void);
uchar ChkIfSaveLog(void);
uchar ChkIfThermalPrinter(void);
uchar ChkIfNeedSecurityCode(void);
uchar ChkIfNeedMac(void);
uchar ChkIfAcqNeedTC(void);
uchar ChkIfAcqNeedDE56(void);
uchar ChkIfAllowExit(void);
uchar ChkIssuerOption(uchar option);
uchar ChkAcqOption(uchar option);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _CHECKOPT_H

// end of file
