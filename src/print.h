
/****************************************************************************
NAME
    print.h - 定义打印模块

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _PRINT_H
#define _PRINT_H

#define PRN_NORMAL		0
#define PRN_REPRINT		1


#define ERR_PRN_BUSY		0x01
#define ERR_PRN_PAPEROUT		0x02
#define ERR_PRN_WRONG_PACKAGE		0x03
#define PRN_ERR				0x04
#define ERR_PRN_OVERHEAT	0x08
#define PRN_LOWVOL		0x09
#define PRN_NOT_DONE	0xF0
#define PRN_NO_FONT		0xFC
#define ERR_PRN_OUTOFMEMORY		0xFE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void  PrnSetSmall(void);
void  PrnSetNormal(void);
void  PrnSetBig(void);

int  DispPrnError(int iErrCode);

extern int PrnBmp(unsigned char *filename, int mode, char alignment, unsigned char *gMallocBuffer);
#ifdef __cplusplus
}
#endif /* __cplusplus */ 

#endif	// _PRINT_H

// end of file
