
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "emvlib.h"
#include "Logger.h"


/********************************************************************************
Update Record:
M01, 2018-03-09, litun:
1.	 Fix the issue that "PIN try limit exceeded" is processed as bypass PIN.

M02, 2018-03-27 zhengyq:
1.   Fix a compatibility issue, change the AID gotten by calling EMVGetFinalAppPara 
     from terminal AID to Card AID finally selected.
	 
M03, 2018-05-02, litun:
1.   Add a new function for EMVSetExtendFunc to change the result of the Terminal Action Analysis.

M04, 2018-05-11, litun:
1.   Add a specific function for EMVSetExtendFunc to enable calling "cEMVGetHolderPwd" for special displaying when PIN try limit is exceeded..

M05, 2018-06-25, litun :
1.   Add "cEMVIccIsoCommand" for prolin platform.

M06, 2018-06-27, zhoujie :
1.   Add a return code EMV_USER_CLEAR for EMVStartTrans and cEMVGetHolderPwd to indicate that
     the cardholders pressed clear button when amount confirmation or PIN enter is required.
2.   Add two API EMVGetCandList and EMVSetCandList.
3.   Add new function for EMVSetExtendFunc to set whether skip building candidate list in EMVAppSelect.

M07, 2018-10-17, litun :
1.   Fix the issue for contactless PBOC can't get the aid information from entry. 

M08, 2019-03-22, zhengyq :
1.	 Fix the issue that transaction log can not record correctly

M09, 2019-05-14, litun :
1.   Add a new API EMVGetExtendFunc;

M10, 2019-06-20, litun :
1.   Fix the issue that the cvm result can't be set normally.

M11, 2019-07-09, litun :
1.   Fix the issue that the cvm result can't be set normally.

M12, 2019-09-19, litun :
1.   If there is only one application remained in the candidate list, the terminal checks b8 of API to 
	 determine if the application shall be displayed for confirmation or not.
********************************************************************************/

#define APIMAP_F_VERSION (char *)"M09"

#ifndef NULL
#define NULL 0
#endif

#define EMV_API_ID_EMVCoreInit             0x01
#define EMV_API_ID_EMVGetParameter         0x02
#define EMV_API_ID_EMVSetParameter         0x03
#define EMV_API_ID_EMVGetScriptResult      0x06
#define EMV_API_ID_EMVReadVerInfo          0x07
#define EMV_API_ID_EMVSetScriptProcMethod  0x08
#define EMV_API_ID_EMVGetParamFlag         0x09
#define EMV_API_ID_EMVGetMCKParam          0x0A
#define EMV_API_ID_EMVSetMCKParam          0x0B
#define EMV_API_ID_EMVSetConfigFlag        0x0C
#define EMV_API_ID_EMVSetAmount            0x0D
#define EMV_API_ID_EMVGetFinalAppPara      0x0E
#define EMV_API_ID_EMVModFinalAppPara      0x0F
#define EMV_API_ID_EMVGetLabelList         0x10
#define EMV_API_ID_EMVAddCAPK              0x21
#define EMV_API_ID_EMVGetCAPK              0x22
#define EMV_API_ID_EMVDelAllCAPK           0x23
#define EMV_API_ID_EMVDelAllApp            0x26
#define EMV_API_ID_EMVAddRevocList         0x27
#define EMV_API_ID_EMVDelRevocList         0x28
#define EMV_API_ID_EMVDelAllRevocList      0x29
#define EMV_API_ID_EMVAppSelect            0x31
#define EMV_API_ID_EMVReadAppData          0x32
#define EMV_API_ID_EMVCardAuth             0x33
#define EMV_API_ID_EMVStartTrans           0x34
#define EMV_API_ID_EMVCompleteTrans        0x35
#define EMV_API_ID_EMVInitTLVData          0x51
#define EMV_API_ID_EMVSwitchClss           0x52

#define  MAX_API_CALLED_LOG_NUM 41

#define EMV_CVM_FAIL_CVM                0x00
#define EMV_CVM_PLAIN_PIN               0x01
#define EMV_CVM_ONLINE_PIN              0x02
#define EMV_CVM_PLAIN_PIN_SIG           0x03
#define EMV_CVM_ENCIPH_PIN              0x04
#define EMV_CVM_ENCIPH_PIN_SIG          0x05
#define EMV_CVM_SIGNATURE               0x1E
#define EMV_CVM_NO_CVM                  0x1F
#define EMV_CVM_CERTIFICATE             0x20

#ifndef EFILEMACRO
#define     MY_O_RDWR           O_RDWR
#define     MY_O_CREATE         O_CREATE
#define     MY_O_ENCRYPT        O_ENCRYPT
#ifdef _PAXME_TERM
#define EFOpen                  open
#define EFRead                  read
#define EFWrite                 write
#define EFClose(a)              close(a)
#define EFseek                  seek 
#define EFGetSize(a)            filesize(a)
#define EFRemove(a)             remove(a)
#define EFTruncate              truncate
#endif
#define EFILEMACRO
#endif//#ifndef EFILEMACRO

// typedef struct{
//     unsigned char aucAppPreName[17];      //Application Prefer Name
//     unsigned char aucAppLabel[17];        //Application Label
//     unsigned char aucIssDiscrData[244];   //tag 'BF0C', the first byte is the length.
//     unsigned char aucAID[17];             //Card AID
//     unsigned char ucAidLen;               //Card AID Length
//     unsigned char ucPriority;             //Priority
//     unsigned char aucAppName[33];         //The Local Application name
// }EMV_CANDLIST;

static unsigned char gl_aucFinalAID[17];
static unsigned char gl_ucFinalAIDLen = 0;
static unsigned char gl_ucPinInput = 0; //cEMVGetHolderPwd has been called or not? 0-no; 1-yes
static unsigned char gl_nEnableConfirmAmt  = 0;
static unsigned char gl_ucAppConfimMethod = 0; 
static int gl_nInputAmtFlag = 0;//transaction amount has been input or not? 0-no; 1-yes
static unsigned char gl_aucAPICalledLog[MAX_API_CALLED_LOG_NUM];// record the order of each API called.
static unsigned char gl_ucAPICalledLogLen = 0;
static unsigned char gl_ucEmvPciUsedFlg = 1;
static unsigned long gl_ulAuthAmt = 0, gl_ulCashBackAmt = 0;
#ifdef _PAXME_TERM

static int gl_nTransLog = 0;//support transaction log or not? 0-no; 1-yes
#endif

static EMV_CAPK gl_tCAPKList[MAX_KEY_NUM];
static int gl_nRevocListNum = 0;
static EMV_REVOCLIST gl_tRevocList[MAX_REVOCLIST_NUM];
static unsigned char gl_ucCancelPinFlag = 0;
static unsigned char gl_ucShowPinExceedFlag = 0;
static unsigned char gl_ucSkipCreCandiListFlg = 0;//Skip creating candidate list for EMVAppSelect

int EMV_II_GetTLVData(unsigned int unTag, unsigned int unExpectOutlen, unsigned char *pucDataOut, unsigned int *punOutLen );
int EMV_II_SetTLVData(unsigned int unTag, unsigned char *pucData, unsigned int unLen);

//int EMV_II_GetCandList(EMV_II_CANDLIST *ptCandList, int *pnAppNum);
int EMV_II_GetLogItemChild(unsigned char ucLogType, unsigned short Tag, unsigned char *TagData, int *TagLen);
int EMV_II_GetLogData(unsigned char *paucLogDataOut, int *pnLenOut);
int EMV_II_InitTransLog(void);
int EMV_II_GetVerifyICCStatus(unsigned char *pucSWA, unsigned char *pucSWB);
int EMV_II_GetScriptResult(unsigned char *Result, int *RetLen);
void EMV_II_SetAutoRunFlag(unsigned char ucAutoRunFlag);
int EMV_II_IsKnowTag(unsigned char aucTag[]);

EMV_CALLBACK k_gl_stCallBack;

int EMVSetCallback(EMV_CALLBACK *pstCallback)
{
    if(pstCallback == NULL)
        return EMV_PARAM_ERR;

    k_gl_stCallBack.cEMVInputAmount = pstCallback->cEMVInputAmount;
    k_gl_stCallBack.cEMVWaitAppSel = pstCallback->cEMVWaitAppSel;
    k_gl_stCallBack.cEMVUnknowTLVData = pstCallback->cEMVUnknowTLVData;
    k_gl_stCallBack.cEMVSetParam = pstCallback->cEMVSetParam;
    k_gl_stCallBack.cEMVGetHolderPwd = pstCallback->cEMVGetHolderPwd;
    k_gl_stCallBack.cEMVVerifyPINOK = pstCallback->cEMVVerifyPINOK;
    k_gl_stCallBack.cEMVAdviceProc = pstCallback->cEMVAdviceProc;
    k_gl_stCallBack.cEMVOnlineProc = pstCallback->cEMVOnlineProc;
    k_gl_stCallBack.cEMVReferProc = pstCallback->cEMVReferProc;
    k_gl_stCallBack.cCertVerify = pstCallback->cCertVerify;
    k_gl_stCallBack.cEMVVerifyPINfailed = pstCallback->cEMVVerifyPINfailed;
    return 0;
}

#define EMV_APP_READ_NUM 50

#ifndef _PAXME_TERM
EMV_APPLIST gl_tTermAppList[MAX_APP_NUM]; // The AID list saved by TM
#else
unsigned char gl_ucAppListNullLoc = 0;
#endif

#ifdef _PAXME_TERM
#define MAX_TRANS_NUM  8

typedef struct
{
    unsigned char ucPAN_SEQ;
    unsigned char aucPAN[10];
    unsigned char ucPANLen;
    unsigned long ulAmt;
}TRANS_LOG;


static unsigned char gl_ucTransNum = 0;// Max value is MAX_TRANS_NUM
static TRANS_LOG gl_tTransLog[MAX_TRANS_NUM];

void InitTransLog_emv(int ClearFlag)
{
    int i;
    int LogFd = 0;
	// The first Byte in EMVTransLog.dat.
	unsigned char ucOffSet = 0;
    if (ClearFlag)
    {
        EFClose(LogFd);
        EFRemove("EMVTransLog.dat");
    }

    gl_ucTransNum = 0;
    memset(gl_tTransLog, 0, MAX_TRANS_NUM*sizeof(TRANS_LOG));
	ucOffSet = sizeof(gl_ucTransNum);  

    LogFd = EFOpen("EMVTransLog.dat", MY_O_RDWR);
    if (LogFd < 0) 
    {
        LogFd = EFOpen("EMVTransLog.dat", MY_O_CREATE);

        if (LogFd < 0) 
            return;
    }
    else 
    {
    	// The first Byte is used to record the transaction number of the next transaction,fetched as transaction number of this transaction.
		EFRead(LogFd, (unsigned char *)(&gl_ucTransNum), ucOffSet); 
		EFseek(LogFd, (long)ucOffSet, SEEK_SET);
		// Read the transaction logs located after the first byte in EMVTransLog.dat one by one.
        for (i = 0; i < MAX_TRANS_NUM; i++) 
        {
            if (EFRead(LogFd, (unsigned char *)(&gl_tTransLog[i]), sizeof(TRANS_LOG)) == sizeof(TRANS_LOG)) 
                continue;
            break;
        }
    }
    EFClose(LogFd);
}

unsigned long GetTransLogAmt_emv(unsigned char ucPAN_SEQ, unsigned char *paucPAN, unsigned char ucPANLen)
{
    int i;
    unsigned long ulLogAmt;

    ulLogAmt = 0;
    if (paucPAN == NULL)
    {
        return EMV_PARAM_ERR;
    }

    for (i = gl_ucTransNum-1; i>=0; i--) 
    {
        if (ucPAN_SEQ != gl_tTransLog[i].ucPAN_SEQ) 
            continue;
        if (gl_tTransLog[i].ucPANLen != ucPANLen) 
            continue;
        if (memcmp(paucPAN, gl_tTransLog[i].aucPAN, ucPANLen)) 
            continue;
        if((0xffffffff-gl_tTransLog[i].ulAmt) < ulLogAmt)
        {
            ulLogAmt = 0xffffffff;
            return ulLogAmt;
        }
        ulLogAmt += gl_tTransLog[i].ulAmt;
        break;
    }
    if(i<0)
    {
        for (i = MAX_TRANS_NUM-1; i>= gl_ucTransNum; i--)
        {
            if (ucPAN_SEQ != gl_tTransLog[i].ucPAN_SEQ) 
                continue;
            if (gl_tTransLog[i].ucPANLen != ucPANLen) 
                continue;
            if (memcmp(paucPAN, gl_tTransLog[i].aucPAN, ucPANLen)) 
                continue;
            if((0xffffffff-gl_tTransLog[i].ulAmt) < ulLogAmt)
            {
                ulLogAmt = 0xffffffff;
                return ulLogAmt;
            }
            ulLogAmt += gl_tTransLog[i].ulAmt;
            break;
         }
    }

    return ulLogAmt;
}

void SaveAmtToTransLog_emv(unsigned char ucPAN_SEQ, unsigned char *paucPAN, unsigned char ucPANLen, unsigned long ulAmt)
{
    TRANS_LOG Log;
    int LogFd = 0;

	unsigned char ucOffSet = 0;    // The first byte in EMVTransLog.dat
	unsigned char ucNextTransNum=0;// The next transaction number.
    if (paucPAN == NULL)
    {
        return;
    }

    LogFd = EFOpen("EMVTransLog.dat", MY_O_RDWR);
    memset(&Log, 0, sizeof(Log));
    Log.ucPAN_SEQ = ucPAN_SEQ;
    memcpy(Log.aucPAN, paucPAN, ucPANLen);
    Log.ucPANLen = ucPANLen;
    Log.ulAmt = ulAmt;
	memcpy(&gl_tTransLog[gl_ucTransNum], &Log, sizeof(TRANS_LOG));
	ucOffSet = sizeof(gl_ucTransNum);    
	ucNextTransNum = gl_ucTransNum + 1;  // Calculate the next transaction number.
	if (ucNextTransNum == MAX_TRANS_NUM) // If the next transaction number is larger than MAX_TRANS_NUM, assign 0 to it.
		ucNextTransNum = 0;
	EFWrite(LogFd, (unsigned char *)&ucNextTransNum, ucOffSet); // Record the next transaction number into EMVTransLog.dat, at the place of the first byte.
	EFseek(LogFd, (long)gl_ucTransNum * sizeof(TRANS_LOG) + ucOffSet, SEEK_SET); // Get the size of the file EMVTransLog.dat,place the cursor at the end of it.
	EFWrite(LogFd, (unsigned char *)&Log, sizeof(TRANS_LOG)); // Add a transaction log at where the cursor located.

    EFClose(LogFd);
}

int EMVProcTrans(void)
{
    int ret = 0, ret1 = 0, nAdviceFlg = 0;
    unsigned char ucACType;
    int IAuthDataLen, ScriptLen, AuthCodeLen;
    unsigned char RspCode[2], AuthCode[6];
     unsigned char IAuthData[16], Script[300];
    unsigned char aucPAN[10], ucPANSeq;
    int nPANLen = 0;

    ret = EMVStartTrans(gl_ulAuthAmt, gl_ulCashBackAmt, &ucACType);
    
    EMVGetParamFlag(0x02, &nAdviceFlg);
    if(nAdviceFlg)
    {
#ifdef _ANDROID_TERM
         if(k_gl_stCallBack.cEMVAdviceProc)
        {
            k_gl_stCallBack.cEMVAdviceProc();
        }
        else
        {
            return EMV_NO_CBFUN;
        }
#else
        cEMVAdviceProc();
#endif
    }
    if (ret)
    {
        return ret;
    }
    
    if (ucACType == AC_ARQC)
    {
        memset(RspCode,0,sizeof(RspCode));
        IAuthDataLen = 0;
        memset(IAuthData,0,sizeof(IAuthData));
        ScriptLen = 0;
        memset(Script,0,sizeof(Script));
        AuthCodeLen = 0;
        memset(AuthCode,0,sizeof(AuthCode));
#ifdef _ANDROID_TERM
        if(k_gl_stCallBack.cEMVOnlineProc)
        {
            ret = k_gl_stCallBack.cEMVOnlineProc(RspCode, AuthCode, &AuthCodeLen, IAuthData, &IAuthDataLen, Script, &ScriptLen);
        }           
        else
        {
            return EMV_NO_CBFUN;
        }
#else
        ret = cEMVOnlineProc(RspCode, AuthCode, &AuthCodeLen, IAuthData, &IAuthDataLen, Script, &ScriptLen);        
#endif        
        if (RspCode[0])
        {
            EMVSetTLVData(0x8A, RspCode, 2);
        }
        if (AuthCodeLen)
        {
            EMVSetTLVData(0x89,AuthCode, AuthCodeLen);
        }
        if (IAuthDataLen)
        {
            if(IAuthDataLen>16)
                IAuthDataLen=16;
            
            EMVSetTLVData(0X91, IAuthData, IAuthDataLen);
        }
        
        if (ret == ONLINE_REFER)
        {
#ifdef _ANDROID_TERM
            if(k_gl_stCallBack.cEMVReferProc)
            {
                if (k_gl_stCallBack.cEMVReferProc() == REFER_APPROVE)
                {
                    ret = ONLINE_APPROVE;
                }
                else
                {
                    ret = ONLINE_DENIAL;
                }
            }
            else
            {
                return EMV_NO_CBFUN;
            }
#else
            if (cEMVReferProc() == REFER_APPROVE)
            {
                ret = ONLINE_APPROVE;
            }
            else
            {
                ret = ONLINE_DENIAL;
            }
#endif
        }
        else if (ret == ONLINE_FAILED)
        {
            ret1 = EMVCompleteTrans(ONLINE_FAILED, Script, &ScriptLen, &ucACType);
        }
        else if (ret == ONLINE_APPROVE)
        {
            ret1 = EMVCompleteTrans(ONLINE_APPROVE, Script, &ScriptLen, &ucACType);
        }
        else if (ret == ONLINE_DENIAL)
        {
            ret1 = EMVCompleteTrans(ONLINE_DENIAL, Script, &ScriptLen, &ucACType);
        }
        else
        {
            ret1 = EMVCompleteTrans(ONLINE_DENIAL, Script, &ScriptLen, &ucACType);
        }

        EMVGetParamFlag(0x02, &nAdviceFlg);
        if(nAdviceFlg)
        {
#ifdef _ANDROID_TERM
            if(k_gl_stCallBack.cEMVAdviceProc)
            {
                k_gl_stCallBack.cEMVAdviceProc();
            }
            else
            {
                return EMV_NO_CBFUN;
            }
#else
            cEMVAdviceProc();
#endif
        }
        if (ret1)
        {
            return ret1;
        }
    }

    if (gl_nTransLog == 1
        && ((ucACType == AC_TC)||(ucACType == AC_ARQC)))
    {
        EMVGetTLVData(0x5F34, &ucPANSeq, &nPANLen);
        EMVGetTLVData(0x5A, aucPAN, &nPANLen);    
        SaveAmtToTransLog_emv(ucPANSeq, aucPAN, (unsigned char)nPANLen, gl_ulAuthAmt);
    }
    return EMV_OK;
}
#endif //#ifdef _PAXME_TERM

void LongToStr_emv(unsigned long ldat, unsigned char *str)
{
    if (str == NULL)
    {
        return;
    }
    
    str[0] = (unsigned char)(ldat >> 24);
    str[1] = (unsigned char)(ldat >> 16);
    str[2] = (unsigned char)(ldat >> 8);
    str[3] = (unsigned char)(ldat);
}

void OneTwo_emv(unsigned char *One,unsigned short len,unsigned char *Two)
{
    unsigned char  i;
    static unsigned char TAB[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    
    if (One == NULL || Two == NULL)
    {
        return;
    }
    
    for (i = 0; i < len; i++) 
    {
        Two[i * 2] = TAB[One[i] >> 4];
        Two[i * 2 + 1] = TAB[One[i] & 0x0f];
    }
}

static int Toupper_emv(int nIn)
{
    int nOut=0; 
    
    if (nIn >= 'a' && nIn <= 'z')
    { 
        nOut = nIn - ('a' - 'A');
    }
    else
    {
        nOut = nIn;
    }
    
    return nOut;    
}

void TwoOne_emv(unsigned char *in, unsigned short in_len, unsigned char *out)
{
    unsigned char tmp;
    unsigned short i;
    
    if (in == NULL || out == NULL)
    {
        return;
    }
    
    for (i = 0; i < in_len; i += 2) 
    {
        tmp = in[i];
        if (tmp > '9')
            tmp = Toupper_emv(tmp) - ('A' - 0x0A);
        else
            tmp &= 0x0f;
        tmp <<= 4;
        out[i / 2] = tmp;
        
        tmp = in[i + 1];
        if (tmp > '9')
            tmp = Toupper_emv(tmp) - ('A' - 0x0A);
        else
            tmp &= 0x0f;
        out[i / 2] += tmp;
    }
}

unsigned char * StringSum_emv(unsigned char * strIn1, unsigned char * strIn2, unsigned char * strOut, int len, int hexFlag)
{
    unsigned char szTmp[20] = {0};
    unsigned char szAmt1[20] = {0};
    unsigned char szAmt2[20] = {0};

    int i, actLen;
    unsigned char a, b;
    int flag = 0;

    if (strIn1 == NULL || strIn2 == NULL || strOut == NULL || len <= 0)
        return NULL;

    actLen = len;

    if (hexFlag)
    {
        OneTwo_emv(strIn1, (unsigned short)len, szAmt1);
        OneTwo_emv(strIn2, (unsigned short)len, szAmt2);
        actLen = len * 2;
    }
    else
    {
        memcpy(szAmt1, strIn1, len);
        memcpy(szAmt2, strIn2, len);
    }
    
    memset(szTmp, 0, sizeof(szTmp));
    for (i = actLen - 1; i >= 0; i--)
    {
        a = szAmt1[i] - 0x30;
        b = szAmt2[i] - 0x30;

        if (a + b + flag>= 10)
        {
            szTmp[i] = a + b + flag - 10 + 0x30;
            flag = 1;
        }
        else
        {
            szTmp[i] = a + b + flag + 0x30;
            flag = 0;
        }
    }

    if (hexFlag)
    {
        TwoOne_emv(szTmp, (unsigned short)actLen, strOut);
    }
    else
    {
        memcpy(strOut, szTmp, actLen);
        strOut[actLen] = '\0';
    }
    
    return strOut;
}

unsigned char *SearchTLV_emv(int DolList, unsigned short Tag, unsigned char *dat, unsigned char *datend, int *TagLen)
{
    int i, j, len, tTag;

    if (dat == NULL || datend == NULL)
    {
        return NULL;
    }

    while (dat < datend) 
    {
        tTag = *dat++;
        if (tTag == 0xFF || tTag == 0x00)
            continue;
        if ((tTag & 0x1F) == 0x1F)
        {
            tTag <<= 8;
            tTag += *dat++;
            if (tTag & 0x80) 
            {
                while (dat < datend && (*dat & 0x80)) 
                    dat++;
                if (dat >= datend) 
                    return NULL;
                tTag = 0;
            }
        }
        if (*dat & 0x80)
        {
            i = (*dat & 0x7F);
            if (dat + i > datend) 
                return NULL;
            dat++;
            for (j = 0, len = 0; j < i; j++) 
            {
                len <<= 8;
                len += *dat++;
            }
        }
        else len = *dat++;

        if (tTag == Tag) 
        {
            if (TagLen != NULL) 
                *TagLen = len;
            return dat;
        }
        if (tTag & 0xFF00)
        {
            if (tTag & 0x2000)
                continue;
        }
        else if (tTag & 0x20)
            continue;
        if (!DolList)
            dat += len;
    }
    return NULL;
}

int EMVGetDebugInfo(int nExpAssistInfoLen, unsigned char *paucAssistInfo, int *pnErrorCode)
{
    if(pnErrorCode == NULL)
    {
        return EMV_PARAM_ERR;
    }
    EMV_II_GetDebugInfo(nExpAssistInfoLen, paucAssistInfo, pnErrorCode);
    return EMV_OK;
}

int EMVGetAPICalledLog(unsigned char *paucAPICalledLog, unsigned char *pucLogLen)
{
    if (paucAPICalledLog == NULL || pucLogLen == NULL)
    {
        return EMV_PARAM_ERR;
    }
    *pucLogLen = gl_ucAPICalledLogLen;
    if (*pucLogLen)
    {
        memcpy(paucAPICalledLog, gl_aucAPICalledLog, gl_ucAPICalledLogLen);
    }
    return EMV_OK;    
}

int SetAPICalledLog_emv(unsigned char ucAPI_ID)
{
    if (gl_ucAPICalledLogLen >= MAX_API_CALLED_LOG_NUM )
    {
        return EMV_OVERFLOW;
    }
    gl_aucAPICalledLog[gl_ucAPICalledLogLen] = ucAPI_ID;
    gl_ucAPICalledLogLen ++;

    return EMV_OK;
}

int CheckAPICalled_emv(unsigned char ucAPI_ID)
{
    int i=0;

    for (i=0;i<gl_ucAPICalledLogLen;i++)
    {
        if (gl_aucAPICalledLog[i] == ucAPI_ID)
        {
             return 1;
        }
    }
    return 0;
}

int InitAPICalledLog_emv(void)
{
    memset(gl_aucAPICalledLog, 0, sizeof(gl_aucAPICalledLog));
    gl_ucAPICalledLogLen=0;
    return EMV_OK;
}
#ifdef _PAXME_TERM
int InitCAPKList(void)
{
    int fd = -1;
    int nRet = 0;

    fd = EFOpen("emvCAPK.dat", MY_O_RDWR);
    if (fd >=0)
    {
        nRet = EFRead(fd, (unsigned char *)&gl_tCAPKList[0], (MAX_KEY_NUM*sizeof(gl_tCAPKList[0])));
        if (nRet < 0)
        {
            return EMV_FILE_ERR;
        }
        EFClose(fd);
        return 0;
    }
    fd = EFOpen("emvCAPK.dat", MY_O_CREATE);
    if (fd < 0)
        return EMV_FILE_ERR;
    EFClose(fd);
    return 0;
}
#endif
int EMVCoreInit(void)
{
#ifndef _PAXME_TERM
    memset(gl_tTermAppList, 0, MAX_APP_NUM*sizeof(EMV_APPLIST));
#endif
#ifdef _PAXME_TERM 
    InitTransLog_emv(0);
#endif
    InitAPICalledLog_emv();
    EMV_II_CoreInit();
#ifdef _ANDROID_TERM
     memset((unsigned char *)&k_gl_stCallBack, 0, sizeof(k_gl_stCallBack));
#endif
    SetAPICalledLog_emv(EMV_API_ID_EMVCoreInit);
    memset(gl_tCAPKList,0,sizeof(gl_tCAPKList));
#ifdef _PAXME_TERM
    InitCAPKList();// Initialize the CAPK list from file in EMVCoreInit[lijian 2018-02-06]
#endif    
	gl_ucAppConfimMethod = 0;
	gl_ucSkipCreCandiListFlg = 0;
    return 0;
}

#ifdef _PAXME_TERM
int  EMVCoreVersion(void)
{
    return 0;    
}
#endif

void EMVGetParameter(EMV_PARAM *Param)
{
    EMV_II_TERMPARAM tTermParam;

    SetAPICalledLog_emv(EMV_API_ID_EMVGetParameter);
    if (Param == NULL)
    {
        return;
    }
    memset(&tTermParam, 0, sizeof(EMV_II_TERMPARAM));
    EMV_II_GetTermParam(&tTermParam);
    memcpy(Param->MerchName, tTermParam.aucMerchName, sizeof(tTermParam.aucMerchName));
    memcpy(Param->MerchCateCode, tTermParam.aucMerchCateCode, sizeof(tTermParam.aucMerchCateCode));
    memcpy(Param->MerchId, tTermParam.aucMerchId, sizeof(tTermParam.aucMerchId));
    memcpy(Param->TermId, tTermParam.aucTermId, sizeof(tTermParam.aucTermId));
    Param->TerminalType = tTermParam.ucTerminalType;
    memcpy(Param->Capability, tTermParam.aucCapability, sizeof(tTermParam.aucCapability));
    memcpy(Param->ExCapability, tTermParam.aucExCapability, sizeof(tTermParam.aucExCapability));
    Param->TransCurrExp = tTermParam.ucTransCurrExp;
    Param->ReferCurrExp = tTermParam.ucReferCurrExp;
    memcpy(Param->ReferCurrCode, tTermParam.aucReferCurrCode, sizeof(tTermParam.aucReferCurrCode));
    memcpy(Param->CountryCode, tTermParam.aucCountryCode, sizeof(tTermParam.aucCountryCode));
    memcpy(Param->TransCurrCode, tTermParam.aucTransCurrCode, sizeof(tTermParam.aucTransCurrCode));
    Param->ReferCurrCon = tTermParam.ulReferCurrCon;
    Param->TransType = tTermParam.ucTransType;
    Param->ForceOnline = tTermParam.ucForceOnline;
    Param->GetDataPIN = tTermParam.ucGetDataPIN;
    Param->SurportPSESel = tTermParam.ucSurportPSESel;
}

void EMVSetParameter(EMV_PARAM *Param)
{
    EMV_II_TERMPARAM tTermParam;

    SetAPICalledLog_emv(EMV_API_ID_EMVSetParameter);
    if (Param == NULL)
    {
        return;
    }
    memset(&tTermParam, 0, sizeof(EMV_II_TERMPARAM));
    EMV_II_GetTermParam(&tTermParam);

    memcpy(tTermParam.aucMerchName, Param->MerchName, sizeof(tTermParam.aucMerchName));
    memcpy(tTermParam.aucMerchCateCode, Param->MerchCateCode, sizeof(tTermParam.aucMerchCateCode));
    memcpy(tTermParam.aucMerchId, Param->MerchId, sizeof(tTermParam.aucMerchId));
    memcpy(tTermParam.aucTermId, Param->TermId, sizeof(tTermParam.aucTermId));
    tTermParam.ucTerminalType = Param->TerminalType;
    memcpy(tTermParam.aucCapability, Param->Capability, sizeof(tTermParam.aucCapability));
    memcpy(tTermParam.aucExCapability, Param->ExCapability, sizeof(tTermParam.aucExCapability));
    tTermParam.ucTransCurrExp = Param->TransCurrExp;
    tTermParam.ucReferCurrExp = Param->ReferCurrExp;
    memcpy(tTermParam.aucReferCurrCode, Param->ReferCurrCode, sizeof(tTermParam.aucReferCurrCode));
    memcpy(tTermParam.aucCountryCode, Param->CountryCode, sizeof(tTermParam.aucCountryCode));
    memcpy(tTermParam.aucTransCurrCode, Param->TransCurrCode, sizeof(tTermParam.aucTransCurrCode));
    tTermParam.ulReferCurrCon = Param->ReferCurrCon;
    tTermParam.ucTransType = Param->TransType;
    tTermParam.ucForceOnline = Param->ForceOnline;
    tTermParam.ucGetDataPIN = Param->GetDataPIN;
    tTermParam.ucSurportPSESel = Param->SurportPSESel;

    EMV_II_SetTermParam(&tTermParam);
}

int EMVGetTLVData(unsigned short Tag, unsigned char *DataOut, int *OutLen)
{    
    if (DataOut == NULL || OutLen == NULL)
    {
        return EMV_PARAM_ERR;
    }

    if (!CheckAPICalled_emv(EMV_API_ID_EMVCoreInit))
    {// EMVCoreInit has not been called.
        return EMV_DATA_ERR;
    }

    return EMV_II_GetTLVData((unsigned int)Tag, 0xff, DataOut, (unsigned int *)OutLen);
}

int EMVSetTLVData(unsigned short Tag, unsigned char *Data, int len)
{
    if (Data == NULL)
    {
        return EMV_DATA_ERR;
    }

    if (!CheckAPICalled_emv(EMV_API_ID_EMVCoreInit))
    {// EMVCoreInit has not been called.
        return EMV_DATA_ERR;
    }

    return EMV_II_SetTLVData((unsigned int)Tag, Data, (unsigned int)len);
}

int EMVGetScriptResult(unsigned char *Result, int *RetLen)
{
    SetAPICalledLog_emv(EMV_API_ID_EMVGetScriptResult);
    if (Result==NULL || RetLen==NULL)
    {
        return EMV_PARAM_ERR;
    }
    return EMV_II_GetScriptResult(Result, RetLen);
}


#ifdef _PAXME_TERM
// transfer the old file format to be new format.
int TmAppFileTransfer_emv(void)
{

    int i=0;
    int fdOld, fdNew;
    int nTmAppNum=0;
    long lnAppfileSize=0;
    unsigned char aucNumTemp[2]={0};
    EMV_APPLIST astApp[32];
    int nRet=0;
    
    fdNew = EFOpen("kAppList.dat", MY_O_RDWR);
    if (fdNew < 0)
    {
        fdOld = EFOpen("emvAppList.dat", MY_O_RDWR);
        if (fdOld < 0)
        {
            return EMV_OK;
        }
        else
        {
            memset(astApp, 0, sizeof(astApp));
            EFseek(fdOld, 0, SEEK_SET);
            nRet = EFRead(fdOld, (unsigned char *)&astApp[0], (32*sizeof(EMV_APPLIST)));
            EFClose(fdOld);
            if (nRet < 0)
            {
                return EMV_FILE_ERR;
            }
            for (i=0; i<32; i++)
            {
                nRet = EMVAddApp(&astApp[i]);
                if ((nRet != EMV_PARAM_ERR) && (nRet != 0))
                {
                    EFRemove("kAppList.dat");
                    return EMV_FILE_ERR;
                }
            }
            EFRemove("emvAppList.dat");
        }
    }
    else 
    {
        EFClose(fdNew);
    }
    
    return EMV_OK;
}

// get the application number saved in TM file
int GetTmAppNum_emv(void)
{
    long lnAppfileSize=0;
    int nAppNum=0;
    unsigned char aucTemp[2]={0};

    lnAppfileSize = EFGetSize("kAppList.dat");
    if (lnAppfileSize < 0)
    {
        return EMV_FILE_ERR;
    }
    
    nAppNum = (lnAppfileSize-2)/sizeof(EMV_APPLIST);

    return nAppNum;
}

// find the application ID in the application list by AID
int FindAppInFile_emv(int nFid, int nAppNum, unsigned char *paucAid, unsigned char ucAidLen
                   , unsigned char ucFullMatchFlag, EMV_APPLIST *pstApp)
{
    EMV_APPLIST astAppList[EMV_APP_READ_NUM];
    int i=0, j=0;
    int nRet=0;
    int nReadNum=0;// the AID number read by one time
    int nReadTime=0;// the times to read all AIDs
    int nAppMatchNo=0;//the AID ID matched.
    int nReadLength=0;
    
    gl_ucAppListNullLoc = 0;
    if ((nAppNum%EMV_APP_READ_NUM) == 0)
    {
        nReadTime = (nAppNum/EMV_APP_READ_NUM);
    }
    else
    {
        nReadTime = (nAppNum/EMV_APP_READ_NUM) + 1;
    }
    
    EFseek(nFid, 2, SEEK_SET);
    for(i=0; i<nReadTime; i++)
    {
        if (i == (nReadTime-1))
        {
            nReadNum = nAppNum % EMV_APP_READ_NUM;
        }
        else
        {
            nReadNum = EMV_APP_READ_NUM;
        }

        memset(astAppList, 0, sizeof(astAppList));
        nRet = EFRead(nFid, (unsigned char*)&astAppList, nReadNum*sizeof(EMV_APPLIST));
        nReadLength = nReadNum*sizeof(EMV_APPLIST);
        if(nRet != nReadLength)
        {
            return EMV_FILE_ERR;
        }
        
        //find the AID matched and return it's ID.
        for (j=0; j<nReadNum; j++)
        {
            if (astAppList[j].AidLen != 0)
            {
                if(memcmp(paucAid,astAppList[j].AID, astAppList[j].AidLen) == 0)
                {
                    if (ucFullMatchFlag == 1 || astAppList[j].SelFlag == 1)
                    {
                        if (ucAidLen != astAppList[j].AidLen)
                        {
                            continue;
                        }
                    }

                    nAppMatchNo = (i*EMV_APP_READ_NUM)+j;
                    if (pstApp != NULL)
                    {
                        memcpy(pstApp, &astAppList[j],sizeof(EMV_APPLIST));
                    }
                    
                    return nAppMatchNo;
                }
            }
            else if (gl_ucAppListNullLoc == 0)
            {
                gl_ucAppListNullLoc = (EMV_APP_READ_NUM*i) + j + 1;
            }
        
        }
    }
    return EMV_NOT_FOUND;
}
#endif

int LoadAllApptoEMVIIKern_emv(void)
{
    int nRet =0; 
    int i = 0;

#ifndef _PAXME_TERM    
    for (i = 0; i < MAX_APP_NUM; i++)
    {
        if (gl_tTermAppList[i].AidLen)
        {
            nRet=EMV_II_AddAidList(gl_tTermAppList[i].AID,gl_tTermAppList[i].AidLen, gl_tTermAppList[i].SelFlag);
            if (nRet)
            {
                return nRet;
            }
        }
    }
    return EMV_OK;
#else
    int fd = 0, nTmAppNum = 0;
    EMV_APPLIST tTmpApp;

    memset(&tTmpApp,0,sizeof(EMV_APPLIST));
    fd = EFOpen("kAppList.dat", MY_O_RDWR);
    if (fd < 0) 
    {
        return EMV_NOT_FOUND;
    }

    nTmAppNum = GetTmAppNum_emv();
    
    for (i = 0; i< nTmAppNum; i++)
    {
        nRet = EFseek(fd, i*sizeof(EMV_APPLIST)+2, SEEK_SET);
        if(nRet)
        {
            EFClose(fd);
            return EMV_FILE_ERR;
        }
        nRet = EFRead(fd, (unsigned char*)&tTmpApp, sizeof(EMV_APPLIST));
        if(nRet != sizeof(EMV_APPLIST))    
        {
            EFClose(fd);
            return    EMV_FILE_ERR;
        }
        nRet=EMV_II_AddAidList(tTmpApp.AID, tTmpApp.AidLen, tTmpApp.SelFlag);
        if (nRet)
        {
            EFClose(fd);
            return nRet;
        }
    }
    EFClose(fd);
#endif    
    return EMV_OK;

}

int EMVAddApp(EMV_APPLIST *App)
{
#ifdef _PAXME_TERM
    int fd;
    int nRet =0; 
    int nAppMatchNo=0;
    EMV_APPLIST stAppTemp;    
    unsigned char aucNumTemp[2]={0};
    int nTmAppNum=0;
#else
    int i;
#endif

    if (App == NULL)
    {
        return EMV_PARAM_ERR;
    }

    if (App->AidLen < 5 || App->AidLen >16)
    {
        return EMV_PARAM_ERR;
    }

#ifndef    _PAXME_TERM 
    for (i = 0; i < MAX_APP_NUM; i++) 
    {
        if (App->AidLen == gl_tTermAppList[i].AidLen)
        {
            if (!memcmp(App->AID, gl_tTermAppList[i].AID, App->AidLen)) 
                break;
        }
    }
    if (i == MAX_APP_NUM)
    {
        for (i = 0; i < MAX_APP_NUM; i++)
        {
            if (!gl_tTermAppList[i].AidLen)
                break;
        }
        if (i == MAX_APP_NUM) 
            return -20;
    }
    memcpy(&gl_tTermAppList[i], App, sizeof(gl_tTermAppList[0]));
#else
    memset(&stAppTemp,0,sizeof(EMV_APPLIST));
    fd = EFOpen("kAppList.dat", MY_O_RDWR);
    if (fd < 0) 
    {
        fd = EFOpen("kAppList.dat", MY_O_CREATE|MY_O_RDWR);
        if (fd < 0)
        {
            return EMV_FILE_ERR;
        }

        memset(aucNumTemp, 0, sizeof(aucNumTemp));
        aucNumTemp[1] = 1;
        
        EFseek(fd, 0, SEEK_SET);
        EFWrite(fd, aucNumTemp, 2);
        EFWrite(fd, (unsigned char *)App, sizeof(EMV_APPLIST));

    }
    else
    {
        nTmAppNum = GetTmAppNum_emv();
        if (nTmAppNum < 0)
        {
            EFClose(fd);
            return EMV_FILE_ERR;
        }
                
        nAppMatchNo = FindAppInFile_emv(fd, nTmAppNum, App->AID, App->AidLen, 1, &stAppTemp);
        if(nAppMatchNo >= 0)
        {
            if (memcmp(&stAppTemp, &App, sizeof(EMV_APPLIST)) == 0)
            {
                EFClose(fd);
                return EMV_OK;
            }
            EFseek(fd, (nAppMatchNo*sizeof(EMV_APPLIST))+2, SEEK_SET);
        }
        else if (nAppMatchNo == EMV_NOT_FOUND)
        {
            if (gl_ucAppListNullLoc == 0)
            {
                EFseek(fd, 0, SEEK_END);
            }
            else
            {
                EFseek(fd, ((gl_ucAppListNullLoc-1)*sizeof(EMV_APPLIST)+2), SEEK_SET);    
            }
        }
        else
        {
            EFClose(fd);
            return EMV_FILE_ERR;
        }
        
        EFWrite(fd, (unsigned char *)App, sizeof(EMV_APPLIST));
        if (nAppMatchNo == EMV_NOT_FOUND)
        {
            nTmAppNum++;
            memset(aucNumTemp, 0, sizeof(aucNumTemp));
            aucNumTemp[0] = nTmAppNum >> 8;
            aucNumTemp[1] = nTmAppNum % 256;
            
            EFseek(fd, 0, SEEK_SET);
            EFWrite(fd, aucNumTemp, 2);
        }
    }

    EFClose(fd);
#endif

    return EMV_OK;
}


int EMVGetApp(int Index, EMV_APPLIST *App)
{
#ifdef _PAXME_TERM
    int fd;
    int nRet =0;
    int nTmAppNum = 0;
    EMV_APPLIST k_tmplist;
#endif
    
    if (App == NULL)
    {
        return EMV_PARAM_ERR;
    }
    
#ifndef _PAXME_TERM
    if (Index < 0 || Index >= MAX_APP_NUM) 
    {
        return EMV_NOT_FOUND;
    }
    
    if (gl_tTermAppList[Index].AidLen) 
    {
        memcpy(App, &gl_tTermAppList[Index], sizeof(gl_tTermAppList[0]));
        return 0;
    }
#else
    nTmAppNum = GetTmAppNum_emv();
    if (nTmAppNum < 0)
    {
        return EMV_NOT_FOUND;
    }
    
    if (Index >= nTmAppNum)
    {
        return EMV_NOT_FOUND;
    }
    
    memset(&k_tmplist,0,sizeof(EMV_APPLIST));
    fd = EFOpen("kAppList.dat", MY_O_RDWR);
    if (fd < 0) 
        return EMV_NOT_FOUND;
    nRet = EFseek(fd, Index*sizeof(EMV_APPLIST)+2, SEEK_SET);
    if(nRet)
    {
        EFClose(fd);
        return nRet;
    }
    
    nRet = EFRead(fd, (unsigned char*)&k_tmplist, sizeof(EMV_APPLIST));
    if(nRet != sizeof(EMV_APPLIST))    
    {
        EFClose(fd);
        return    EMV_FILE_ERR;
    }
    memcpy(App, &k_tmplist, sizeof(EMV_APPLIST));
    EFClose(fd);
    
    if((!memcmp(k_tmplist.AID, (unsigned char*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",16))||
        (!k_tmplist.AidLen))
    {
        return EMV_NOT_FOUND;
    }
    
    return 0;
#endif
    return EMV_NOT_FOUND;
}

int EMVDelApp(unsigned char *AID, int AidLen)
{
#ifdef _PAXME_TERM
    int fd, j=0, nAppLoc=0, nAppNum=0;
    int nRet =0;
    EMV_APPLIST stEmvapp;
    long lnAppfileSizeOld=0, lnAppfileSizeNew=0;
    unsigned char aucNumTemp[2]={0};
    int nTmAppNum=0;
    int nAppMatchNo=0;
    long loffset=0;
#else
    int i;
#endif
    
    if (AID == NULL) 
    {
        return EMV_PARAM_ERR;
    }
    
#ifndef _PAXME_TERM
    for (i = 0; i < MAX_APP_NUM; i++) 
    {
        if (AidLen == gl_tTermAppList[i].AidLen) 
        {
            if (!memcmp(AID, gl_tTermAppList[i].AID, AidLen)) 
                break;
        }
    }
    if (i == MAX_APP_NUM)
        return EMV_NOT_FOUND;
    
    memset(&gl_tTermAppList[i], 0, sizeof(gl_tTermAppList[0]));
#else

    fd = EFOpen("kAppList.dat", MY_O_RDWR);
    if (fd < 0) 
        return EMV_NOT_FOUND;
    
    nTmAppNum = GetTmAppNum_emv();
    if (nTmAppNum <= 0)
    {
        EFClose(fd);
        return EMV_NOT_FOUND;
    }
                
    nAppMatchNo = FindAppInFile_emv(fd, nTmAppNum, AID, (unsigned char)AidLen, 1, NULL);
    if (nAppMatchNo >= 0)
    {
        nAppLoc = 2 + nAppMatchNo*sizeof(EMV_APPLIST);
        EFseek(fd, nAppLoc, SEEK_SET);
        memset(&stEmvapp, 0, sizeof(stEmvapp));
        EFWrite(fd, (unsigned char*)&stEmvapp, sizeof(EMV_APPLIST));
        
        nTmAppNum--;
        aucNumTemp[0] = nTmAppNum >> 8;
        aucNumTemp[1] = nTmAppNum % 256;
        EFseek(fd, 0, SEEK_SET);
        EFWrite(fd, aucNumTemp, 2);
        EFClose(fd);
    }
    else
    {
        EFClose(fd);
        return nAppMatchNo;
    }
    
#endif
    return 0;
}

int EMVDelAllApp(void)
{
    SetAPICalledLog_emv(EMV_API_ID_EMVDelAllApp);
    EMV_II_DelAllAidList();

#ifdef _PAXME_TERM
    EFRemove("emvAppList.dat");
    EFRemove("kAppList.dat");
#else
    memset(gl_tTermAppList, 0, MAX_APP_NUM*sizeof(EMV_APPLIST));
#endif
    return EMV_OK;
}

int  EMVModFinalAppPara(EMV_APPLIST *ptEMV_APP)
{
    EMV_II_AIDPARAM tAidParam;
	unsigned char aucTmAID[17] = {0};
	int nTmAidLen = 0;
	int nRet = -1;
    SetAPICalledLog_emv(EMV_API_ID_EMVModFinalAppPara);
    if (ptEMV_APP == NULL)
    {
        return EMV_PARAM_ERR;
    }
	
	if(!CheckAPICalled_emv(EMV_API_ID_EMVAppSelect))//Contactless PBOC need to get the selected AID information from Entry library
	{
		memset(gl_aucFinalAID, 0x00, sizeof(gl_aucFinalAID));
		gl_ucFinalAIDLen = 0;

		nRet = EMVGetTLVData(0x9F06, aucTmAID, &nTmAidLen);
		if(nRet == EMV_OK)
		{
			memcpy(gl_aucFinalAID, aucTmAID, nTmAidLen);
			gl_ucFinalAIDLen = nTmAidLen;
		}
		else
		{
			return EMV_NOT_FOUND;
		}
	}
	
	if((ptEMV_APP->AidLen == gl_ucFinalAIDLen)&& (!memcmp(ptEMV_APP->AID,gl_aucFinalAID,gl_ucFinalAIDLen)))
	{
		memset(&tAidParam, 0, sizeof(EMV_II_AIDPARAM));
		tAidParam.ucTargetPer = ptEMV_APP->TargetPer;
		tAidParam.ucMaxTargetPer = ptEMV_APP->MaxTargetPer;
		tAidParam.ucFloorLimitCheck = ptEMV_APP->FloorLimitCheck;
		tAidParam.ucRandTransSel = ptEMV_APP->RandTransSel;
		tAidParam.ucVelocityCheck = ptEMV_APP->VelocityCheck;
		tAidParam.ulFloorLimit = ptEMV_APP->FloorLimit;
		tAidParam.ulThreshold = ptEMV_APP->Threshold;
		memcpy(tAidParam.aucTACDenial, ptEMV_APP->TACDenial, sizeof(tAidParam.aucTACDenial));
		memcpy(tAidParam.aucTACOnline, ptEMV_APP->TACOnline, sizeof(tAidParam.aucTACOnline));
		memcpy(tAidParam.aucTACDefault, ptEMV_APP->TACDefault, sizeof(tAidParam.aucTACDefault));
		memcpy(tAidParam.aucAcquierId, ptEMV_APP->AcquierId, sizeof(tAidParam.aucAcquierId));
		memcpy(tAidParam.aucdDOL, ptEMV_APP->dDOL, sizeof(tAidParam.aucdDOL));
		memcpy(tAidParam.auctDOL, ptEMV_APP->tDOL, sizeof(tAidParam.auctDOL));
		memcpy(tAidParam.aucVersion, ptEMV_APP->Version, sizeof(tAidParam.aucVersion));
		memcpy(tAidParam.aucRiskManData, ptEMV_APP->RiskManData, sizeof(tAidParam.aucRiskManData));
		return  EMV_II_SetAidParam(&tAidParam); 
		
	}
	return EMV_NOT_FOUND;
}

int  EMVGetFinalAppPara(EMV_APPLIST *ptEMV_APP)
{
#ifdef _PAXME_TERM
    int fd;
    int nRet =0;
    int nTmAppNum=0;
#else
    int i;
#endif
	
    SetAPICalledLog_emv(EMV_API_ID_EMVGetFinalAppPara);
    if (ptEMV_APP == NULL)
    {
        return EMV_PARAM_ERR;
    }
    memset(ptEMV_APP, 0, sizeof(EMV_APPLIST));
#ifndef _PAXME_TERM
    for (i = 0; i < MAX_APP_NUM; i++) 
    {
        if (gl_tTermAppList[i].SelFlag == FULL_MATCH)
        {
            if (gl_ucFinalAIDLen == gl_tTermAppList[i].AidLen
                && !memcmp(gl_aucFinalAID, gl_tTermAppList[i].AID, gl_ucFinalAIDLen)) 
            {
                break;
            }
        }
        else
        {
            if (!memcmp(gl_aucFinalAID, gl_tTermAppList[i].AID, gl_tTermAppList[i].AidLen)) 
            {
                break;
            }
        }
    }
    if(i == MAX_APP_NUM)
    {
        return EMV_NO_APP;
    }
    memcpy(ptEMV_APP, &gl_tTermAppList[i], sizeof(EMV_APPLIST));
#else
    fd = EFOpen("kAppList.dat", MY_O_RDWR);
    if (fd < 0) 
    {
        return EMV_NO_APP;
    }
    
    nTmAppNum = GetTmAppNum_emv();
	
    nRet = FindAppInFile_emv(fd, nTmAppNum, gl_aucFinalAID, gl_ucFinalAIDLen, 0, ptEMV_APP);
    EFClose(fd);
    if (nRet < 0) 
    {
        return EMV_NO_APP;
    }
#endif
    memcpy(ptEMV_APP->AID,gl_aucFinalAID,gl_ucFinalAIDLen);
	ptEMV_APP->AidLen = gl_ucFinalAIDLen;
    return EMV_OK;
}


int  EMVGetLabelList(APPLABEL_LIST  *ptAppLabel, int *pnAppNum)
{
    int nRet=0, i=0;
    EMV_II_CANDLIST tCandAppList[MAX_APP_ITEMS];

    SetAPICalledLog_emv(EMV_API_ID_EMVGetLabelList);
    if (ptAppLabel == NULL || pnAppNum == NULL )
    {
        return EMV_PARAM_ERR;
    }

    memset(tCandAppList, 0, MAX_APP_ITEMS*sizeof(EMV_II_CANDLIST));
    nRet = EMV_II_GetCandList(tCandAppList, pnAppNum);
    if (nRet)
    {
        return nRet;
    }

    if (*pnAppNum)
    {
        for (i=0; i < (*pnAppNum); i++)
        {
            memcpy(ptAppLabel[i].aucAppPreName, tCandAppList[i].aucAppPreName, sizeof(ptAppLabel[i].aucAppPreName));
            memcpy(ptAppLabel[i].aucAppLabel, tCandAppList[i].aucAppLabel, sizeof(ptAppLabel[i].aucAppLabel));
            memcpy(ptAppLabel[i].aucIssDiscrData, tCandAppList[i].aucIssDiscrData, sizeof(ptAppLabel[i].aucIssDiscrData));
            memcpy(ptAppLabel[i].aucAID, tCandAppList[i].aucAID, sizeof(ptAppLabel[i].aucAID));
            ptAppLabel[i].ucAidLen = tCandAppList[i].ucAidLen;
        }
        return EMV_OK;
    }
    return EMV_NO_DATA;
}

int EMVAddCAPK(EMV_CAPK  *capk)
{    
#ifdef _PAXME_TERM
    int fd;
#endif
    int i, len;
    unsigned char checkSum[20], buff[300];

    if (capk == NULL)
    {
        return EMV_PARAM_ERR;
    }
    
    memcpy(buff, capk->RID, 5);
    buff[5] = capk->KeyID;
    len = 6;
    memcpy(buff + len, capk->Modul, capk->ModulLen);
    len += capk->ModulLen;
    memcpy(buff + len, capk->Exponent, capk->ExponentLen);
    len += capk->ExponentLen;
    DEVICE_Hash(buff, len, checkSum);
    
    if (memcmp(capk->CheckSum, checkSum, 20) && ((capk->ArithInd != 0x04) || (capk->HashInd != 0x07)))
    {
        return EMV_SUM_ERR;
    }
    
    for (i = 0; i < MAX_KEY_NUM; i++) 
    {
        if (gl_tCAPKList[i].KeyID == capk->KeyID) 
        {
            if (!memcmp(gl_tCAPKList[i].RID, capk->RID, 5)) 
                break;
        }
    }
    if (i == MAX_KEY_NUM) 
    {
        for (i = 0; i < MAX_KEY_NUM; i++)
        {
            if (gl_tCAPKList[i].ModulLen == 0) 
                break;
        }
    }
    if (i == MAX_KEY_NUM) 
    {
        return EMV_OVERFLOW;
    }
    memcpy(&gl_tCAPKList[i], capk, sizeof(gl_tCAPKList[0]));

#ifdef _PAXME_TERM
    fd = EFOpen("emvCAPK.dat", MY_O_RDWR);
    if (fd < 0) 
    {
        fd = EFOpen("emvCAPK.dat", MY_O_CREATE|MY_O_RDWR);
        if (fd < 0)
        {
            return EMV_FILE_ERR;
        }
    }

    EFseek(fd, (long)i * sizeof(gl_tCAPKList[0]), SEEK_SET);
    EFWrite(fd, (unsigned char *)&gl_tCAPKList[i], sizeof(gl_tCAPKList[0]));
    EFClose(fd);
#endif
    return 0;       
}

int  EMVGetCAPK(int Index, EMV_CAPK  *capk)
{
    if (capk == NULL)
    {
        return EMV_PARAM_ERR;
    }
    
    if (Index < 0 || Index >= MAX_KEY_NUM) 
        return EMV_NOT_FOUND;
    if (gl_tCAPKList[Index].ModulLen) 
    {
        memcpy(capk, &gl_tCAPKList[Index], sizeof(gl_tCAPKList[0]));
        return 0;
    }
    else 
    {
        return EMV_NOT_FOUND;
    }
}

int  EMVDelAllCAPK(void)
{
    SetAPICalledLog_emv(EMV_API_ID_EMVDelAllCAPK);
    EMV_II_DelAllCAPK();
#ifdef _PAXME_TERM
    memset(gl_tCAPKList,0,sizeof(gl_tCAPKList));
    EFRemove("emvCAPK.dat");
#endif
    return EMV_OK;
}

int EMVDelCAPK(unsigned char KeyID, unsigned char *RID)
{
    int i;
#ifdef _PAXME_TERM
    int fd;
#endif
    
    if (RID == NULL)
    {
        return EMV_PARAM_ERR;
    }
    
    for (i = 0; i < MAX_KEY_NUM; i++) 
    {
        if (gl_tCAPKList[i].KeyID == KeyID) 
        {
            if (!memcmp(gl_tCAPKList[i].RID, RID, 5)) 
                break;
        }
    }
    if (i == MAX_KEY_NUM)
        return EMV_NOT_FOUND;
    memset(&gl_tCAPKList[i], 0, sizeof(gl_tCAPKList[0]));
    
#ifdef _PAXME_TERM
    fd = EFOpen("emvCAPK.dat", MY_O_RDWR);
    if (fd < 0) 
        return EMV_FILE_ERR;
    EFseek(fd, (long)i * sizeof(gl_tCAPKList[0]), SEEK_SET);
    EFWrite(fd, (unsigned char *)&gl_tCAPKList[i], sizeof(gl_tCAPKList[0]));
    EFClose(fd);
#endif
    return 0;
}

int EMVCheckCAPK(unsigned char *KeyID, unsigned char *RID)
{
    int i;
    unsigned char aucTransDate[3];
    int nLen = 0;
        
    if (KeyID == NULL || RID == NULL)
    {
        return EMV_PARAM_ERR;
    }
    
    EMVGetTLVData(0x9A, aucTransDate, &nLen);

    for (i = 0; i < MAX_KEY_NUM; i++)
    {
        if (!gl_tCAPKList[i].ModulLen)
            continue;
        if (aucTransDate[0] >= 0x50)
            continue;
        if (gl_tCAPKList[i].ExpDate[0] >= 0x50 || memcmp(aucTransDate,gl_tCAPKList[i].ExpDate,3) > 0) 
        {
            *KeyID = gl_tCAPKList[i].KeyID;
            memcpy(RID, gl_tCAPKList[i].RID, 5);
            return EMV_KEY_EXP;
        }
    }
    return 0;
}

int  EMVAddRevocList(EMV_REVOCLIST *pRevocList)
{
    int i = 0;

    if (pRevocList == NULL) 
    {
        return (EMV_DATA_ERR);
    }

    i = 0;
    if (gl_nRevocListNum > 0) 
    {
        for (i = 0; i < gl_nRevocListNum; i++) 
        {
            if ((pRevocList->ucIndex == gl_tRevocList[i].ucIndex) 
                && (!memcmp(pRevocList->ucRid, gl_tRevocList[i].ucRid, 5)))
            {
                memcpy(gl_tRevocList[i].ucCertSn, pRevocList->ucCertSn, 3);
                
                return 0;
            }
        }
        if (gl_nRevocListNum == MAX_REVOCLIST_NUM) 
        {
            return (EMV_OVERFLOW);
        }
    }
    memcpy(gl_tRevocList+i, pRevocList, sizeof(EMV_REVOCLIST));
    gl_nRevocListNum++;
    return EMV_OK;
}

int  EMVDelRevocList(unsigned char ucIndex, unsigned char *pucRID)
{
    int i = 0;
    int j = 0;
    
    if (pucRID == NULL)
    {
        return (EMV_DATA_ERR);
    }
    
    for (i = 0; i < gl_nRevocListNum; i++)
    {
        if ((ucIndex == gl_tRevocList[i].ucIndex)
            && (!(memcmp(pucRID, gl_tRevocList[i].ucRid, 5))))
        {
            memset(gl_tRevocList+i, 0, sizeof(EMV_REVOCLIST));
            
            for (j = i; j < gl_nRevocListNum-1; j++) 
            {
                memmove(gl_tRevocList+j, gl_tRevocList+j+1, sizeof(EMV_REVOCLIST));                
            }
            gl_nRevocListNum--;
            return 0;
        }
    }
    return (EMV_DATA_ERR);
}

void EMVDelAllRevocList(void)
{
    memset(gl_tRevocList, 0, MAX_REVOCLIST_NUM*sizeof(EMV_REVOCLIST));
    return;
}

int SaveAmtToEMVII_emv(unsigned char * paucAuthAmount, unsigned char * paucCashBackAmount)
{
    unsigned char aucStrAmtSum[20] = {0};
    unsigned char aucNumAmtSum[20] = {0};
    unsigned char aucStrAmtOther[20] = {0};
    unsigned char aucNumAmtOther[20] = {0};
    unsigned char i=0;
    unsigned char ucTransType=0;
    unsigned int unLen = 0; 
    int nRet = 0;
    unsigned long ulAuthAmt=0,ulCashBackAmt=0; 

    if (paucAuthAmount == NULL)
    {
        return EMV_DATA_ERR;
    }

    EMV_II_GetTLVData(0x9C, 1, &ucTransType, &unLen);
    if(ucTransType == 0x09)//cash back
    {
        if(paucCashBackAmount == NULL)
        {
            return EMV_DATA_ERR;
        }
        memcpy(aucNumAmtOther, paucCashBackAmount, 6);
        OneTwo_emv(aucNumAmtOther, 6, aucStrAmtOther);

        StringSum_emv(paucAuthAmount, paucCashBackAmount, aucNumAmtSum, 6, 1);

        if ((memcmp(aucNumAmtSum, paucAuthAmount, 6) < 0) || (memcmp(aucNumAmtSum, paucCashBackAmount, 6) < 0))
        {
            return EMV_DATA_ERR;
        }
        OneTwo_emv(aucNumAmtSum, 6, aucStrAmtSum);
    }
    else
    {
        memcpy(aucNumAmtSum, paucAuthAmount, 6);
        OneTwo_emv(aucNumAmtSum, 6, aucStrAmtSum);
    }

    //check if the Amount is right
    for(i=0; i<12; i++)
    {
        if(aucStrAmtSum[i] > '9')
        {
            return EMV_DATA_ERR;
        }
        if(ucTransType == 0x09)//cashback
        {
            if(aucStrAmtOther[i] > '9')
            {
                return EMV_DATA_ERR;
            }
        }
    }

    if (memcmp(aucNumAmtSum, "\x00\x42\x94\x96\x72\x95", 6) >= 0)
    {
        ulAuthAmt = 0xffffffff;
    }
    else
    {
        ulAuthAmt = atol((char*)aucStrAmtSum);
    }
    
    if(ucTransType == 0x09)//cashback
    {
        if (memcmp(aucNumAmtOther, "\x00\x42\x94\x96\x72\x95", 6) >= 0)
        {
            ulCashBackAmt = 0xffffffff;
        }
        else
        {
            ulCashBackAmt = atol((char*)aucStrAmtOther);
        }
    }

    memset(aucStrAmtSum, 0, sizeof(aucStrAmtSum));
    LongToStr_emv(ulAuthAmt, aucStrAmtSum);
    nRet=EMV_II_SetTLVData(0x81, aucStrAmtSum, 4);
    if (nRet)
    {
        return nRet;
    }
    nRet=EMV_II_SetTLVData(0x9F02, aucNumAmtSum, 6);
    if (nRet)
    {
        return nRet;
    }
    if (ucTransType == 0x09)//cashback
    {
        memset(aucStrAmtOther, 0, sizeof(aucStrAmtOther));
        LongToStr_emv(ulCashBackAmt, aucStrAmtOther);
        nRet=EMV_II_SetTLVData(0x9F04, aucStrAmtOther, 4);
        if (nRet)
        {
            return nRet;
        }
        nRet=EMV_II_SetTLVData(0x9F03, aucNumAmtOther, 6);
        if (nRet)
        {
            return nRet;
        }
    }
    
    return EMV_OK;
}

int InputAmt_emv(void)
{
    int nRet=0,nLen=0;
    unsigned char ucTransType=0;
    unsigned char aucAuthAmt[14], aucCashBackAmt[14];
    
    nRet=EMV_II_GetTLVData(0x9C, 1, &ucTransType,(unsigned int *)&nLen);
    if(nRet)
    {
        return nRet;
    }
    
    if(ucTransType & EMV_CASHBACK)    
    {
#ifdef _ANDROID_TERM
        if(k_gl_stCallBack.cEMVInputAmount)
        {
             nRet = k_gl_stCallBack.cEMVInputAmount(&gl_ulAuthAmt, &gl_ulCashBackAmt);
        }   
        else
        {
            return EMV_NO_CBFUN;
        }
#else
        nRet = cEMVInputAmount(&gl_ulAuthAmt, &gl_ulCashBackAmt);
#endif
    }
    else
    {
#ifdef _ANDROID_TERM
        if(k_gl_stCallBack.cEMVInputAmount)
        {
            nRet = k_gl_stCallBack.cEMVInputAmount(&gl_ulAuthAmt, NULL);    
        }
        
        else
        {
            return EMV_NO_CBFUN;
        }
#else
        nRet = cEMVInputAmount(&gl_ulAuthAmt, NULL);
#endif
    }
    
    if (nRet)
    {
        return nRet;
    }

    gl_nInputAmtFlag=1;

    memset(aucAuthAmt, 0, sizeof(aucAuthAmt));
    memset(aucCashBackAmt, 0, sizeof(aucCashBackAmt));
    
    sprintf((char *)aucAuthAmt, "%012lu", gl_ulAuthAmt);
    TwoOne_emv(aucAuthAmt,12,aucAuthAmt);
    sprintf((char *)aucCashBackAmt, "%012lu", gl_ulCashBackAmt);
    TwoOne_emv(aucCashBackAmt,12,aucCashBackAmt);
    nRet = SaveAmtToEMVII_emv(aucAuthAmt, aucCashBackAmt);

    return nRet;
}

void ConvCandList_emv(EMV_II_CANDLIST tCandAppList, EMV_CANDLIST *ptCandAppList_old)
{
    memcpy(ptCandAppList_old->aucAID, tCandAppList.aucAID, tCandAppList.ucAidLen);
    ptCandAppList_old->ucAidLen = tCandAppList.ucAidLen;
    ptCandAppList_old->ucPriority = tCandAppList.ucPriority;
    strcpy((char*)ptCandAppList_old->aucAppLabel, (char*)tCandAppList.aucAppLabel);
    strcpy((char*)ptCandAppList_old->aucAppName, (char*)tCandAppList.aucAppName);
    strcpy((char*)ptCandAppList_old->aucAppPreName, (char*)tCandAppList.aucAppPreName);
    strcpy((char*)ptCandAppList_old->aucIssDiscrData, (char*)tCandAppList.aucIssDiscrData);
}

int  CandAppSel_emv(int TryCnt, EMV_CANDLIST List[], int AppNum)
{
    EMV_APPLIST atAppList[MAX_APP_ITEMS];
    int i = 0;

    memset(atAppList, 0, MAX_APP_ITEMS*sizeof(EMV_APPLIST));
    for (i = 0; i<AppNum; i++)
    {
        memcpy(atAppList[i].AID, List[i].aucAID, List[i].ucAidLen);
        atAppList[i].AidLen = List[i].ucAidLen;
        memcpy(atAppList[i].AppName, List[i].aucAppName, sizeof(List[i].aucAppName));
        atAppList[i].Priority = List[i].ucPriority;
    }
#ifdef _ANDROID_TERM
    if(k_gl_stCallBack.cEMVWaitAppSel)
    {
        return k_gl_stCallBack.cEMVWaitAppSel(TryCnt, atAppList, AppNum);
    }
    else
    {
        return EMV_NO_CBFUN;
    }
#else
    return cEMVWaitAppSel(TryCnt, atAppList, AppNum);
#endif
}

int  EMVAppSelectForLog(int Slot, unsigned char ucFlg)
{
    int nRet = 0, i=0;
    EMV_II_CANDLIST atCandAppList[MAX_APP_ITEMS];
    EMV_CANDLIST atCandAppList_old[MAX_APP_ITEMS];
    int nAppNum = 0;
    int nSelTryCnt=0;
    int nAppSelNo=0;

    DEVICE_SetIccSlot((unsigned char)Slot);
    nRet = DEVICE_IccReset();
    if (nRet)
    {
        return ICC_RESET_ERR;
    }
    
    LoadAllApptoEMVIIKern_emv();

    memset(atCandAppList, 0, sizeof(atCandAppList));
    if(ucFlg == 0) 
        ucFlg = 1;
    else
        ucFlg = 0;
    EMV_II_SetAppSelectForLog_PBOC(1, ucFlg);
    nRet = EMV_II_AppSelect(atCandAppList,&nAppNum);
    if(nRet)
    {
        return nRet;
    }
    
    nSelTryCnt=0;
    
    if ((nAppNum == 1) && (!nSelTryCnt) && !(atCandAppList[0].ucPriority & 0x80))
    {
        nRet=EMV_II_FinalSelect(0,atCandAppList, &nAppNum);
    }
    else while(1)
    {
        memset(atCandAppList_old, 0, sizeof(atCandAppList_old));
        for (i=0;i<nAppNum;i++)
        {
            ConvCandList_emv(atCandAppList[i], &atCandAppList_old[i]);
        }

		//Automatically select the candidate application if the candlist contains only one application
		if((nAppNum == 1) && !(atCandAppList[0].ucPriority & 0x80)&& (gl_ucAppConfimMethod == 0))
		{
			nAppSelNo=0;
			nRet= EMV_II_FinalSelect(0,atCandAppList, &nAppNum);
			break;
		}
		
        nRet = CandAppSel_emv(nSelTryCnt, atCandAppList_old, nAppNum);
        nSelTryCnt++;
        if(nRet<0)
        {
            return nRet;
        }
        nAppSelNo=nRet;
        nRet= EMV_II_FinalSelect((unsigned char)nAppSelNo,atCandAppList, &nAppNum);
        
        if (nRet != EMV_SELECT_NEXT)
        {
            break;
        }
    }    

    if (nRet)
    {
        return nRet;
    }

    return EMV_OK;
}

void LoadFinalAppParaToEMVII_emv(void)
{
    EMV_APPLIST tEMV_APP;
    
    EMVGetFinalAppPara(&tEMV_APP);
    EMVModFinalAppPara(&tEMV_APP);
}

int CheckDOLRelatedData_emv(unsigned char *aucDOL, unsigned int unDOLLen)
{
    int i, len;
    unsigned short Tag;
    unsigned char buff[256];
    unsigned char *paucDOLStart, *paucDOLEnd;
    unsigned char *pucTemTag = NULL, aucTemTag[4];//PUB_MAX_TAG_LEN
    
    if (aucDOL == NULL)
    {
        return EMV_PARAM_ERR;
    }

    paucDOLStart = aucDOL;
    paucDOLEnd = aucDOL+unDOLLen;
    memset(buff,0,sizeof(buff));
    while (paucDOLStart < paucDOLEnd) 
    {
        pucTemTag = paucDOLStart;
        Tag = *paucDOLStart++;
        if (Tag == 0) 
            break;
        if ((Tag & 0x1F) == 0x1F) 
        {
            Tag <<= 8;
            Tag += *paucDOLStart++;
            if (Tag & 0x80) 
            {
                while (paucDOLStart < paucDOLEnd && (*paucDOLStart & 0x80)) 
                    paucDOLStart++;
                if (paucDOLStart >= paucDOLEnd) 
                    return EMV_DATA_ERR;
                Tag = 0;
            }
        }

        memset(aucTemTag, 0, sizeof(aucTemTag));
        memcpy(aucTemTag, pucTemTag, paucDOLStart-pucTemTag);
        
        len = *paucDOLStart++;

        i = EMV_II_IsKnowTag(aucTemTag);
        if (i < 0) 
        {
#ifdef _ANDROID_TERM
            if(k_gl_stCallBack.cEMVUnknowTLVData)
            {
                if (k_gl_stCallBack.cEMVUnknowTLVData(Tag, buff, len)) 
                {
                    memset(buff,0,sizeof(buff));
                    EMVSetTLVData(Tag, buff, len);
                }
                else
                {
                    EMVSetTLVData(Tag, buff, len);
                }
            }
            else
            {
                return EMV_NO_CBFUN;
            }
#else
            if (cEMVUnknowTLVData(Tag, buff, len)) 
            {
                memset(buff,0,sizeof(buff));
                EMVSetTLVData(Tag, buff, len);
            }
            else
            {
                EMVSetTLVData(Tag, buff, len);
            }
                
#endif
        }
    }
    return 0;
}

int EMVAppSelect(int Slot, unsigned long TransNo) 
{
    unsigned char aucPDOL[256], ucTransType;
    int InmputAmt=0;
    unsigned int nPDOLLen = 0, nLen=0;
    int nRet =0 ;
    EMV_II_CANDLIST atCandAppList[MAX_APP_ITEMS];
    EMV_CANDLIST atCandAppList_old[MAX_APP_ITEMS];
    int nAppNum = 0, i=0;
    int nSelTryCnt=0;
    int nAppSelNo=0;
    unsigned char buf[20], buff[100];
    
#ifdef _LINUX_TERM
    DEVICE_CALLBACK stCallback;

    stCallback.cDevice_getkey = NULL;
    stCallback.cDevice_cEMVIccIsoCommand = NULL;
    stCallback.cDevice_PedVerifyPlainPin = NULL;
    stCallback.cDevice_PedVerifyCipherPin = NULL;
    stCallback.cRFU2 = NULL;

    stCallback.cDevice_cEMVIccIsoCommand = cEMVIccIsoCommand;
    stCallback.cDevice_PedVerifyPlainPin = cEMVPedVerifyPlainPin;
    stCallback.cDevice_PedVerifyCipherPin = cEMVPedVerifyCipherPin;

    DEVICE_SetCallback(&stCallback);//rename DEVICESetCallback[lijian 2018-02-06]
#endif
	
    SetAPICalledLog_emv(EMV_API_ID_EMVAppSelect);

    gl_nEnableConfirmAmt = 0;

    DEVICE_SetIccSlot((unsigned char)Slot);
    nRet = DEVICE_IccReset();
    if (nRet)
    {
        return ICC_RESET_ERR;
    }

    LoadAllApptoEMVIIKern_emv();

	EMV_II_SetAppSelectForLog_PBOC(0, 0);

	memset(atCandAppList, 0, sizeof(atCandAppList));
	nAppNum = 0;
	if (gl_ucSkipCreCandiListFlg == 0)//Skip creating candidate list for EMVAppSelect
	{	
		nRet = EMV_II_AppSelect(atCandAppList,&nAppNum);
	}
	else
	{
		nRet = EMV_II_GetCandList(atCandAppList,&nAppNum);// use the Candidate list set by application
	}
	if(nRet)
	{
		return nRet;
	}
    
    nSelTryCnt=0;

    while(1)
    {
        if ((nAppNum == 1) && (!nSelTryCnt) && !(atCandAppList[0].ucPriority & 0x80)&& (gl_ucAppConfimMethod == 0))
        {
            nAppSelNo=0;
            nRet=EMV_II_FinalSelect(0,atCandAppList, &nAppNum);
        }
        else while(2)
        {
            memset(atCandAppList_old, 0, sizeof(atCandAppList_old));
            for (i=0;i<nAppNum;i++)
            {
                ConvCandList_emv(atCandAppList[i], &atCandAppList_old[i]);
            }

			//Automatically select the candidate application if the candlist contains only one application
			if((nAppNum == 1) && !(atCandAppList[0].ucPriority & 0x80)&& (gl_ucAppConfimMethod == 0))
			{
				nAppSelNo=0;
				nRet= EMV_II_FinalSelect(0,atCandAppList, &nAppNum);
				break;
			}
		
            nRet = CandAppSel_emv(nSelTryCnt, atCandAppList_old, nAppNum);
            nSelTryCnt++;
            if(nRet<0)
            {
                return nRet;
            }
            nAppSelNo=nRet;
            nRet= EMV_II_FinalSelect((unsigned char)nAppSelNo,atCandAppList, &nAppNum);
            if (nRet != EMV_SELECT_NEXT)
            {
                break;
            }
        }//while (2)

        if (nRet)
        {
            return nRet;
        }

        memset(gl_aucFinalAID, 0, sizeof(gl_aucFinalAID));
        memcpy(gl_aucFinalAID, atCandAppList[nAppSelNo].aucAID, atCandAppList[nAppSelNo].ucAidLen);
        gl_ucFinalAIDLen = atCandAppList[nAppSelNo].ucAidLen;

        LoadFinalAppParaToEMVII_emv();
#ifdef _ANDROID_TERM
        if(k_gl_stCallBack.cEMVSetParam)
        {
            nRet = k_gl_stCallBack.cEMVSetParam(); 
            if (nRet) 
            {
                return nRet;
            }
        }
        else
        {
            return EMV_NO_CBFUN;
        }
#else
        nRet = cEMVSetParam();
        if (nRet)
        {
            return nRet;
        }
#endif
        while (3) 
        {
            InmputAmt = 1;
            memset(aucPDOL, 0, sizeof(aucPDOL));
            EMV_II_GetTLVData(0x9F38, sizeof(aucPDOL), aucPDOL, &nPDOLLen);

            if (SearchTLV_emv(1, 0x81, aucPDOL, aucPDOL+nPDOLLen, NULL) != NULL) 
            {
                break;
            }

            if (SearchTLV_emv(1, 0x9F02, aucPDOL, aucPDOL+nPDOLLen, NULL) != NULL) 
            {
                break;
            }
        
            EMV_II_GetTLVData(0x9C, 1, &ucTransType, &nLen);
            if (ucTransType == 0x09
                && (SearchTLV_emv(1, 0x9F03, aucPDOL, aucPDOL+nPDOLLen, NULL) == NULL
                || SearchTLV_emv(1, 0x9F04, aucPDOL, aucPDOL+nPDOLLen, NULL) == NULL))
            {
                break;
            }
            
            InmputAmt = 0;
            break;
        }//while (3)

        gl_nInputAmtFlag=0;
        
        if (InmputAmt)
        {
            nRet = InputAmt_emv();
            if(nRet)
            {
                return nRet;
            }
        }
        
        sprintf((char *)buf, "%08lu", TransNo);
        memset(buff, 0, sizeof(buff));
        TwoOne_emv(buf, 8, buff);
        nRet = EMVSetTLVData(0x9F41, buff, 4);

        
#ifdef _ANDROID_TERM 
         if(k_gl_stCallBack.cEMVUnknowTLVData)
        {
            if (!k_gl_stCallBack.cEMVUnknowTLVData(0x9F37, buff, 4))//get UN
            {
                EMVSetTLVData(0x9F37, buff, 4);
            }
            if (!k_gl_stCallBack.cEMVUnknowTLVData(0x9A, buff, 3))//Get transaction date
            {
                EMVSetTLVData(0x9A, buff, 3);
            }
            if (!k_gl_stCallBack.cEMVUnknowTLVData(0x9F21, buff, 3))//Get transaction time
            {
                EMVSetTLVData(0x9F21, buff, 3);
            }
            if (!k_gl_stCallBack.cEMVUnknowTLVData(0x9F1E, buff, 8))//Get terminal SN
            {
                EMVSetTLVData(0x9F1E, buff, 8);
            }
        }
        else
        {
            return EMV_NO_CBFUN;
        }
#endif
#ifdef _LINUX_TERM
        if (!cEMVUnknowTLVData(0x9F37, buff, 4))//get UN
        {
            EMVSetTLVData(0x9F37, buff, 4);
        }
        if (!cEMVUnknowTLVData(0x9A, buff, 3))//Get transaction date
        {
            EMVSetTLVData(0x9A, buff, 3);
        }
        if (!cEMVUnknowTLVData(0x9F21, buff, 3))//Get transaction time
        {
            EMVSetTLVData(0x9F21, buff, 3);
        }
        if (!cEMVUnknowTLVData(0x9F1E, buff, 8))//Get terminal SN
        {
            EMVSetTLVData(0x9F1E, buff, 8);
        }
#endif
        CheckDOLRelatedData_emv(aucPDOL, nPDOLLen);

        nRet = EMV_II_InitApp(atCandAppList, &nAppNum);
        if(( nRet == EMV_APP_BLOCK )||(nRet == ICC_RSP_6985 ))
        {
            if(nAppNum == 0)
                return nRet;
            continue;
        }
        break;
    }//while (1)
    return nRet;
}

int EMVReadAppData(void)
{
    int nRet = 0;

    SetAPICalledLog_emv(EMV_API_ID_EMVReadAppData);
    EMV_II_InitTransLog();

    if (!gl_nInputAmtFlag)
    {

        nRet = InputAmt_emv();
        if (nRet)
        {
            return nRet;
        }    
    }
    return EMV_II_ReadAppData();
}

int GetCAPKByIndex_emv(unsigned char ucIndex, unsigned char *pRid,EMV_CAPK *ptEmvCAPK)
{
    int i = 0;
    
    if(ptEmvCAPK == NULL)
    {
        return EMV_PARAM_ERR;
    }
    
    for (i = 0; i < MAX_KEY_NUM; i++)
    {
        if (ucIndex == gl_tCAPKList[i].KeyID) 
        {
            if (memcmp(pRid, gl_tCAPKList[i].RID, 5) == 0) 
            {
                memcpy(ptEmvCAPK,&gl_tCAPKList[i],sizeof(EMV_CAPK));
                return EMV_OK;
            }
        }
    }
    return EMV_NOT_FOUND;
}

int GetRevocListByIndex_emv(unsigned char ucIndex, unsigned char *pRid,EMV_REVOCLIST *ptEmvRevoclist)
{
    int i = 0;
    
    if((pRid == NULL) ||(ptEmvRevoclist == NULL))
    {
        return EMV_PARAM_ERR;
    }

    for (i = 0; i < gl_nRevocListNum; i++) 
    {
        if ((ucIndex == gl_tRevocList[i].ucIndex) 
            && (memcmp(pRid, gl_tRevocList[i].ucRid, 5) == 0)) 
        {
            memcpy(ptEmvRevoclist,&gl_tRevocList[i],sizeof(EMV_REVOCLIST));
            
            return EMV_OK;
        }
    }
    return EMV_NOT_FOUND;
}

int EMVCardAuth(void)
{
    int ret = 0;
    EMV_II_REVOCLIST tEMVIIRevocList;
    EMV_CAPK tEmvCAPK;
    EMV_REVOCLIST tEmvRevoclist;
    unsigned char aucTermAid[17] = {0};
    int nTempLen = 0;
    unsigned char ucPkIndex = 0;

    SetAPICalledLog_emv(EMV_API_ID_EMVCardAuth);

    //set CAPK and Revoclist into EMVII kernel according to the index and RID returned by card.
    EMV_II_DelAllCAPK();
    EMV_II_DelAllRevocList();

    if (EMVGetTLVData(0x8F, &ucPkIndex, &nTempLen) == 0)
    {
        if (EMVGetTLVData(0x9F06, aucTermAid, &nTempLen) == 0)
        {
            ret = GetCAPKByIndex_emv(ucPkIndex, aucTermAid,&tEmvCAPK);
            if(ret == 0)
            {
                ret = EMV_II_AddCAPK((EMV_II_CAPK *)&tEmvCAPK);

                EMVSetTLVData(0x9F22, &ucPkIndex, 1);    //2CA.124.00-2 need 9F22
            }
            ret = GetRevocListByIndex_emv(ucPkIndex,aucTermAid,&tEmvRevoclist);

            if(ret == 0)
            {
                memset(&tEMVIIRevocList, 0, sizeof(EMV_II_REVOCLIST));
                tEMVIIRevocList.ucIndex = tEmvRevoclist.ucIndex;
                memcpy(tEMVIIRevocList.aucCertSn, tEmvRevoclist.ucCertSn, sizeof(tEmvRevoclist.ucCertSn));
                memcpy(tEMVIIRevocList.aucRid, tEmvRevoclist.ucRid, sizeof(tEmvRevoclist.ucRid));
                ret = EMV_II_AddRevocList(&tEMVIIRevocList);
            }
        }
    }
    return EMV_II_CardAuth();
}


int HolderVerify_emv(void)
{
    int nRet =0,nPwdRst=0;
    unsigned char ucCVMType=0,ucPINCnt=0;
    unsigned char aucPINData[30];
    unsigned char aucPINForamt[30];
    unsigned char aucTVR[5];
    unsigned char ucBuff[5];
    unsigned char ucPINTryCount =0;
    int nTryFlg=0;
    int nTVRlen = 0;
    unsigned char ucBypassedOne=0;
    unsigned char ucPINCancelFlag = 0;
    EMV_EXTMPARAM tmpExTmParam;
    EMV_MCKPARAM tMCKParam;

    memset(&tMCKParam, 0, sizeof(EMV_MCKPARAM));
    memset(&tmpExTmParam, 0, sizeof(EMV_EXTMPARAM));
    
    tMCKParam.pvoid = &tmpExTmParam;
    EMVGetMCKParam(&tMCKParam);

    memset(aucPINData,0,sizeof(aucPINData));
    memset(aucPINForamt,0,sizeof(aucPINForamt));
    memset(aucTVR,0,sizeof(aucTVR));
    memset(ucBuff,0,sizeof(ucBuff));

    while(1)
    {
        ucPINCancelFlag = 0;
        nRet=EMV_II_StartCVM(&ucCVMType,&ucPINCnt);
        if(nRet)
        {
            if(nRet == EMV_QUIT_CVM)
                return EMV_OK;
            else
                return nRet;
        }

        
        
        if(ucCVMType == EMV_CVM_CERTIFICATE)
        {
#ifdef _ANDROID_TERM
            if(k_gl_stCallBack.cCertVerify)
            {
                nPwdRst = k_gl_stCallBack.cCertVerify();
            }
            else
            {
                return EMV_NO_CBFUN;
            }
#else
            nPwdRst = cCertVerify();
#endif
            nRet=EMV_II_CompleteCVM(nPwdRst,NULL,&ucPINTryCount);
        }
        else if(ucCVMType == EMV_CVM_SIGNATURE || ucCVMType == EMV_CVM_FAIL_CVM || ucCVMType == EMV_CVM_NO_CVM)    
        {
            nRet=EMV_II_CompleteCVM(0,NULL,&ucPINTryCount);
        }
        else if(ucCVMType == EMV_CVM_ONLINE_PIN)
        {
            if (ucBypassedOne == 1 && tmpExTmParam.ucBypassAllFlg == 1)
            {
                nRet = EMV_NEXT_CVM;
                continue;
            }

#ifdef _ANDROID_TERM
             if(k_gl_stCallBack.cEMVGetHolderPwd)
            {
                nPwdRst = k_gl_stCallBack.cEMVGetHolderPwd(0,0,NULL);
				// The cardholder pressed CLEAR button when amount confirmation is required.
				if (nPwdRst == EMV_USER_CLEAR)
				{
					return EMV_USER_CLEAR;
				}
            }
            else
            {
                return EMV_NO_CBFUN;
            }
#else
            nPwdRst = cEMVGetHolderPwd(0,0,NULL);
#endif
            gl_ucPinInput = 1;
            nRet =EMV_II_CompleteCVM(nPwdRst,NULL,&ucPINTryCount);
            if (nRet == EMV_NEXT_CVM) 
            {
                if(nPwdRst == EMV_NO_PASSWORD)
                {
                    ucBypassedOne = 1;
                    continue;
                }
            }
			else if (nRet == EMV_USER_CLEAR)
			{
				return EMV_USER_CLEAR;
			}
        }
        else
        {
            while(2)
            {
                if (ucBypassedOne == 1 && tmpExTmParam.ucBypassAllFlg == 1) 
                {
                    nRet = EMV_NEXT_CVM;
                    break;
                }

#ifdef _ANDROID_TERM
                 if(k_gl_stCallBack.cEMVGetHolderPwd)
                {
                    nPwdRst = k_gl_stCallBack.cEMVGetHolderPwd(nTryFlg, ucPINCnt, aucPINData);
					// The cardholder pressed CLEAR button when amount confirmation is required.
					if (nPwdRst == EMV_USER_CLEAR)
					{
						return EMV_USER_CLEAR;
					}
                }    
                else
                {
                    return EMV_NO_CBFUN;
                }
#else
                nPwdRst = cEMVGetHolderPwd(nTryFlg, ucPINCnt, aucPINData);
#endif
                gl_ucPinInput = 1;
                if(DEVICE_IccGetTxnIF() == DEVICE_CONTACT_TXNIF)
                {
                    if (gl_ucEmvPciUsedFlg == 1)
                    {
                        aucPINForamt[0] = 0x00;
                        aucPINForamt[1] = 0x00;
                    }
                    else
                    {
                        aucPINForamt[0] = 0x01;
                        aucPINForamt[1] = strlen((char*)aucPINData);
                    }
                    memcpy(&aucPINForamt[2],aucPINData,aucPINForamt[1]);
                    nRet=EMV_II_CompleteCVM(nPwdRst,aucPINForamt,&ucPINTryCount);
                }
                else
                {
                    aucPINForamt[0] = 0x01;
                    aucPINForamt[1] = (unsigned char)(strlen((char*)aucPINData));
                    memcpy(&aucPINForamt[2],aucPINData,aucPINForamt[1]);
                    nRet=EMV_II_CompleteCVM(nPwdRst,aucPINForamt,&ucPINTryCount);
                }
                if(nRet)
                {
                    if(nRet == EMV_USER_CANCEL && ucPINCancelFlag == 0 && gl_ucCancelPinFlag == 1)// for Italy customer
                    {
                        if(k_gl_stCallBack.cEMVVerifyPINfailed)
                        {
                            nRet = k_gl_stCallBack.cEMVVerifyPINfailed(ucBuff);
                        }
                        else
                        {
                            return EMV_NO_CBFUN;
                        }

                        if (nRet == 0)
                        {
                            ucPINCancelFlag = 1;
                            continue;
                        }
                        else
                        {
                            nRet = EMV_NEXT_CVM;
							nTVRlen = 0;
							memset(aucTVR,0,sizeof(aucTVR));
                            EMVGetTLVData(0x95, aucTVR, &nTVRlen);
                            aucTVR[2] |= 0x08;//PIN entry required, PIN pad present, but PIN was not entered
                            EMVSetTLVData(0x95, aucTVR, nTVRlen);
                        }
                        
                    }
                    else if(nRet == EMV_NEXT_CVM)
                    {
						nTVRlen = 0;
						memset(aucTVR,0,sizeof(aucTVR));
						if(gl_ucShowPinExceedFlag == 1)// for Japanese customer
						{
#ifdef _ANDROID_TERM
							if(k_gl_stCallBack.cEMVGetHolderPwd)
							{
								k_gl_stCallBack.cEMVGetHolderPwd(nTryFlg, ucPINTryCount, aucPINData);
							}
							else
							{
								return EMV_NO_CBFUN;
							}
#else
							cEMVGetHolderPwd(nTryFlg, ucPINTryCount, aucPINData);
#endif
						}
                        EMVGetTLVData(0x95, aucTVR, &nTVRlen);
						if((aucTVR[2] &0x08 ) == 0x08)//PIN entry required, PIN pad present, but PIN was not entered
						{
							ucBypassedOne = 1;
						}
                        break;
                    }

                    else if(nRet == EMV_PIN_TRYAGAIN)
                    {
                        nTryFlg =1;
                        ucPINCnt = ucPINTryCount;
                        memset(aucPINData,0,sizeof(aucPINData));
                        continue;
                    }
                    else if (nRet == EMV_PED_FAIL)
                    {
                        aucPINData[0] = aucPINForamt[0];//EMV_PED_WAIT and EMV_PED_TIMEOUT will be outputted for PCI mode [8/9/2017 zhoujie]
                        continue;
                    }
					else if (nRet == EMV_USER_CLEAR)
					{
						return EMV_USER_CLEAR;
					}
                }
                else//  [9/29/2016 zhoujie]
                {
#ifdef _ANDROID_TERM
                    if(k_gl_stCallBack.cEMVVerifyPINOK)
                    {
                        k_gl_stCallBack.cEMVVerifyPINOK();
                    }
                    else
                    {
                        return EMV_NO_CBFUN;
                    }
#else
                    cEMVVerifyPINOK();
#endif
                }
                break;
            }
        }//if(ucCVMType)

        if(nRet != EMV_NEXT_CVM)
            return nRet;
    }
    return 0;
}

int  EMVStartTrans (unsigned long ulAuthAmt, unsigned long ulCashBackAmt, unsigned char *pACType)
{
    unsigned char buff[20]; 
    int ret;
    unsigned int len;
    int ucECOnlinePINFlg;
    unsigned char aucAuthAmt[13], aucCashBackAmt[13], aucLogAmt[13];
#ifdef _PAXME_TERM
    unsigned char aucPAN[10], ucPANSeq;
    int nPANLen = 0;
    unsigned long ulLogAmt = 0;
#endif

    SetAPICalledLog_emv(EMV_API_ID_EMVStartTrans);
     if (pACType == NULL)
    {
        return EMV_PARAM_ERR;
    }
    *pACType = AC_AAC; 

    //if big amount is not set, ulAuthAmt and ulCashBackAmt will be saved here.
    memset(buff, 0, sizeof(buff));
    EMV_II_GetTLVData(0x9F02, sizeof(buff), buff, &len);
    if (!(len != 0 && (memcmp(buff, "\x00\x42\x94\x96\x72\x95", 6) >= 0)))
    {
        memset(aucAuthAmt, 0, sizeof(aucAuthAmt));
        memset(aucCashBackAmt, 0, sizeof(aucCashBackAmt));
        
        sprintf((char *)aucAuthAmt, "%012lu", ulAuthAmt);
        TwoOne_emv(aucAuthAmt,12,aucAuthAmt);
        sprintf((char *)aucCashBackAmt, "%012lu", ulCashBackAmt);
        TwoOne_emv(aucCashBackAmt,12,aucCashBackAmt);
        
        SaveAmtToEMVII_emv(aucAuthAmt, aucCashBackAmt);
    }

    EMV_II_ProcRestric();

    gl_ucPinInput = 0;
    ret = 0;
    ret = HolderVerify_emv(); 
    if (ret)
    {
        return ret;  
    }
    
     if (gl_nEnableConfirmAmt && gl_ucPinInput == 0)
    {
#ifdef _ANDROID_TERM
        if(k_gl_stCallBack.cEMVGetHolderPwd)
        {
            ret = k_gl_stCallBack.cEMVGetHolderPwd(2, 0, NULL);
        }
        else
        {
            return EMV_NO_CBFUN;
        }
#else
        ret = cEMVGetHolderPwd(2, 0, NULL);
#endif
        if (ret) 
            return ret;
    }

#ifdef _PAXME_TERM
    if (gl_nTransLog == 1)
    {
        EMVGetTLVData(0x5F34, &ucPANSeq, &nPANLen);
        EMVGetTLVData(0x5A, aucPAN, &nPANLen);
        ulLogAmt = GetTransLogAmt_emv(ucPANSeq, aucPAN, (unsigned char)nPANLen);
        
        memset(aucLogAmt, 0, sizeof(aucLogAmt));
        sprintf((char *)aucLogAmt, "%012lu", ulLogAmt);
        TwoOne_emv(aucLogAmt,12,aucLogAmt);
        EMVSetTLVData(EMV_LAST_TRANS_AMOUNT, aucLogAmt, 6);
    }
#else
#ifdef _ANDROID_TERM
    if(k_gl_stCallBack.cEMVUnknowTLVData)
    {
        if(!k_gl_stCallBack.cEMVUnknowTLVData(EMV_LAST_TRANS_AMOUNT, aucLogAmt, 6))
        {
            EMVSetTLVData(EMV_LAST_TRANS_AMOUNT, aucLogAmt, 6);
        }
    }
    else
    {
        return EMV_NO_CBFUN;
    }
#else
    if(!cEMVUnknowTLVData(EMV_LAST_TRANS_AMOUNT, aucLogAmt, 6))
    {
        EMVSetTLVData(EMV_LAST_TRANS_AMOUNT, aucLogAmt, 6);
    }
#endif
#endif
    
    EMV_II_RiskManagement(0);
    
    ret = EMV_II_TermActAnalyse(pACType);
    
    EMV_II_GetParamFlag(0x03, &ucECOnlinePINFlg);
    if(ucECOnlinePINFlg == 1)
    {
#ifdef _ANDROID_TERM
        if(k_gl_stCallBack.cEMVGetHolderPwd)
        {
           k_gl_stCallBack.cEMVGetHolderPwd(0, 0, NULL);
        }
        else
        {
            return EMV_NO_CBFUN;
        }
#else
        cEMVGetHolderPwd(0, 0, NULL);
#endif
    }
	if(ret)
	{
		return ret;
	}
#ifdef _PAXME_TERM
	if ((gl_nTransLog == 1) && (*pACType == AC_TC))
    {
        EMVGetTLVData(0x5F34, &ucPANSeq, &nPANLen);
        EMVGetTLVData(0x5A, aucPAN, &nPANLen);    
        SaveAmtToTransLog_emv(ucPANSeq, aucPAN, (unsigned char)nPANLen, gl_ulAuthAmt);
    }
#endif
    return ret;
    
}

int EMVCompleteTrans(int nCommuStatus, unsigned char *paucScript, int *pnScriptLen, unsigned char *pACType)
{
    unsigned char aucAppScriptRst[80];
    int nAppScrRstLen=0;
	int nRet = 0;
#ifdef _PAXME_TERM
	unsigned char aucPAN[10], ucPANSeq;
    int nPANLen = 0;
#endif
    SetAPICalledLog_emv(EMV_API_ID_EMVCompleteTrans);

    if (paucScript==NULL || pnScriptLen==NULL || pACType==NULL)
    {
        return EMV_PARAM_ERR;
    }
    memset(aucAppScriptRst, 0, sizeof(aucAppScriptRst));
    nRet = EMV_II_CompleteTrans(nCommuStatus, paucScript, pnScriptLen, aucAppScriptRst,&nAppScrRstLen, pACType);
	if(nRet)
	{
		return nRet;
	}
#ifdef _PAXME_TERM
	if ((gl_nTransLog == 1)&& ((*pACType == AC_TC)))
    {
        EMVGetTLVData(0x5F34, &ucPANSeq, &nPANLen);
        EMVGetTLVData(0x5A, aucPAN, &nPANLen);    
        SaveAmtToTransLog_emv(ucPANSeq, aucPAN, (unsigned char)nPANLen, gl_ulAuthAmt);
    }
#endif
	return nRet;
}

//(PBOC2.0)
int ReadLogRecord(int RecordNo)
{
    int nRet=0,nLogLen;
    unsigned char aucLogData[300];

    memset(aucLogData, 0, sizeof(aucLogData));
    nRet = EMV_II_ReadLogRecord_PBOC(0, RecordNo,aucLogData,&nLogLen);

    return nRet;
}

int GetLogItem(unsigned short Tag, unsigned char *TagData, int *TagLen)
{
    if(TagData == NULL || TagLen == NULL)
    {
        return EMV_PARAM_ERR;
    }
    return EMV_II_GetLogItemChild(0, Tag, TagData, TagLen);
}

int EMVReadSingleLoadLog(int nRecordNoIn)
{
    int nRet=0,nLogLen;
    unsigned char aucLogData[300];

    memset(aucLogData, 0, sizeof(aucLogData));
    nRet = EMV_II_ReadLogRecord_PBOC(1, nRecordNoIn,aucLogData,&nLogLen);
    
    return nRet;
}

int EMVGetSingleLoadLogItem(unsigned short usTagIn, unsigned char *paucDataOut, int *pnLenOut)
{
    if(paucDataOut == NULL || pnLenOut == NULL)
    {
        return EMV_PARAM_ERR;
    }
    return EMV_II_GetLogItemChild(1, usTagIn, paucDataOut, pnLenOut);    
}

int EMVReadAllLoadLogs(unsigned char *paucLogDataOut, int *pnLenOut)
{
    if (paucLogDataOut==NULL || pnLenOut==NULL)
    {
        return EMV_PARAM_ERR;
    }

    return EMV_II_ReadLogRecord_PBOC(2, 0, paucLogDataOut, pnLenOut);
}

int EMVGetLogData(unsigned char *paucLogDataOut, int *pnLenOut)
{
    if (paucLogDataOut == NULL || pnLenOut == NULL)
    {
        return EMV_PARAM_ERR;
    }

    return EMV_II_GetLogData(paucLogDataOut,pnLenOut);
}

int EMVGetCardECBalance(unsigned long *plBalance)
{
    if (plBalance==NULL)
    {
        return EMV_PARAM_ERR;
    }

    return EMV_II_GetCardECBalance_PBOC(plBalance);
}

int EMVSetTmECParam(EMV_TMECPARAM *tParam)//for PBOC 
{
    EMV_II_TMECPARAM tEMVECPARAM;

    if (tParam==NULL)
    {
        return EMV_DATA_ERR;
    }

    memset(&tEMVECPARAM, 0, sizeof(tEMVECPARAM));
    tEMVECPARAM.ucECTSIFlg = tParam->ucECTSIFlg;
    tEMVECPARAM.ucECTSIVal = tParam->ucECTSIVal;
    tEMVECPARAM.ucECTTLFlg = tParam->ucECTTLFlg;
    tEMVECPARAM.ulECTTLVal = tParam->ulECTTLVal;
    return EMV_II_SetTmECParam_PBOC(&tEMVECPARAM);
}

int EMVSetMCKParam(EMV_MCKPARAM *pMCKParam)
{
    EMV_II_TERMPARAM tTermParam;
    EMV_EXTMPARAM tExTmParam;

    
    if (pMCKParam == NULL) 
    {
        return EMV_DATA_ERR;
    }
    
    SetAPICalledLog_emv(EMV_API_ID_EMVSetMCKParam);

    memset(&tTermParam, 0, sizeof(tTermParam));
    EMV_II_GetTermParam(&tTermParam);

    tTermParam.ucBypassPin = pMCKParam->ucBypassPin;
    tTermParam.ucBatchCapture = pMCKParam->ucBatchCapture;
    if (pMCKParam->pvoid != NULL)
    {
        memset(&tExTmParam, 0, sizeof(tExTmParam));
        memcpy(&tExTmParam, pMCKParam->pvoid, sizeof(EMV_EXTMPARAM));
        tTermParam.ucUseTermAIPFlg = tExTmParam.ucUseTermAIPFlg;
        memcpy(tTermParam.aucTermAIP, tExTmParam.aucTermAIP, sizeof(tTermParam.aucTermAIP));
        tTermParam.ucBypassAllFlg = tExTmParam.ucBypassAllFlg;
    }
    EMV_II_SetTermParam(&tTermParam);
    return EMV_OK;
}

int EMVGetMCKParam(EMV_MCKPARAM *pMCKParam)
{
    EMV_II_TERMPARAM tTermParam;
    EMV_EXTMPARAM tExTmParam;

    SetAPICalledLog_emv(EMV_API_ID_EMVGetMCKParam);
    if (pMCKParam == NULL) 
    {
        return EMV_DATA_ERR;
    }

    memset(&tTermParam, 0, sizeof(tTermParam));
    EMV_II_GetTermParam(&tTermParam);
    
    pMCKParam->ucBypassPin = tTermParam.ucBypassPin;
    pMCKParam->ucBatchCapture = tTermParam.ucBatchCapture;
    memset(&tExTmParam, 0, sizeof(tExTmParam));
    tExTmParam.ucUseTermAIPFlg = tTermParam.ucUseTermAIPFlg;
    memcpy(tExTmParam.aucTermAIP, tTermParam.aucTermAIP, sizeof(tTermParam.aucTermAIP));
    tExTmParam.ucBypassAllFlg = tTermParam.ucBypassAllFlg;
    if(pMCKParam->pvoid != NULL)
    {
        memcpy(pMCKParam->pvoid,&tExTmParam,sizeof(EMV_EXTMPARAM));

    }
    
    return EMV_OK;
}


int EMVReadVerInfo(char *paucVer)
{
    SetAPICalledLog_emv(EMV_API_ID_EMVReadVerInfo);
    if(paucVer == NULL)
    {
        return EMV_PARAM_ERR;
    }

    return EMV_II_ReadVerInfo(paucVer);
}

void EMVSetConfigFlag(int nConfigflag)
{
    EMV_II_TERMPARAM tTermParam;
    
    SetAPICalledLog_emv(EMV_API_ID_EMVSetConfigFlag);
    memset(&tTermParam, 0, sizeof(tTermParam));
    EMV_II_GetTermParam(&tTermParam);

    if (nConfigflag & 0x0001)
    {
        tTermParam.ucAdviceFlg = 1;
    }
    else
    {
        tTermParam.ucAdviceFlg = 0;
    }
    if (nConfigflag & 0x0002)
    {
        gl_nEnableConfirmAmt = 1;
    }
    else
    {
        gl_nEnableConfirmAmt = 0;
    }
#ifdef _PAXME_TERM
    if (nConfigflag & 0x0004)
    {
        gl_nTransLog = 1;
    }
    else
    {
        gl_nTransLog = 0;
    }
#endif
    //litun 2017.12.12
    if (nConfigflag & 0x0010)
    {
        gl_ucAppConfimMethod = 1;
        EMVSetExtendFunc(1,(unsigned char *)"\x01",1);
    }
    else
    {
        gl_ucAppConfimMethod = 0;
    }
    EMV_II_SetTermParam(&tTermParam);
}

int EMVAddIccTag(ELEMENT_ATTR tEleAttr[], int nAddNum)
{
    //EMVII kernel can save all the Tag set by application or responded by ICC.
    //So nothing need to do here.
    return 0;
}

void EMVSetScriptProcMethod(unsigned char ucScriptMethodIn)
{
    EMV_II_TERMPARAM tTermParam;
    
    SetAPICalledLog_emv(EMV_API_ID_EMVSetScriptProcMethod);
    memset(&tTermParam, 0, sizeof(tTermParam));
    EMV_II_GetTermParam(&tTermParam);
    
    tTermParam.ucScriptMethod = ucScriptMethodIn;
    
    EMV_II_SetTermParam(&tTermParam);
}

void EMVInitTLVData(void)
{
    SetAPICalledLog_emv(EMV_API_ID_EMVInitTLVData);
    EMV_II_InitTLVDataClss_PBOC();
}

int EMVSwitchClss(Clss_TransParam *ptTransParam,unsigned char *pucSelData, int nSelLen,
                  unsigned char *pucGPOData, int nGPOLen)
{
    SetAPICalledLog_emv(EMV_API_ID_EMVSwitchClss);
    if (ptTransParam==NULL || pucSelData==NULL|| pucGPOData==NULL)
    {
        return EMV_PARAM_ERR;
    }

    gl_nInputAmtFlag = 1;//The transaction amount is inputted by Clss_TransParam
    return EMV_II_SwitchClss_PBOC(ptTransParam, pucSelData, nSelLen, pucGPOData, nGPOLen);
}

int EMVSetAmount(unsigned char * szAuthAmount, unsigned char * szCashBackAmount)
{
    SetAPICalledLog_emv(EMV_API_ID_EMVSetAmount);
    if (szAuthAmount == NULL)
    {
        return EMV_DATA_ERR;
    }

    return SaveAmtToEMVII_emv(szAuthAmount, szCashBackAmount);
}

int EMVGetParamFlag(unsigned char ucParam, int *pnFlg)
{
    SetAPICalledLog_emv(EMV_API_ID_EMVGetParamFlag);
    if (pnFlg == NULL)
    {
        return EMV_PARAM_ERR;
    }
    return EMV_II_GetParamFlag(ucParam, pnFlg);
}

int EMVSetPCIModeParam(unsigned char ucPciMode, unsigned char *pucExpectPinLen,unsigned long ulTimeoutMs)
{
#ifndef _PCI_PINPAD_
    
    if (ucPciMode >= 1)
    {
        return EMV_DATA_ERR;
    }
    gl_ucEmvPciUsedFlg = 0;

    DEVICE_SetPinInputParam((unsigned char *)"0", 0);
    return 0;
#else
    if (ucPciMode > 1  || (ucPciMode == 1 && pucExpectPinLen == NULL))
    {
        return EMV_DATA_ERR;
    }
    gl_ucEmvPciUsedFlg = ucPciMode;

    DEVICE_SetPinInputParam(pucExpectPinLen, ulTimeoutMs);
    return 0;
#endif
}

int EMVClearTransLog(void)
{
    EMV_II_InitTransLog();
#ifdef _PAXME_TERM 
    InitTransLog_emv(1);
#endif
    return 0;
}

// for quick test, skip offline PIN enter
void EMVSetAutoRunFlag(unsigned char ucAutoRunFlag)
{
    EMV_II_SetAutoRunFlag(ucAutoRunFlag);
}

int EMVGetVerifyICCStatus(unsigned char *pucSWA, unsigned char *pucSWB)
{
    if (pucSWA == NULL || pucSWB == NULL)
    {
        return EMV_PARAM_ERR;
    }
    
    return EMV_II_GetVerifyICCStatus(pucSWA, pucSWB);
}

int  EMVGetCandListSWAB(unsigned char aucCandListSWAB[][2], int *pnAppNum)
{
    int ucExpdatalen = 34;//MAX_APP_ITEMS*2
    int nDataOutLen = 0;
    unsigned char paucDataOut[34] = {0};//MAX_APP_ITEMS*2
    int ret = 0;

    if (aucCandListSWAB==NULL || pnAppNum==NULL ) 
    {
        return EMV_PARAM_ERR;
    }

    ret = EMV_II_GetExtendFunc(2, ucExpdatalen, paucDataOut, &nDataOutLen);
    if (ret != 0)
    {
        return ret;
    }
    memcpy(aucCandListSWAB, paucDataOut, nDataOutLen);
    *pnAppNum = nDataOutLen/2;
    return EMV_OK;
}


int EMVGetExtendFunc(int nFlag, int nExpDataOutLen, unsigned char *paucDataOut, int *pnActualDataOutLen)
{
	EMV_II_CANDLIST atCandList[MAX_APP_ITEMS];
	int nAppNum = 0;
	int nRet = 0;
	int i = 0;

	if (paucDataOut==NULL || pnActualDataOutLen==NULL ) 
	{
		return EMV_PARAM_ERR;
	}

	switch(nFlag)
	{
	case 1:
		memset(atCandList,0,sizeof(atCandList));
		nRet = EMV_II_GetCandList(atCandList, &nAppNum);
		if (nRet)
		{
			return EMV_NO_DATA;
		}
		
		if (nExpDataOutLen < nAppNum)
		{
			return EMV_OVERFLOW;
		}

		*pnActualDataOutLen= nAppNum;	
		
		for (i=0;i<nAppNum;i++)
		{
			memcpy(paucDataOut+i,&atCandList[i].aucRFU[1],1);
		}
		break;
	default:
		break;
	}
	return EMV_OK;
}
//Flag: 
//1
//2
//3
//4
//5 -- Set the flag whether skip creating candidate list for EMVAppSelect
int EMVSetExtendFunc(int nFlag,unsigned char *paucDataIn, int nDataInLen)
{
    if (paucDataIn==NULL) 
    {
        return EMV_PARAM_ERR;
    }
    switch(nFlag)
    {
    case 1:
        if (paucDataIn[0] != 0x01 && paucDataIn[0] != 0x00)
        {
            return EMV_PARAM_ERR;
        }
        if (nDataInLen != 1)
        {
            return EMV_PARAM_ERR;
        }
        EMV_II_SetExtendFunc(2,paucDataIn, nDataInLen);
        break;
    case 2:
        if (paucDataIn[0] != 0x01 && paucDataIn[0] != 0x00)
        {
            return EMV_PARAM_ERR;
        }
        if (nDataInLen != 1)
        {
            return EMV_PARAM_ERR;
        }
        gl_ucCancelPinFlag = paucDataIn[0];
        break;
	case 3:
		if (paucDataIn[0] != 0x80 && paucDataIn[0] != 0x00 && paucDataIn[0] != 0x40) // for Brazil customer 
		{
			return EMV_PARAM_ERR;
		}
		if (nDataInLen != 1)
        {
            return EMV_PARAM_ERR;
        }
		EMV_II_SetExtendFunc(1,paucDataIn,nDataInLen);
		break;
	case 4:
		if (paucDataIn[0] != 0x01 && paucDataIn[0] != 0x00)
        {
            return EMV_PARAM_ERR;
        }
        if (nDataInLen != 1)
        {
            return EMV_PARAM_ERR;
        }
        gl_ucShowPinExceedFlag = paucDataIn[0];
		break;
	case 5:
		if (paucDataIn[0] != 0x00 && paucDataIn[0] != 0x01)
        {
            return EMV_PARAM_ERR;
        }
        if (nDataInLen != 1)
        {
            return EMV_PARAM_ERR;
        }
		gl_ucSkipCreCandiListFlg = paucDataIn[0];
		break;
    default:
        break;
    }
    return EMV_OK;
}

int EMVGetCandList(EMV_CANDLIST *patCandList, int *pnCandListNum)
{
	EMV_II_CANDLIST atCandAppList_new[MAX_APP_ITEMS];
	int i = 0, nRet = 0;

	if (patCandList == NULL || pnCandListNum == NULL)
	{
		return EMV_PARAM_ERR;
	}

	memset(atCandAppList_new, 0, sizeof(atCandAppList_new));
	*pnCandListNum = 0;
	nRet = EMV_II_GetCandList(atCandAppList_new, pnCandListNum);
	if (nRet)
	{
		return nRet;
	}
	for (i=0;i<(*pnCandListNum);i++)
	{
		ConvCandList_emv(atCandAppList_new[i], &patCandList[i]);
    }
	return EMV_OK;
}

int EMVSetCandList(EMV_CANDLIST *patCandList, int nCandListNum)
{
	EMV_II_CANDLIST atCandAppList_new[MAX_APP_ITEMS];
	int i = 0;

	if (patCandList == NULL)
	{
		return EMV_PARAM_ERR;
	}

	memset(atCandAppList_new, 0, sizeof(atCandAppList_new));
	for (i=0;i<nCandListNum;i++)
	{
		memcpy(atCandAppList_new[i].aucAID, patCandList[i].aucAID, patCandList[i].ucAidLen);
		atCandAppList_new[i].ucAidLen = patCandList[i].ucAidLen;
		atCandAppList_new[i].ucPriority = patCandList[i].ucPriority;
		strcpy((char*)atCandAppList_new[i].aucAppLabel, (char*)patCandList[i].aucAppLabel);
		strcpy((char*)atCandAppList_new[i].aucAppName, (char*)patCandList[i].aucAppName);
		strcpy((char*)atCandAppList_new[i].aucAppPreName, (char*)patCandList[i].aucAppPreName);
		strcpy((char*)atCandAppList_new[i].aucIssDiscrData, (char*)patCandList[i].aucIssDiscrData);
    }
	return EMV_II_SetCandList(atCandAppList_new, nCandListNum);
}

