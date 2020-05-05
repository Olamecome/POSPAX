
/****************************************************************************
NAME
    manage.h - 定义管理类交易模块

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.10.25      - created
****************************************************************************/

#ifndef _MANAGE_H
#define _MANAGE_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void UnLockTerminal(void);
int LockTerm(void);
int ClearAllRecord(void);
int ClearConfig(void);
int ClearPassword(void);
int ClearReversal(void);
int DoClear(void);
int ViewTotal(void);
int ViewTranList(void);
int ViewSpecList(void);
int PrnLastTrans(void);
int RePrnSpecTrans(void);
int PrnTotal(void);
int RePrnSettle(void);
int PrintEmvErrLog(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _MANAGE_H

// end of file
