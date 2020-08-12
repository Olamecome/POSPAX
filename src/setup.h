
/****************************************************************************
NAME
    setup.h - 定义终端参数设置、查询模块

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _SETUP_H
#define _SETUP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int GetIpLocalSettings(void *pstParam);
int GetRemoteIp(const uchar *pszHalfText, uchar bAllowHostName, uchar bAllowNull, void *pstIPAddr);
int ChkIfValidIp(const uchar *pszIP);
int ChkIfValidPort(const uchar *pszPort);

int  SetTcpIpParam(void *pstParam);
void SyncTcpIpParam(void *pstDst, const void *pstSrc);

int  SetWirelessParam(WIRELESS_PARAM *pstParam);
void SyncWirelessParam(WIRELESS_PARAM *pstDst, const WIRELESS_PARAM *pstSrc);

void SetSystemParamAll(void);
void SetSysLang(uchar ucSelectMode);
void SetEdcLangExt(const char *pszDispName);

int SetRs232Param(RS232_PARA *rs232);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _SETUP_H

// end of file
