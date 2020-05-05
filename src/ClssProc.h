/****************************************************************************
NAME
    TransClss.h - 定义交易处理模块

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _TRANCLSS_H
#define _TRANCLSS_H

/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//CLSS status added by kevinliu 2016/01/26
typedef enum clssLightStatus{
	CLSSLIGHTSTATUS_INIT,
	CLSSLIGHTSTATUS_NOTREADY,
	CLSSLIGHTSTATUS_IDLE,
	CLSSLIGHTSTATUS_READYFORTXN,
	CLSSLIGHTSTATUS_PROCESSING,
	CLSSLIGHTSTATUS_READCARDDONE,
	CLSSLIGHTSTATUS_REMOVECARD,
	CLSSLIGHTSTATUS_COMPLETE,
	CLSSLIGHTSTATUS_DIALING,
	CLSSLIGHTSTATUS_SENDING,
	CLSSLIGHTSTATUS_RECEIVING1,
	CLSSLIGHTSTATUS_RECEIVING2,
	CLSSLIGHTSTATUS_PRINTING,
	CLSSLIGHTSTATUS_ERROR,
} CLSSLIGHTSTATUS;

#define PICC_LED_ALL (PICC_LED_BLUE | PICC_LED_YELLOW | PICC_LED_GREEN | PICC_LED_RED)
#define PICC_LED_CLSS 0x00


int ClssTransInit();
int TransClssSale(uchar skipDetect);
//added by kevinliu 2016/01/26
void SetClssLightStatus(CLSSLIGHTSTATUS status);
int nSetDETData(uchar *pucTag, uchar ucTagLen, uchar *pucData, uchar ucDataLen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _TRANPROC_H

// end of file


