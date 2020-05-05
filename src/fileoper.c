
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
/********************** Internal variables declaration *********************/
static uchar FILE_SYS_PARAM[]  = "SYSPARAM.DAT";		// system config parameters
static uchar FILE_SYS_CTRL[]   = "SYSCTRL.DAT";			// system control parameters
static uchar FILE_TRAN_LOG[]   = "TRANLOG.DAT";			// transaction log
static uchar FILE_EMV_STATUS[] = "EMVSTATUS.DAT";		// status of the last EMV transaction
static uchar FILE_ERR_LOG[]    = "EMVERRLOG.DAT";		// error log
static uchar FILE_PASSWORD[]   = "PASSWORD";			// password
static uchar FILE_POS_PARAMS[] = "POSPARAM.DATA"; // Pos Params;


/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// 初始化交易日志文件
// Init transaction log file.
// The management of transaction log is quite different with old style.
int InitTranLogFile(void)
{
	logTrace(__func__);
	int			fd, iWriteBytes, iCnt;
	TRAN_LOG	stTranLog;
#ifdef ENABLE_EMV
	EMV_ERR_LOG	stErrLog;
#endif

	fd = open((char *)FILE_TRAN_LOG, O_CREATE|O_RDWR);
	if( fd<0 )
	{
		PubTRACE0("InitTranLogFile");
		return ERR_FILEOPER;
	}

	memset(&stTranLog, 0, sizeof(stTranLog));
	for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
	{
		iWriteBytes = PubFWriteN(fd, &stTranLog, sizeof(stTranLog));
		if( iWriteBytes!=(int)sizeof(stTranLog) )
		{
			PubTRACE0("InitTranLogFile");
			close(fd);
			return ERR_FILEOPER;
		}
	}
	close(fd);

	// init error log file
	fd = open((char *)FILE_ERR_LOG, O_CREATE|O_RDWR);
	if( fd<0 )
	{
		PubTRACE0("InitTranLogFile");
		return ERR_FILEOPER;
	}

	memset(&stErrLog, 0, sizeof(stErrLog));
	for(iCnt=0; iCnt<MAX_ERR_LOG; iCnt++)
	{
		iWriteBytes = PubFWriteN(fd, &stErrLog, sizeof(stErrLog));
		if( iWriteBytes!=(int)sizeof(stErrLog) )
		{
			PubTRACE0("InitTranLogFile");
			close(fd);
			return ERR_FILEOPER;
		}
	}
	close(fd);

	return 0;
}

int LoadPosParams(void)
{
	int		iRet;

	iRet = PubFileRead(FILE_POS_PARAMS, 0L, &glPosParams, sizeof(PosParams));
	if (iRet != 0)
	{
		PubTRACE0("LoadPosParams()");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// 保存系统参数
// save system parameters
int SavePosParams(void)
{
	int		iRet;

	iRet = PubFileWrite(FILE_POS_PARAMS, 0L, &glPosParams, sizeof(PosParams));
	if (iRet != 0)
	{
		PubTRACE0("SavePosParams()");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// 读取系统参数
// load system parameters
int LoadSysParam(void)
{
	int		iRet;

	iRet = PubFileRead(FILE_SYS_PARAM, 0L, &glSysParam, sizeof(SYS_PARAM));
	if (iRet != 0)
	{
		PubTRACE0("LoadSysParam()");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}




// 保存系统参数
// save system parameters
int SaveSysParam(void)
{
	int		iRet;

	iRet = PubFileWrite(FILE_SYS_PARAM, 0L, &glSysParam, sizeof(SYS_PARAM));
	if( iRet!=0 )
	{
		PubTRACE0("SaveSysParam");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// 保存EDC参数
// save EDC parameters
int SaveEdcParam(void)
{
	int		iRet;

	iRet = PubFileWrite(FILE_SYS_PARAM,
						OFFSET(SYS_PARAM, stEdcInfo),
						&glSysParam.stEdcInfo,
						sizeof(glSysParam.stEdcInfo));
	if( iRet!=0 )
	{
		PubTRACE0("SaveSysParam");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// 保存系统密码
// save passwords
int SavePassword(void)
{
	int		iRet;

	iRet = PubFileWrite(FILE_SYS_PARAM,
						OFFSET(SYS_PARAM, sPassword),
						glSysParam.sPassword,
						sizeof(glSysParam.sPassword));
	if( iRet!=0 )
	{
		PubTRACE0("SavePassword");
		SysHalt();
		return ERR_FILEOPER;
	}

	return SyncPassword();
}

// 判断系统参数文件是否存在
// check if system files are existed
int ExistSysFiles(void)
{
	if ((fexist((char *)FILE_POS_PARAMS)<0) 
		||
		/*(fexist((char *)FILE_SYS_PARAM)<0) ||
		(fexist((char *)FILE_SYS_CTRL)<0) ||*/
		(fexist((char *)FILE_TRAN_LOG)<0)) 
		
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

// 判断系统参数文件大小
// verify if sizes of the system files are correct or not
int ValidSysFiles(void)
{
	if ((fexist((char *)FILE_POS_PARAMS)<0) ||
		(filesize((char *)FILE_POS_PARAMS) != sizeof(PosParams)))
	{
		return FALSE;
	}
	/*if ((fexist((char *)FILE_SYS_PARAM)<0) ||
		(filesize((char *)FILE_SYS_PARAM)!=sizeof(SYS_PARAM)) )
	{
		return FALSE;
	}
	if ((fexist((char *)FILE_SYS_CTRL)<0) ||
		(filesize((char *)FILE_SYS_CTRL)!=sizeof(SYS_CONTROL)) )
	{
		return FALSE;
	}*/
	if ((fexist((char *)FILE_TRAN_LOG)<0) ||
		(filesize((char *)FILE_TRAN_LOG)!=MAX_TRANLOG*sizeof(TRAN_LOG)) )
	{
		return FALSE;
	}
	return TRUE;
}

void RemoveSysFiles(void)
{
	remove((char *)FILE_POS_PARAMS);
	remove((char *)FILE_SYS_PARAM);
	remove((char *)FILE_SYS_CTRL);
	remove((char *)FILE_TRAN_LOG);
}

// 读取全部运行控制参数
// Load the whole "glSysCtrl"
int LoadSysCtrlAll(void)
{
	int		iRet;

	iRet = PubFileRead(FILE_SYS_CTRL, 0L, &glSysCtrl, sizeof(SYS_CONTROL));
	if( iRet!=0 )
	{
		PubTRACE0("LoadSysCtrlAll");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// 保存全部运行控制参数(谨慎使用,比较耗时)
// Save the whole "glSysCtrl" to file (very time-consuming)
int SaveSysCtrlAll(void)
{
	int		iRet;

	// about 60K bytes
	iRet = PubFileWrite(FILE_SYS_CTRL, 0L, &glSysCtrl, sizeof(SYS_CONTROL));
	if( iRet!=0 )
	{
		PubTRACE0("SaveSysCtrlAll");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// 保存基本运行控制参数(STAN/Invoice No/....)
// Save most basic system-control parameter. part of "glSysCtrl"
int SaveSysCtrlBase(void)
{
	int		iRet;

	// about 1.5K bytes
	iRet = PubFileWrite(FILE_SYS_CTRL, 0L, &glSysCtrl, LEN_SYSCTRL_BASE);
	if( iRet!=0 )
	{
		PubTRACE0("SaveSysCtrlBase");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// save basic information & reversal information
int SaveSysCtrlNormal(void)
{
	int		iRet;

	// about 20K bytes
	iRet = PubFileWrite(FILE_SYS_CTRL, 0L, &glSysCtrl, LEN_SYSCTRL_NORMAL);
	if( iRet!=0 )
	{
		PubTRACE0("SaveSysCtrlNormal");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// save reprint settle information
int SaveRePrnStlInfo(void)
{
	int		iRet;

	// about 40K bytes
	iRet = PubFileWrite(FILE_SYS_CTRL,
						OFFSET(SYS_CONTROL, stRePrnStlInfo),
						&glSysCtrl.stRePrnStlInfo,
						sizeof(REPRN_STL_INFO));
	if( iRet!=0 )
	{
		PubTRACE0("SaveStlInfo");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// 读取一个交易日志
// Load one transaction log from file
int LoadTranLog(void *pstLog, ushort uiIndex)
{
	memset(pstLog, 0, sizeof(TRAN_LOG));

	int		iRet;
	iRet = PubFileRead(FILE_TRAN_LOG, (long)(sizeof(TRAN_LOG)*uiIndex),
						pstLog, sizeof(TRAN_LOG));
	if( iRet!=0 )
	{
		PubTRACE0("LoadTranLog");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// 更新一个交易日志
// update one transaction log
int UpdateTranLog(const void *pstLog, ushort uiIndex)
{
	int		iRet;
	iRet = PubFileWrite(FILE_TRAN_LOG, (long)(sizeof(TRAN_LOG)*uiIndex),
						pstLog, sizeof(TRAN_LOG));
	if( iRet!=0 )
	{
		PubTRACE0("UpdateTranLog");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

int SaveTranLog(const void *pstLog)
{
	int		iRet, iCnt;
	int		fd;

	iCnt = glPosParams.tranRecordCount;

	if( iCnt>=MAX_TRANLOG )
	{
		PubTRACE0("SaveTranLog");
		return ERR_FILEOPER;
	}

	fd = open((char *)FILE_TRAN_LOG, O_RDWR | O_CREATE);
	if (fd<0)
	{
		logd(("file open error %d", iRet));
		PubTRACE0("RecoverTranLog");
		SysHalt();
		return ERR_FILEOPER;
	}

	iRet = seek(fd, (long)(iCnt * sizeof(TRAN_LOG)), SEEK_SET);
	if (iRet<0)
	{
		logd(("Seek error %d", iRet));
		close(fd);
		SysHalt();
		return ERR_FILEOPER;
	}

	iRet = PubFWriteN(fd, pstLog, sizeof(TRAN_LOG));
	close(fd);
	if (iRet != (int)sizeof(TRAN_LOG))
	{
		logd(("File write error %d", iRet));
		PubTRACE0("RecoverTranLog");
		SysHalt();
		return iRet;
	}

	glPosParams.tranRecordCount++;
	SavePosParams();
}

// 恢复日志记录的不一致性
// Finish the save steps.
int RecoverTranLog(void)
{
	int		fd, iRet;

	if( !glSysCtrl.stWriteInfo.bNeedSave )
	{
		return 0;
	}

	fd = open((char *)FILE_TRAN_LOG, O_RDWR|O_CREATE);
	if( fd<0 )
	{
		PubTRACE0("RecoverTranLog");
		SysHalt();
		return ERR_FILEOPER;
	}

	iRet = seek(fd, (long)(glSysCtrl.stWriteInfo.uiRecNo*sizeof(TRAN_LOG)), SEEK_SET);
	if( iRet<0 )
	{
		close(fd);
		SysHalt();
		return ERR_FILEOPER;
	}

	iRet = PubFWriteN(fd, &glSysCtrl.stWriteInfo.stTranLog, sizeof(TRAN_LOG));
	close(fd);
	if( iRet!=(int)sizeof(TRAN_LOG) )
	{
		PubTRACE0("RecoverTranLog");
		SysHalt();
		return iRet;
	}

	if( !(glSysCtrl.stWriteInfo.stTranLog.uiStatus & TS_NOSEND) )
	{
		
	}

	glSysCtrl.stWriteInfo.bNeedSave = FALSE;
	glSysCtrl.uiLastRecNo = glSysCtrl.stWriteInfo.uiRecNo;

	return SaveSysCtrlBase();
}


// 获取交易记录总数
// Retrieve total transaction record number.
ushort GetTranLogNum(uchar ucAcqKey)
{
	return glPosParams.tranRecordCount;
}

// 检查最后一笔交易是否与当前交易重复,并进行提示
// Check whether the last transaction is duplicated with current going-on one.
// Modified by Kim_LinHB 2014-6-8
uchar AllowDuplicateTran(void)
{
	TRAN_LOG	stTranLog;
	ushort		uiTranNum;

	uiTranNum = GetTranLogNum(ACQ_ALL);
	if( uiTranNum==0 || glSysCtrl.uiLastRecNo>=MAX_TRANLOG )
	{
		return TRUE;
	}
	if( glSysCtrl.sAcqKeyList[glSysCtrl.uiLastRecNo]==INV_ACQ_KEY )
	{
		return TRUE;
	}

	memset(&stTranLog, 0, sizeof(TRAN_LOG));
	LoadTranLog(&stTranLog, glSysCtrl.uiLastRecNo);
	if( stTranLog.ucTranType!=glProcInfo.stTranLog.ucTranType                       ||
		strncmp((char *)stTranLog.szPan, (char *)glProcInfo.stTranLog.szPan, 19)!=0 ||
		memcmp(stTranLog.szAmount,       glProcInfo.stTranLog.szAmount,      12)!=0 ||
		memcmp(stTranLog.szOtherAmount,    glProcInfo.stTranLog.szOtherAmount,   12)!=0
		)
	{
		return TRUE;
	}

	Gui_ClearScr();
	if(GUI_OK == Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("DUPLICATE? Y/N"), gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL))
	{	
		return 1;
	}
	return 0;
}


// 输入交易流水号以获取该交易数据
// Get invoice to retrieve the transaction record
// return 0 if successful
// Modified by Kim_LinHB 2014-6-8
int GetRecord(uint uiStatus, void *pstOutTranLog)
{
	int			iRet;
	ushort		uiCnt;
	ulong		ulInvoiceNo;
	TRAN_LOG	*pstLog;
	unsigned char szStatus[16 + 1];

	pstLog = (TRAN_LOG *)pstOutTranLog;
	while( 1 )
	{
		memset(szStatus, 0, sizeof(szStatus));
		iRet = InputInvoiceNo(&ulInvoiceNo);
		if( iRet!=0 )
		{
			return iRet;
		}

		for(uiCnt=0; uiCnt<MAX_TRANLOG; uiCnt++)
		{
			if( glSysCtrl.sAcqKeyList[uiCnt]==INV_ACQ_KEY )
			{
				continue;
			}

			memset(pstLog, 0, sizeof(TRAN_LOG));
			iRet = LoadTranLog(pstLog, uiCnt);
			if( iRet!=0 )
			{
				return iRet;
			}
			if( pstLog->ulInvoiceNo!=ulInvoiceNo )
			{
				continue;
			}
			if( ((pstLog->uiStatus) & 0x0000000F)<=uiStatus )
			{
				glProcInfo.uiRecNo = uiCnt;
				return 0;
			}
			else
			{
				GetStateText(pstLog->uiStatus, szStatus);
				break;
			}
		}
		if( uiCnt>=MAX_TRANLOG )
		{
			strcpy(szStatus, "INVALID TRACE");
		}
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szStatus, gl_stCenterAttr, GUI_BUTTON_NONE, 2, NULL);
	}

	return 0;
}

// Modified by lrz v1.01.0007 
void CalcTotal(uchar ucAcqKey)
{
	
}

#ifdef ENABLE_EMV
// save last emv status
int SaveEmvStatus(void)
{
	int		iRet;

	iRet = PubFileWrite(FILE_EMV_STATUS, 0L, &glEmvStatus, sizeof(EMV_STATUS));
	if( iRet!=0 )
	{
		PubTRACE0("SaveEmvStatus");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}
#endif

#ifdef ENABLE_EMV
// save last emv status
int LoadEmvStatus(void)
{
	int		iRet;

	iRet = PubFileRead(FILE_EMV_STATUS, 0L, &glEmvStatus, sizeof(EMV_STATUS));
	if( iRet!=0 )
	{
		PubTRACE0("LoadEmvStatus");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}
#endif

#ifdef ENABLE_EMV
// save EMV error log message
int SaveEmvErrLog(void)
{
	int			iRet, iLength;
	EMV_ERR_LOG	stErrLog;

	// collect message for log
	memset(&stErrLog, 0, sizeof(EMV_ERR_LOG));
	stErrLog.bValid = TRUE;
	stErrLog.ucAidLen = glProcInfo.stTranLog.ucAidLen;
	memcpy(stErrLog.sAID, glProcInfo.stTranLog.sAID, stErrLog.ucAidLen);
	sprintf((char *)stErrLog.szPAN, "%.19s", glProcInfo.stTranLog.szPan);
	stErrLog.ucPANSeqNo = glProcInfo.stTranLog.bPanSeqOK ? glProcInfo.stTranLog.ucPanSeqNo : 0xFF;
	sprintf((char *)stErrLog.szAmount, "%.12s", glSendPack.szTranAmt);
	sprintf((char *)stErrLog.szTipAmt, "%.12s", glSendPack.szExtAmount);
	sprintf((char *)stErrLog.szRspCode, "%.2s", glProcInfo.stTranLog.szRspCode);
	GetDateTime(stErrLog.szDateTime);
	sprintf((char *)stErrLog.szRRN, "%.12s",    glProcInfo.stTranLog.szRRN);
	sprintf((char *)stErrLog.szAuthCode, "%.6s", glProcInfo.stTranLog.szAuthCode);

	EMVGetTLVData(0x95, stErrLog.sTVR, &iLength);
	EMVGetTLVData(0x9B, stErrLog.sTSI, &iLength);

	stErrLog.uiReqICCDataLen = (ushort)PubChar2Long(glSendPack.sICCData, 2);
	memcpy(stErrLog.sReqICCData, &glSendPack.sICCData[2], stErrLog.uiReqICCDataLen);
	stErrLog.uiReqField56Len = (ushort)PubChar2Long(glSendPack.sICCData2, 2);
	memcpy(stErrLog.sReqField56, &glSendPack.sICCData2[2], stErrLog.uiReqField56Len);
	if( memcmp(glSendPack.szSTAN, glRecvPack.szSTAN, 6)==0 )
	{
		stErrLog.uiRspICCDataLen = (ushort)PubChar2Long(glRecvPack.sICCData, 2);
		memcpy(stErrLog.sRspICCData, &glRecvPack.sICCData[2], stErrLog.uiRspICCDataLen);
	}
	stErrLog.ulSTAN = glProcInfo.stTranLog.ulSTAN;

	PubASSERT( glSysCtrl.uiErrLogNo<MAX_ERR_LOG );
	if( glSysCtrl.uiErrLogNo>=MAX_ERR_LOG )
	{
		glSysCtrl.uiErrLogNo = 0;
	}
	iRet = PubFileWrite(FILE_ERR_LOG, (long)(glSysCtrl.uiErrLogNo * sizeof(EMV_ERR_LOG)),
						&stErrLog, sizeof(EMV_ERR_LOG));
	if( iRet!=0 )
	{
		PubTRACE0("SaveEmvErrLog");
		SysHalt();
		return ERR_FILEOPER;
	}

	glSysCtrl.uiErrLogNo++;
	if( glSysCtrl.uiErrLogNo>=MAX_ERR_LOG )
	{
		glSysCtrl.uiErrLogNo = 0;
	}
	SaveSysCtrlBase();

	return 0;
}
#endif

#ifdef ENABLE_EMV
// load one error log
int LoadErrLog(ushort uiRecNo, void *pOutErrLog)
{
	int		iRet;

	iRet = PubFileRead(FILE_ERR_LOG, (long)(uiRecNo * sizeof(EMV_ERR_LOG)),
						pOutErrLog, sizeof(EMV_ERR_LOG));
	if( iRet!=0 )
	{
		PubTRACE0("LoadEmvErrLog");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}
#endif

// 同步密码文件到manager和EPS
// Sync password to manager application. usually for HongKong
int SyncPassword(void)
{
	int		iRet;

	iRet = PubFileWrite(FILE_PASSWORD, 0L, glSysParam.sPassword, sizeof(glSysParam.sPassword));
	if( iRet!=0 )
	{
		PubTRACE0("SyncPassword");
		SysHalt();
		return ERR_FILEOPER;
	}

	return 0;
}

// for BEA fallback process
int LastRecordIsFallback(void)
{
	

	return FALSE;
}

int saveLogoBmpFile(uchar* bmp, int len) {
	return PubFileWrite(RECEIPT_LOGO_FILE, 0, bmp, len);
}

int readLogoBmpFile(uchar* bmp, int* len) {
	*len = PubFileRead(RECEIPT_LOGO_FILE, 0, bmp, *len);
	return *len;
}

// end of file
