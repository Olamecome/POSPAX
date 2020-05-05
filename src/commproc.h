
/****************************************************************************
NAME
    commproc.h - 实现通讯控制

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.14      - created
****************************************************************************/

#ifndef _COMMPROC_H
#define _COMMPROC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int TranProcess(void);
int PreDial(void);
int ConnectHost(void);
int ReferralDial(const uchar *pszPhoneNo);

/**
*
* @param dataIn
* @param inlen
* @param dataOut
* @param outlen
* @return
*/
int sendSocketRequest(char* dataIn, int inlen, char* dataOut, int* outlen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _COMMPROC_H

// end of file
