#include <posapi.h>

#include "global.h"
//*******************************Global variable*****************************************************//
uchar gl_aucOutcomeParamSet[8]={0};
uchar gl_aucUserInterReqData[22]={0};
uchar gl_aucErrIndication[6]={0};

CLSS_DISC_DATA_MC gl_DiscretionayData;//the element flag in discretionary data exist or not
static App_SchemeId gl_stSchemeId;
static int gl_nAppTornLogNum = 0;//number of Tornlog
static CLSS_TORN_LOG_RECORD gl_atAppTornTransLog[5];//Torn Transaction Log
static uchar gl_ucAppType = KERNTYPE_DEF; 
uchar gl_ucTransPath=0;
static uchar gl_ucACType = 0;//ACTYPE
extern EMV_CAPK glCAPKeyList[100];
uchar gl_ucAdviceFlag = 0;	//added by kevinliu 2015/10/16

uchar gl_aucFinalAID[17]={0};
uchar gl_ucFinalAIDLen=0;
Clss_TransParam gl_tTransParam;

//*****************************************************************************************************//
extern uchar PubWaitKey(short iWaitTime);

//outcome
void vAppCreateOutcomeData_MC(int nRet)
{
	int nErrorCode=0;

	switch(nRet)
	{
	case CLSS_USE_CONTACT:
		gl_aucErrIndication[1]=0;
		gl_aucErrIndication[3]=0;
		gl_aucErrIndication[4]=0;

		gl_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S53.13

		gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//S53.14 for paypass 3.0.1 by zhoujie
		gl_aucUserInterReqData[1]=MI_IDLE;
		memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);
		//		nSendTransDataOutput_MC(CLSS_DATA_UIRD);//S53.14

		gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;//S53.15 for paypass 3.0.1 by zhoujie
		gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
		gl_DiscretionayData.ucErrIndicFlg = 1;
		//		nSendTransDataOutput_MC(CLSS_DATA_OCPS | CLSS_DATA_DISD);//S53.15
		break;
	case ICC_BLOCK:
		gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//s52.8 for paypass 3.0.1 by zhoujie
		gl_aucUserInterReqData[1]=MI_IDLE;
		memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

		gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;//s52.9 for paypass 3.0.1 by zhoujie
		gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
		gl_DiscretionayData.ucErrIndicFlg = 1;
		//		nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S52.8 S52.9
		break;
	case EMV_NO_APP_PPSE_ERR:
		Clss_GetErrorCode_Entry(&nErrorCode);
		if(nErrorCode == EMV_DATA_ERR)//S52.11
		{
			gl_aucErrIndication[1]=L2_PARSING_ERROR;//S52.11 for paypass 3.0.1 by zhoujie

			gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//S52.18 for paypass 3.0.1 by zhoujie
			gl_aucUserInterReqData[1]=MI_IDLE;
			memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;//S52.19 for paypass 3.0.1 by zhoujie
			gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			gl_DiscretionayData.ucErrIndicFlg = 1;
			//			nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S52.18 S52.19
		}
		else if (nErrorCode == EMV_RSP_ERR || nErrorCode == EMV_APP_BLOCK)
		{
			gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//s52.8 for paypass 3.0.1 by zhoujie
			gl_aucUserInterReqData[1]=MI_IDLE;
			memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

			gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;//s52.9 for paypass 3.0.1 by zhoujie
			gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
			gl_DiscretionayData.ucErrIndicFlg = 1;
			//			nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S52.8 S52.9
		}
		break;
	case EMV_NO_APP:
		gl_aucErrIndication[1] = L2_EMPTY_CANDIDATE_LIST;//S52.14

		gl_aucUserInterReqData[0]=MI_ERROR_OTHER_CARD;//S52.18 for paypass 3.0.1 by zhoujie
		gl_aucUserInterReqData[1]=MI_IDLE;
		memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

		gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;//S52.19 for paypass 3.0.1 by zhoujie
		gl_aucErrIndication[5]=MI_ERROR_OTHER_CARD;
		gl_DiscretionayData.ucErrIndicFlg = 1;
		//		nSendTransDataOutput_MC(T_UIRD | T_OCPS | T_DISD);//S52.18 S52.19
		break;
	default:
		Clss_GetErrorCode_Entry(&nErrorCode);
		if(nErrorCode == EMV_NO_DATA)
		{
			gl_aucErrIndication[1] = L2_CARD_DATA_MISSING;//S1.7
		}
		else //EMV_DATA_ERR and CLSS_RESELECT_APP
		{
			gl_aucErrIndication[1] = L2_PARSING_ERROR;//S1.7
		}
		gl_aucOutcomeParamSet[0] = OC_SELECT_NEXT;
		gl_aucOutcomeParamSet[1] = OC_C;
		vAppInitDiscData_MC();
		gl_DiscretionayData.ucErrIndicFlg = 1;
		//		nSendTransDataOutput_MC(CLSS_DATA_OCPS | CLSS_DATA_DISD);//S1.8
		break;
	}
}

void vAppInitDiscData_MC(void)
{
	memset(&gl_DiscretionayData,0,sizeof(gl_DiscretionayData));
}

void vAppSetAppType(uchar ucType)
{
	gl_ucAppType = ucType;
}

uchar ucAppGetAppType(void)
{
	return gl_ucAppType;
}

int nAppSetCurAppType(uchar ucKernType)
{
	int nRet=0;
	if ((ucKernType == KERNTYPE_VIS && gl_stSchemeId.ucSupportVISA == 1))
	{
		vAppSetAppType(KERNTYPE_VIS);
		Clss_CoreInit_Wave();
	}
	else if (ucKernType == KERNTYPE_MC && gl_stSchemeId.ucSupportMC == 1)
	{
		vAppSetAppType(KERNTYPE_MC);
		Clss_CoreInit_MC(1);
		Clss_SetParam_MC("\x04", 1);// set the timer number for kernel [1/21/2014 ZhouJie]
		nAppCleanTornLog_MC();
	}
	else if (ucKernType == KERNTYPE_AE && gl_stSchemeId.ucSupportAE == 1)
	{
		vAppSetAppType(KERNTYPE_AE);
		Clss_CoreInit_AE();
	}
	else if (ucKernType == KERNTYPE_ZIP && gl_stSchemeId.ucSupportDPAS == 1)
	{
		vAppSetAppType(KERNTYPE_ZIP);
		Clss_CoreInit_DPAS();
	}
	else if (ucKernType == KERNTYPE_JCB && gl_stSchemeId.ucSupportJCB == 1)
	{
		vAppSetAppType(KERNTYPE_JCB);
		Clss_CoreInit_JCB();
	}
	else
	{
		return CLSS_TERMINATE;
	}
	return EMV_OK;
}


int nAppCleanTornLog_MC(void)
{
	uchar aucBuff[8];
	int nUpdatedFlg = 0, nRet=0;
	GetTime(aucBuff);//  [11/18/2011 zhoujie]
	if (gl_nAppTornLogNum == 0)
	{
		return 0;
	}
	Clss_SetTornLog_MC_MChip(gl_atAppTornTransLog, gl_nAppTornLogNum);
	nRet = Clss_CleanTornLog_MC_MChip(aucBuff, 6, 0);
	if (nRet)
	{
		return 0;
	}
	memset(gl_atAppTornTransLog, 0, sizeof(gl_atAppTornTransLog));
	gl_nAppTornLogNum = 0;
	nUpdatedFlg = 0;
	Clss_GetTornLog_MC_MChip(gl_atAppTornTransLog, &gl_nAppTornLogNum, &nUpdatedFlg);
	return 0;
}


int nAppGetTornLog_MC(CLSS_TORN_LOG_RECORD *ptTornLog, int *pnTornNum)
{
	if (ptTornLog == NULL || pnTornNum== NULL)
	{
		return EMV_PARAM_ERR;
	}
	memcpy(ptTornLog, gl_atAppTornTransLog, gl_nAppTornLogNum*sizeof(CLSS_TORN_LOG_RECORD));
	*pnTornNum = gl_nAppTornLogNum;
	return 0;
}

int nAppSaveTornLog_MC(CLSS_TORN_LOG_RECORD *ptTornLog, int nTornNum)//save Torn Log in file
{
	int nFid=0, nRet=0;
	if (ptTornLog == NULL)
	{
		return EMV_PARAM_ERR;
	}
	memcpy(gl_atAppTornTransLog, ptTornLog, nTornNum*sizeof(CLSS_TORN_LOG_RECORD));
	gl_nAppTornLogNum = nTornNum;
	nFid = open("KernalFileMCTornLog", O_CREATE|O_RDWR);
	if (nFid<0)
	{
		return EMV_FILE_ERR;
	}
	nRet = write(nFid, (uchar *)&nTornNum, 1);
	if (nRet<0)
	{
		return EMV_FILE_ERR;
	}
	nRet = write(nFid, (uchar *)ptTornLog, nTornNum*sizeof(CLSS_TORN_LOG_RECORD));
	close(nFid);
	if (nRet<0)
	{
		return EMV_FILE_ERR;
	}
	return EMV_OK;
}

int nAppLoadTornLog_MC(void)//read Torn Log from file
{
	int nFid=0, nRet=0;
	nFid = open("KernalFileMCTornLog", O_RDWR);
	if (nFid<0)
	{
		return EMV_FILE_ERR;
	}
	memset(gl_atAppTornTransLog, 0, sizeof(gl_atAppTornTransLog));
	gl_nAppTornLogNum = 0;
	nRet = read(nFid, (uchar *)&gl_nAppTornLogNum, 1);
	if (nRet != 1)
	{
		return EMV_FILE_ERR;
	}
	if (gl_nAppTornLogNum == 0)
	{
		return NO_TRANS_LOG;
	}
	nRet = read(nFid, (uchar *)gl_atAppTornTransLog, gl_nAppTornLogNum*sizeof(CLSS_TORN_LOG_RECORD));
	close(nFid);
	if (nRet<0)
	{
		return EMV_FILE_ERR;
	}
	return EMV_OK;	
}

void vAppSetTransPath(uchar ucTransPath)
{
	gl_ucTransPath = ucTransPath;
}

uchar ucAppGetTransPath(void)
{
	return gl_ucTransPath;
}

// add for paypass 3.0 [12/30/2014 jiangjy]
void vSetFinalSelectAID(uchar *paucAID, uchar ucAIDLen)
{
	if (ucAIDLen && paucAID != NULL)
	{
		memcpy(gl_aucFinalAID, paucAID, ucAIDLen);
	}
	gl_ucFinalAIDLen = ucAIDLen;
}

void SetClssTxnParam(Clss_TransParam *pParam)
{
	memcpy(&gl_tTransParam, pParam, sizeof(Clss_TransParam));
}

void GetClssTxnParam(Clss_TransParam *pParam)
{
	memcpy(pParam, &gl_tTransParam, sizeof(Clss_TransParam));
}

void ClssBaseParameterSet_WAVE(void)
{
	Clss_ReaderParam ClssParam;
	Clss_VisaAidParam tVisaAidParam;

	Clss_GetReaderParam_Wave(&ClssParam);
	memcpy(ClssParam.aucTmCap, EMV_CAPABILITY,3);
	memcpy(ClssParam.aucTmCapAd,"\xE0\x00\xF0\xA0\x01",5);
	ClssParam.ucTmType = 0x22;

	memcpy(ClssParam.aucTmCntrCode, glEmvParam.CountryCode, 2);
	memcpy(ClssParam.aucTmRefCurCode, glEmvParam.ReferCurrCode, 2);
	memcpy(ClssParam.aucTmTransCur, glEmvParam.TransCurrCode, 2);
	Clss_SetReaderParam_Wave(&ClssParam);
// move [1/8/2015 jiangjy]
// 	memset(&ClssVisaAidParam,0,sizeof(Clss_VisaAidParam));
// 	ClssVisaAidParam.ulTermFLmt = 0;
// 	Clss_SetVisaAidParam_Wave(&ClssVisaAidParam);

	memset(&tVisaAidParam,0,sizeof(Clss_VisaAidParam));
	tVisaAidParam.ucCvmReqNum = 2;
	tVisaAidParam.aucCvmReq[0] = RD_CVM_REQ_SIG;
	tVisaAidParam.aucCvmReq[1] = RD_CVM_REQ_ONLINE_PIN;

	tVisaAidParam.ucDomesticOnly = 0x00; // 01(default):only supports domestic cl transaction
	tVisaAidParam.ucEnDDAVerNo = 0;// fDDA ver 00 & 01 are all supported
	tVisaAidParam.ulTermFLmt = 0;
	Clss_SetVisaAidParam_Wave(&tVisaAidParam);

}

void SetAEAidParam_AE(void)
{
	CLSS_AEAIDPARAM stAIDParam;

	memset(&stAIDParam, 0, sizeof(CLSS_AEAIDPARAM));
	memcpy(stAIDParam.AcquierId, "\x00\x00\x00\x12\x34\x56", 6);
	stAIDParam.FloorLimit = 500;
	stAIDParam.FloorLimitCheck = 1;
	memcpy(stAIDParam.TACDefault, "\xFE\x50\xBC\xA0\x00", 5);
	memcpy(stAIDParam.TACDenial, "\x00\x00\x00\x00\x00", 5);
	memcpy(stAIDParam.TACOnline, "\xFE\x50\xBC\xF8\x00", 5);
	memcpy(stAIDParam.dDOL, "\x9F\x37\x04", 3);
	memcpy(stAIDParam.tDOL, "\x9F\x02\x06\x5F\x2A\x02\x9A\x03\x9C\x01\x95\x05\x9F\x37\x04", 15);
	memcpy(stAIDParam.Version, "\x00\x02", 2);
	//memcpy(stAIDParam.Version, "\x00\x01", 2);
	stAIDParam.ucAETermCap = 0xC8;
	Clss_SetAEAidParam_AE(&stAIDParam);
}

//added by kevinliu 2015/11/24
//set kernel configuration parameters
void ClssBaseParameterSet_AE(void)
{
	int iRet = 0;
	Clss_ReaderParam_AE stParam;
	Clss_AddReaderParam_AE stAddParam;

	Clss_GetReaderParam_AE(&stParam);
	//the range of Magstripe UN(Unpredictable Number)
	memcpy(stParam.aucUNRange, "\x00\x60", 2);
	//Contactless card reader parameters
	memcpy(stParam.stReaderParam.aucTmCap, EMV_CAPABILITY, 3);
	memcpy(stParam.stReaderParam.aucTmCapAd, "\xE0\x00\xF0\xA0\x01", 5);
	stParam.stReaderParam.ucTmType = 0x22;
	memcpy(stParam.stReaderParam.aucTmCntrCode, glEmvParam.CountryCode, 2);
	memcpy(stParam.stReaderParam.aucTmRefCurCode, glEmvParam.ReferCurrCode, 2);
	memcpy(stParam.stReaderParam.aucTmTransCur, glEmvParam.TransCurrCode, 2);
	//the terminal whether to support the optimization mode transaction flag
	stParam.ucTmSupOptTrans = 1;
	iRet = Clss_SetReaderParam_AE(&stParam);

	Clss_GetAddReaderParam_AE(&stAddParam);
	//Terminal Transaction Capabilities - EMV Tag ??9F6E?бе
	// resolve case 25
	memcpy(stAddParam.aucTmTransCapa,"xD8\xB0\x40\x00",4);
	//memcpy(stAddParam.aucTmTransCapa,"\x58\x70\x40\x00",4);
	//1: support Delayed Authorization; 0:not support Delayed Authorization
	stAddParam.ucDelayAuthFlag = 0;
	iRet = Clss_SetAddReaderParam_AE(&stAddParam);

	SetAEAidParam_AE();
}

//TODO setting
void ClssTermParamSet_JCB(void)
{
	int iRet = 0;
	Clss_TransParam tTransParam;

	memset(&tTransParam, 0, sizeof(Clss_TransParam));
	GetClssTxnParam(&tTransParam);
	// static Param
	nSetDETData((uchar *)"\xFF\x81\x30", 3, (uchar *)"\x7B\x00", 2);
//	nSetDETData((uchar *)"\xFF\x81\x31", 3, (uchar *)"\x60", 1);
//	nSetDETData((uchar *)"\xFF\x81\x32", 3, (uchar *)"\x7B\x00", 2);
	
	nSetDETData("\xDF\x81\x20", 3, "\x04\x00\x00\x00\x00", 5);
	nSetDETData("\xDF\x81\x21", 3, "\x04\x40\x00\x00\x00", 5);
	nSetDETData("\xDF\x81\x22", 3, "\xF8\x50\xAC\xF8\x00", 5);//TAC online
	
	//[1/9/2015 jiangjy] limit  set for AID
	nSetDETData("\xDF\x81\x23", 3, "\x00\x00\x00\x00\x00\x00", 6);//floor limit
	nSetDETData("\xDF\x81\x24", 3, "\x00\x00\x00\x10\x00\x00", 6);
	nSetDETData("\xDF\x81\x26", 3, "\x00\x00\x00\x00\x50\x00", 6);//cvm limit 

	nSetDETData("\x9F\x53", 2, "\xF2\x80\x00", 3);//Terminal Interchange Profile
	nSetDETData("\x9F\x33", 2, "\xE0\xF8\xC8", 3);//Teminal Capabilities
	nSetDETData("\x9F\x01", 2, "\x00\x00\x00\x12\x34\x56", 6); //Acquirer Identifier
	nSetDETData("\x9F\x15", 2, "\x00\x00", 2);
	nSetDETData("\x9F\x4E", 2, "\x00", 1);
	nSetDETData("\x9F\x1A", 2, glEmvParam.CountryCode, 2); //Terminal Country Code

	nSetDETData("\x9F\x35", 2, "\x22", 1);

	nSetDETData((uchar *)"\x5F\x2A", 2, glEmvParam.TransCurrCode, 2);
	nSetDETData((uchar *)"\x5F\x36", 2, "\x02", 1);
}

void ClssTxnParamSet_JCB(void)
{
	int iRet = 0;
	uchar aucBuff[50] = {0},aucAmount[6] = {0};
	Clss_TransParam tTransParam;

	memset(&tTransParam, 0, sizeof(Clss_TransParam));
	GetClssTxnParam(&tTransParam);

	//dynamic Param
	memset(aucAmount, 0, sizeof(aucAmount));
	memset(aucBuff, 0, sizeof(aucBuff));

	sprintf((char *)aucBuff, "%012ld", tTransParam.ulAmntAuth);

	vTwoOne_app(aucBuff, 12, aucAmount);
	iRet = nSetDETData((uchar *)"\x9F\x02", 2, aucAmount, 6);
	memset(aucAmount, 0, sizeof(aucAmount));
	memset(aucBuff, 0, sizeof(aucBuff));

	sprintf((char *)aucBuff, "%012ld", tTransParam.ulAmntOther);

	vTwoOne_app(aucBuff, 12, aucAmount);
	iRet = nSetDETData((uchar *)"\x9F\x03", 2, aucAmount, 6);
	iRet = nSetDETData((uchar *)"\x9F\x21", 2, tTransParam.aucTransTime, 3);
	iRet = nSetDETData((uchar *)"\x9A", 1, tTransParam.aucTransDate, 3);
	iRet = nSetDETData((uchar *)"\x9C", 1, &tTransParam.ucTransType, 1);
}

void ClssTermParamSet_DPAS(void)
{
	int iRet = 0;
	Clss_TransParam tTransParam;

	memset(&tTransParam, 0, sizeof(Clss_TransParam));
	GetClssTxnParam(&tTransParam);
	// static Param
	nSetDETData((uchar *)"\xDF\x81\x20", 3, (uchar *)"\x04\x00\x00\x00\x00", 5);

	nSetDETData("\xDF\x81\x21", 3, "\x04\x40\x00\x80\x00", 5);

	nSetDETData("\xDF\x81\x22", 3, "\xF8\x50\xAC\xF8\x00", 5);//TAC online
	//[1/9/2015 jiangjy] limit  set for AID
	nSetDETData("\xDF\x81\x23", 3, "\x00\x00\x00\x00\x00\x00", 6);//floor limit
	nSetDETData("\xDF\x81\x24", 3, "\x0\x00\x00\x10\x00\x00", 6);
	nSetDETData("\xDF\x81\x26", 3, "\x00\x00\x00\x00\x50\x00", 6);//cvm limit

	nSetDETData("\x9F\x33", 2, EMV_CAPABILITY,3);//Teminal Capabilities
	nSetDETData("\x9F\x01", 2, "\x00\x00\x00\x12\x34\x56", 6); //Acquirer Identifier
	nSetDETData("\x9F\x15", 2, "\x00\x00", 2);
	nSetDETData("\x9F\x4E", 2, "\x00", 1);
	nSetDETData("\x9F\x1A", 2, glEmvParam.CountryCode, 2); //Terminal Country Code

	nSetDETData("\x9F\x35", 2, "\x22", 1);

	nSetDETData((uchar *)"\x5F\x2A", 2, glEmvParam.TransCurrCode, 2);
	nSetDETData((uchar *)"\x5F\x36", 2, "\x02", 1);
}

void ClssTxnParamSet_DPas(void)
{
	int iRet = 0;
	uchar aucBuff[50] = {0},aucAmount[6] = {0};
	Clss_TransParam tTransParam;

	memset(&tTransParam, 0, sizeof(Clss_TransParam));
	GetClssTxnParam(&tTransParam);

	//dynamic Param
	memset(aucAmount, 0, sizeof(aucAmount));
	memset(aucBuff, 0, sizeof(aucBuff));

	sprintf((char *)aucBuff, "%012ld", tTransParam.ulAmntAuth);

	vTwoOne_app(aucBuff, 12, aucAmount);
	iRet = nSetDETData((uchar *)"\x9F\x02", 2, aucAmount, 6);
	memset(aucAmount, 0, sizeof(aucAmount));
	memset(aucBuff, 0, sizeof(aucBuff));

	sprintf((char *)aucBuff, "%012ld", tTransParam.ulAmntOther);

	vTwoOne_app(aucBuff, 12, aucAmount);
	iRet = nSetDETData((uchar *)"\x9F\x03", 2, aucAmount, 6);
	iRet = nSetDETData((uchar *)"\x9F\x21", 2, tTransParam.aucTransTime, 3);
	iRet = nSetDETData((uchar *)"\x9A", 1, tTransParam.aucTransDate, 3);
	iRet = nSetDETData((uchar *)"\x9C", 1, &tTransParam.ucTransType, 1);
}


void ClssTermParamSet_MC(void)//for paypass
{
	uchar aucBuff[50], aucAmount[6];
	Clss_TransParam tTransParam;

	memset(&tTransParam, 0, sizeof(Clss_TransParam));
	GetClssTxnParam(&tTransParam);

	Clss_SetTLVDataList_MC("\x5F\x57\x00", 3);//Account type no value
	Clss_SetTLVDataList_MC("\x9F\x01\x00", 3);//Acquire id no value
	Clss_SetTLVDataList_MC("\x9F\x1E\x08\x11\x22\x33\x44\x55\x66\x77\x88", 11);//Interface Device Serial Number with any value
	Clss_SetTLVDataList_MC("\x9F\x15\x02\x00\x01", 5);//Merchant Category Code with any value
	Clss_SetTLVDataList_MC("\x9F\x16\x00", 3);//Merchant Identifier no value
	Clss_SetTLVDataList_MC("\x9F\x4E\x00", 3);//Merchant Name and Location no value
	Clss_SetTLVDataList_MC("\x9F\x33\x00", 3);//terminal capability no value
	Clss_SetTLVDataList_MC("\x9F\x1C\x00", 3);//terminal id no value

	Clss_SetTagPresent_MC("\xDF\x81\x04", 0);//Balance Before GAC
	Clss_SetTagPresent_MC("\xDF\x81\x05", 0);//Balance After GAC
	Clss_SetTagPresent_MC("\xDF\x81\x2D", 0);//Message Hold Time
	Clss_SetTLVDataList_MC("\x9F\x7E\x00", 3);//Mobile Support Indicator

	Clss_SetTLVDataList_MC("\xDF\x81\x08\x00", 4);//DS AC Type
	Clss_SetTLVDataList_MC("\xDF\x60\x00", 3);//DS Input (Card)
	Clss_SetTLVDataList_MC("\xDF\x81\x09\x00", 4);//DS Input (Term)
	Clss_SetTLVDataList_MC("\xDF\x62\x00", 3);//DS ODS Info
	Clss_SetTLVDataList_MC("\xDF\x81\x0A\x00", 4);//DS ODS Info For Reader
	Clss_SetTLVDataList_MC("\xDF\x63\x00", 3);//DS ODS Term

	Clss_SetTagPresent_MC("\xDF\x81\x10", 0);//Proceed To First Write Flag Tag not present
	Clss_SetTagPresent_MC("\xDF\x81\x12", 0);//Tags To Read Tag not present
	Clss_SetTagPresent_MC("\xFF\x81\x02", 0);//Tags To Write Before Gen AC
	Clss_SetTagPresent_MC("\xFF\x81\x03", 0);//Tags To Write After Gen AC
	Clss_SetTagPresent_MC("\xDF\x81\x27", 0);//Time Out Value Tag not present

											 //modified by kevin liu 20160628
	//nSetDETData("\x9F\x5C", 2, "\x7A\x45\x12\x3E\xE5\x9C\x7F\x40", 8);//DS Requested Operator ID
	nSetDETData("\x9F\x5C", 2, "\x00", 1); // Turn off IDS
	Clss_SetTagPresent_MC("\xDF\x81\x0D", 0);
	Clss_SetTagPresent_MC("\x9F\x70", 0);
	Clss_SetTagPresent_MC("\x9F\x75", 0);

	nSetDETData("\x9F\x09", 2, "\x00\x02", 2);
	nSetDETData("\x9F\x40", 2, "\x00\x00\x00\x00\x00", 5);
	nSetDETData("\xDF\x81\x17", 3, "\xE0", 1);//card data input capability
	nSetDETData("\xDF\x81\x18", 3, "\x60", 1);//ONLINE PIN :40 /SIG:20
	nSetDETData("\xDF\x81\x19", 3, "\x08", 1);//NO CVM
	nSetDETData("\xDF\x81\x1F", 3, "\xC8", 1);//08:CDA      //40:DDA 
	nSetDETData("\xDF\x81\x1A", 3, "\x9F\x6A\x04", 3);//Default UDOL
	nSetDETData("\x9F\x6D", 2, "\x00\x01", 2);
	nSetDETData("\xDF\x81\x1E", 3, "\x20", 1); //Modified by Kevin Liu 20160801
	nSetDETData("\xDF\x81\x2C", 3, "\x00", 1);

	memset(aucAmount, 0, sizeof(aucAmount));
	memset(aucBuff, 0, sizeof(aucBuff));

	sprintf((char *)aucBuff, "%012ld", tTransParam.ulAmntAuth);

	vTwoOne_app(aucBuff, 12, aucAmount);
	nSetDETData("\x9F\x02", 2, aucAmount, 6);

	memset(aucAmount, 0, sizeof(aucAmount));
	memset(aucBuff, 0, sizeof(aucBuff));

	sprintf((char *)aucBuff, "%012ld", tTransParam.ulAmntOther);

	vTwoOne_app(aucBuff, 12, aucAmount);
	nSetDETData("\x9F\x03", 2, aucAmount, 6);

	nSetDETData("\x9C", 1, &tTransParam.ucTransType, 1);
	nSetDETData("\x9A", 1, tTransParam.aucTransDate, 3);
	nSetDETData("\x9F\x21", 2, tTransParam.aucTransTime, 3);

	if (!memcmp(gl_aucFinalAID, "\xA0\x00\x00\x00\x04\x10\x10", 7))
	{
		nSetDETData("\xDF\x81\x20", 3, "\x04\x00\x00\x00\x00", 5);
		nSetDETData("\xDF\x81\x21", 3, "\x04\x00\x00\x00\x00", 5);
	}
	else if (memcmp(gl_aucFinalAID, "\xA0\x00\x00\x00\x04\x30\x60", 7) == 0)
	{
		nSetDETData("\xDF\x81\x20", 3, "\xF4\x50\x04\x80\x0C", 5);
		nSetDETData("\xDF\x81\x21", 3, "\x00\x00\x80\x00\x00", 5);
	}
	nSetDETData("\xDF\x81\x22", 3, "\xF8\x50\xAC\xF8\x00", 5);//TAC online

															  //[1/9/2015 jiangjy] limit  set for AID
	nSetDETData("\xDF\x81\x23", 3, "\x00\x00\x00\x00\x00\x00", 6);//floor limit
	nSetDETData("\xDF\x81\x24", 3, "\x00\x00\x00\x20\x00\x00", 6);
	nSetDETData("\xDF\x81\x25", 3, "\x00\x00\x00\x20\x00\x00", 6);
	nSetDETData("\xDF\x81\x26", 3, "\x00\x00\x00\x10\x00\x00", 6);//cvm limit

	nSetDETData("\x9F\x35", 2, "\x22", 1);

	Clss_SetTagPresent_MC("\xDF\x81\x30", 0);
	nSetDETData("\xDF\x81\x1C", 3, "\x00\x00", 2);
	nSetDETData("\xDF\x81\x1D", 3, "\x00", 1);
	nSetDETData("\xDF\x81\x0C", 3, "\x02", 1);
	Clss_SetTagPresent_MC("\xDF\x81\x2D", 0);
	nSetDETData("\x9F\x1A", 2, glEmvParam.CountryCode, 2);
	nSetDETData("\x5F\x2A", 2, glEmvParam.TransCurrCode, 2);
	if (gl_ucFinalAIDLen == 0)
	{
		return;
	}
	if (!memcmp(gl_aucFinalAID, "\xA0\x00\x00\x00\x04\x10\x10", 7))// MCHIP [1/4/2015 jiangjy]
	{
		logd(("MCHIP"));
		//MCD
		nSetDETData("\xDF\x81\x1B", 3, "\x3F", 1);
		//Tag 9F1D set based on definition
		nSetDETData("\x9F\x1D", 2, (uchar *)"\x6C\xF8\x80\x00\x00\x00\x00\x00", 8);
	}
	else if (memcmp(gl_aucFinalAID, "\xA0\x00\x00\x00\x04\x30\x60", 7) == 0)
	{
		//Mestro  NO MAG-STRIPE 
		nSetDETData("\xDF\x81\x1B", 3, "\xBF", 1);  // Maestro magstripe decline fix
													//Tag 9F1D
		nSetDETData("\x9F\x1D", 2, (uchar *)"\x4C\xF8\x80\x00\x00\x00\x00\x00", 8);
	}
}


void vTwoOne_app(unsigned char *in, unsigned short in_len, unsigned char *out)
{
	unsigned char tmp;
	unsigned short i;

	for (i = 0; i < in_len; i += 2) {
		tmp = in[i];
		if (tmp > '9')
			tmp = toupper(tmp) - ('A' - 0x0A);
		else
			tmp &= 0x0f;
		tmp <<= 4;
		out[i / 2] = tmp;

		tmp = in[i + 1];
		if (tmp > '9')
			tmp = toupper(tmp) - ('A' - 0x0A);
		else
			tmp &= 0x0f;
		out[i / 2] += tmp;
	}
}

void vOneTwo_app(unsigned char *One,unsigned short len,unsigned char *Two)
{
	unsigned char  i;
	static unsigned char TAB[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	for (i = 0; i < len; i++) {
		Two[i * 2] = TAB[One[i] >> 4];
		Two[i * 2 + 1] = TAB[One[i] & 0x0f];
	}
}

void vInitPaymentData(void)//KS.2 paypass 3.0.1 by zhoujie
{
	//Clss_TermConfig_MC tMCTermConfig;
	// 
	// 	memset(&tMCTermConfig, 0, sizeof(tMCTermConfig));
	// 	Clss_GetTermConfig_MC(&tMCTermConfig);
	// 	tMCTermConfig.ucMobileSupFlg = CLSS_TAG_EXIST_WITHVAL;
	// 	tMCTermConfig.ucMobileSup = 0x01;
	// 	Clss_SetTermConfig_MC(&tMCTermConfig);
	Clss_SetTLVDataList_MC("\x9F\x7E\x01\x01", 4);//Mobile Support Indicator

	memset(gl_aucOutcomeParamSet, 0, sizeof(gl_aucOutcomeParamSet));
	gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;//for s51.13
	gl_aucOutcomeParamSet[1] = OC_NA;//s51.3
	gl_aucOutcomeParamSet[2] = OC_NA;
	gl_aucOutcomeParamSet[3] = OC_NA;
	gl_aucOutcomeParamSet[4] |= 0x10;
	gl_aucOutcomeParamSet[5] = OC_NA;
	gl_aucOutcomeParamSet[6] = 0xFF;

	memset(gl_aucUserInterReqData, 0, sizeof(gl_aucUserInterReqData));
	gl_aucUserInterReqData[0] = MI_NA;
	gl_aucUserInterReqData[1] = MI_NA;
	memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

	memset(gl_aucErrIndication, 0, sizeof(gl_aucErrIndication));
	gl_aucErrIndication[5] = MI_NA;

	vInitDiscData();
	//	gl_ucTornLogRecordExist = 0;
}

void vInitDiscData(void)//Initialize Discretionary Data by zhoujie
{
	memset(&gl_DiscretionayData,0,sizeof(gl_DiscretionayData));
}

//add for check whether this should be a MSR trans
uchar ucDetOtherCancelCmd(void)
{
	if (IccDetect(0) == 0)
	{
		PiccClose();
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD INSERTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30,NULL);
		disp_clss_err(CLSS_USE_CONTACT);
		kbflush();
		DelayMs(2000);
		return CLSS_USE_CONTACT;
	}			
	else if (MagSwiped() == 0)
	{
		PiccClose();
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD SWIPED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30,NULL);
		disp_clss_err(CLSS_USE_CONTACT);
		kbflush();
		DelayMs(2000);
		return CLSS_USE_CONTACT;
	}

	return 0;		
}

int disp_clss_err(int err)
{
    SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);

	Gui_ClearScr();
	switch(err)
	{
	case EMV_NO_APP:
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("NO SUPPORT APP"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30,NULL);
		break;
	case CLSS_USE_CONTACT:
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS USE CONTACT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30,NULL);
		break;
	case ICC_BLOCK:
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD LOCKED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30,NULL);
		break;
	case EMV_APP_BLOCK:
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("APP LOCKED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30,NULL);
		break;
	case CLSS_PARAM_ERR:
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PARAM ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30,NULL);
		break;
	case ICC_CMD_ERR:
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("ICC COMMAND ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30,NULL);
		break;
	case EMV_DATA_ERR:
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("EMV DATA ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 30,NULL);
		break;
	default:
		break;
	}
	SetClssLightStatus(CLSSLIGHTSTATUS_NOTREADY);
	return err;
}



int nAppTransProc_VISA(uchar ucTransPath, uchar ucACType)
{
	int nRet = 0;
	int nTempLen = 0;
	uchar ucPkIndex = 0;
	uchar ucDDAFailFlg = 0;
	Clss_TransParam tTransParm;
	GetClssTxnParam(&tTransParm);

	switch (ucTransPath)
	{
	case CLSS_VISA_MSD:
	case CLSS_VISA_MSD_CVN17:
	case CLSS_VISA_MSD_LEGACY:
		break;
	case CLSS_VISA_QVSDC:

		appRemovePicc();
		nRet = Clss_ProcRestric_Wave();

		if (nRet)
		{
			return nRet;
		}

		if (ucACType == AC_TC && tTransParm.ucTransType != 0x20)//REFUND
		{
			Clss_DelAllRevocList_Wave();
			Clss_DelAllCAPK_Wave();

			if ( Clss_GetTLVData_Wave(0x8F, &ucPkIndex,  &nTempLen) == 0)  
			{
				//if (AppConv_GetTLVData(0x9F06, aucAid, &nTempLen) == 0)//get aid
				{
					AppSetClss_capk(ucPkIndex, gl_aucFinalAID,gl_ucAppType);// capk
					AppSetClss_RevLst(ucPkIndex, gl_aucFinalAID,gl_ucAppType);
				}

				nRet = Clss_CardAuth_Wave(&ucACType, &ucDDAFailFlg);//capk

				if (nRet != 0) 
				{					
					if (nRet != CLSS_USE_CONTACT) 
					{
						nRet =  CLSS_TERMINATE;	
					}
				}
				else
				{
					if (ucDDAFailFlg == 1) 
					{
						nRet =  CLSS_TERMINATE;							
					}
					else
					{
						//modified by Kevin Liu 20160621
//						Inter_DisplayMsg(MSG_APPROVED);
					}
				}
			}
			else
			{
				if (ucACType == AC_ARQC || ucACType == AC_TC)
				{
					nRet = 0;
				}
				else // (ucACType == AC_AAC)
				{
					nRet = EMV_DENIAL;
				}
			}
		}

		break;
	case CLSS_VISA_WAVE2:
		if (ucACType == AC_TC)
		{
			Clss_DelAllRevocList_Wave();
			Clss_DelAllCAPK_Wave();

			if ( Clss_GetTLVData_Wave(0x8F, &ucPkIndex,  &nTempLen) == 0)
			{
				//if (AppConv_GetTLVData(0x9F06, aucAid, &nTempLen) == 0)
				{
					AppSetClss_capk(ucPkIndex, gl_aucFinalAID,gl_ucAppType);
					AppSetClss_RevLst(ucPkIndex, gl_aucFinalAID,gl_ucAppType);
				}
			}
			// internal authenticate, offline approval
			nRet = Clss_CardAuth_Wave(&ucACType, &ucDDAFailFlg);

			if (nRet)
			{
				return EMV_DENIAL; // 0424-3
			}
		}
		appRemovePicc();


		break;
	default:
		//appRemovePicc(PICC_LED_NONE);
		nRet = CLSS_TERMINATE;
		break;
	}

	if (nRet == 0)
	{
		vAppSetTransACType(ucACType);
	}
	return nRet;
}


int nAppTransProc_MC(uchar ucTransPath, uchar *pucACType)
{
	logd((__func__));
	int nRet=0,nTempLen=0;
	uchar ucPkIndex,aucAid[17]={0};
	CLSS_TORN_LOG_RECORD atAppTornTransLog[5];//Torn Transaction Log
	int nAppTornLogNum = 0;//number of Tornlog
	int nUpdatedFlg = 0;

	if (ucTransPath == CLSS_MC_MCHIP) // 0x06)
	{
		Clss_DelAllRevocList_MC_MChip();
		Clss_DelAllCAPK_MC_MChip();

		if (Clss_GetTLVDataList_MC((uchar*)"\x8F", 1, 1, &ucPkIndex, (uint *)&nTempLen) == 0)
		{
			if (Clss_GetTLVDataList_MC((uchar*)"\x4F", 1, sizeof(aucAid), aucAid, (uint *)&nTempLen) == 0)
			{

				AppSetClss_capk(ucPkIndex, aucAid, gl_ucAppType);// CAPK [1/4/2015 jiangjy]
				AppSetClss_RevLst(ucPkIndex, aucAid, gl_ucAppType);
			}
		}

		nAppGetTornLog_MC(atAppTornTransLog, &nAppTornLogNum);
		if (nAppTornLogNum)
		{
			Clss_SetTornLog_MC_MChip(atAppTornTransLog, nAppTornLogNum);
		}
		nRet = Clss_TransProc_MC_MChip(pucACType);
		memset(atAppTornTransLog, 0, sizeof(atAppTornTransLog));
		nAppTornLogNum = 0;
		nUpdatedFlg = 0;
		Clss_GetTornLog_MC_MChip(atAppTornTransLog, &nAppTornLogNum, &nUpdatedFlg);
		if (nUpdatedFlg)
		{
			nAppSaveTornLog_MC(atAppTornTransLog, nAppTornLogNum);
		}
	}
	else if (ucTransPath == CLSS_MC_MAG) // 0x05)
	{
		nRet = Clss_TransProc_MC_Mag(pucACType);
	}
	else
	{
		nRet = CLSS_TERMINATE;
	}

	if (nRet != ICC_CMD_ERR)//3G10-9301(Trx1_CCC_Res) 3G10-9300(Trx2_CCC_Res)
	{
		appRemovePicc();//appRemovePicc(PICC_LED_GREEN);
		//Inter_DisplayMsg(MSG_CARD_READ_OK);
	}
	if (nRet)
	{
		return nRet;
	}

	vAppSetTransACType(*pucACType);

	return EMV_OK;
}

//added by kevinliu 2015/10/16
int nAppTransProc_AE(uchar ucTransPath, uchar *pucACType)
{
	int iRet = 0, iTempLen = 0;
	uchar ucAdviceFlag = 0;		
	uchar ucOnlineFlag = 0;
	uchar ucOptimizeFlag = 0;
	uchar aucTmTransCapa[4] = {0};
	uchar ucCardType = 0;
	int iErrorCode = 0;
	Clss_AddReaderParam_AE tAERdParam;
	ONLINE_PARAM ptOnlineParam;

	iRet = Clss_ReadRecord_AE(&ucOptimizeFlag);
	if (iRet != EMV_OK) 
	{
		return iRet;
	}
/*
	Clss_DelAllRevocList_AE();
	Clss_DelAllCAPK_AE();
	if (Clss_GetTLVData_AE(0x8F, &ucPkIndex, &iTempLen) == 0)
	{
		if (Clss_GetTLVData_AE(0x9F06, aucAid, &iTempLen) == 0)
		{
			if (AppSetClss_capk(ucPkIndex, aucAid, KERNTYPE_AE) != 0)
			{
				return iRet;
			}
			if (AppSetClss_RevLst(ucPkIndex, aucAid, KERNTYPE_AE) != 0)
			{
				return iRet;
			}
		}
	}
*/
	iRet = Clss_CardAuth_AE();
	if (iRet != EMV_OK) 
	{
		return iRet;
	}
	
	//do not support fullonline
	iRet = Clss_StartTrans_AE(0, &ucAdviceFlag, &ucOnlineFlag);
	if (iRet != EMV_OK) 
	{
		Clss_GetDebugInfo_AE(0, NULL, &iErrorCode);
	}

	ucCardType = ucAppGetTransPath();
	//TODO
	Clss_GetTLVData_AE(0x9F6E, aucTmTransCapa, &iTempLen);
	if (iRet == EMV_OK)//3G10-9301(Trx1_CCC_Res) 3G10-9300(Trx2_CCC_Res)
	{
		if ((ucOnlineFlag != 1) || (ucCardType ==  AE_MAGMODE) || ((aucTmTransCapa[0] & 0x20) == 0))    //full online not supported)
		{
		//Modified by Kevin Liu 20160801, remove card before prompt
			appRemovePicc();
			//Inter_DisplayMsg(MSG_CARD_READ_OK);
		}
	    if (ucOnlineFlag)
	    {
	    	*pucACType = AC_ARQC ;
	    }
	    else
	    {
	    	*pucACType = AC_TC ;
	    }

	}
	else if ((iRet == CLSS_CVMDECLINE) || (iRet == EMV_DENIAL))
	{
		if ((ucOnlineFlag != 1) || (ucCardType ==  AE_MAGMODE) || ((aucTmTransCapa[0] & 0x20) == 0))    //full online not supported)
		{
			//Modified by Kevin Liu 20160801, remove card before prompt
			appRemovePicc();
			Inter_DisplayMsg(MSG_DECLINED);
		}
		*pucACType = AC_AAC;
		vAppSetTransACType(*pucACType);
		return CLSS_DECLINE;
	}
	else if (iRet == CLSS_REFER_CONSUMER_DEVICE)
	{
		if ((ucOnlineFlag != 1) || (ucCardType ==  AE_MAGMODE) || ((aucTmTransCapa[0] & 0x20) == 0))    //full online not supported)
		{
			//Modified by Kevin Liu 20160801, remove card before prompt
			appRemovePicc();
			Inter_DisplayMsg(MSG_SEE_PHONE);
		}

		*pucACType = AC_AAC;
		vAppSetTransACType(*pucACType);
		return App_Try_Again;
	}
	else
	{
		if ((ucOnlineFlag != 1) || (ucCardType ==  AE_MAGMODE) || ((aucTmTransCapa[0] & 0x20) == 0))    //full online not supported)
		{
			appRemovePicc();
		}
		return iRet;
	}
	vAppSetTransACType(*pucACType);

	//Delayed Authorization
	if (ucOnlineFlag == 1)
	{
		memset(&tAERdParam,0, sizeof(tAERdParam));
		iRet = Clss_GetAddReaderParam_AE(&tAERdParam);
		if (iRet)
		{
			return iRet;
		}

		memset(&ptOnlineParam, 0, sizeof(ONLINE_PARAM));
		memcpy(ptOnlineParam.aucAuthCode, "00", 2);
		if (tAERdParam.ucDelayAuthFlag == 1)
		{
			iRet = Clss_CompleteTrans_AE(ONLINE_APPROVE,AE_DELAYAUTH_PARTIALONLINE, &ptOnlineParam,&ucAdviceFlag);
			if(!iRet)
			{
				*pucACType = AC_TC;
			}
		}
	}
	appRemovePicc();
	return EMV_OK;
}

int nAppTransProc_JCB(uchar ucTransPath, uchar *pucACType)
{
	int iRet=0, iTempLen=0;
	uchar ucPkIndex = 0, aucAid[17] = {0}, ucExceptFileFlg = 0;

	if (ucTransPath == CLSS_JCB_EMV)
	{
		Clss_DelAllRevocList_JCB();
		Clss_DelAllCAPK_JCB();

		if (Clss_GetTLVDataList_JCB((uchar*)"\x8F", 1, 1, &ucPkIndex, (uint *)&iTempLen) == 0)
		{
			if (Clss_GetTLVDataList_JCB((uchar*)"\x4F", 1, sizeof(aucAid), aucAid, (uint *)&iTempLen) == 0)
			{
				AppSetClss_capk(ucPkIndex, aucAid, gl_ucAppType);
				AppSetClss_RevLst(ucPkIndex, aucAid, gl_ucAppType);
			}
		}

		ucExceptFileFlg = 0;
		iRet = Clss_TransProc_JCB(ucExceptFileFlg);
		if(iRet != EMV_OK)
		{
			return iRet;
		}

		iRet = Clss_CardAuth_JCB();
		if(iRet != EMV_OK)
		{
			return iRet;
		}

	}
	else
	{
		ucExceptFileFlg = 0;
		iRet = Clss_TransProc_JCB(ucExceptFileFlg);
		if(iRet != EMV_OK)
		{
			return iRet;
		}
	}
	appRemovePicc();
	return EMV_OK;
}



int cSendTransDataOutput_DPAS(uchar ucMsgFlag)
{
	return nAppSaveTransDataOutput_DPAS(ucMsgFlag);
}

// added by Gillian 2015/12/17
int nAppSaveTransDataOutput_DPAS(uchar ucMsgFlag)
{
	uint unLen=0, unBuffLen=0;
	uchar aucBuff[1024];
	if(ucMsgFlag & CLSS_DATA_OCPS)
	{
		Clss_GetTLVDataList_DPAS((uchar *)"\xDF\x81\x29", 3, sizeof(gl_aucOutcomeParamSet), gl_aucOutcomeParamSet, &unLen);
	}
	if(ucMsgFlag & CLSS_DATA_UIRD)
	{
		Clss_GetTLVDataList_DPAS((uchar *)"\xDF\x81\x16", 3, sizeof(gl_aucUserInterReqData), gl_aucUserInterReqData, &unLen);
	}
	if(ucMsgFlag & CLSS_DATA_DISD)
	{
		Clss_GetTLVDataList_DPAS((uchar *)"\xDF\x81\x15", 3, sizeof(gl_aucErrIndication), gl_aucErrIndication, &unLen);
		memset(aucBuff, 0, sizeof(aucBuff));
		Clss_GetTLVDataList_DPAS((uchar *)"\xFF\x81\x06", 3, sizeof(aucBuff), aucBuff, &unBuffLen);
		if (App_SearchTLV(0, (uchar *)"\x9F\x42", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucAppCurrCodeFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucAppCurrCodeFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucAppCurrCodeFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x04", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucBalBeforeGACFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucBalBeforeGACFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucBalBeforeGACFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x05", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucBalAfterGACFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucBalAfterGACFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucBalAfterGACFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x02", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucDSSum3Flg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucDSSum3Flg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucDSSum3Flg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x0B", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucDSSumStFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucDSSumStFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucDSSumStFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x15", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				Clss_GetTLVDataList_MC((uchar*)"\xDF\x81\x15", 3, sizeof(gl_aucErrIndication), gl_aucErrIndication, &unLen);
				gl_DiscretionayData.ucErrIndicFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucErrIndicFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucErrIndicFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x0E", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucPostGACPutDataStFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucPostGACPutDataStFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucPostGACPutDataStFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x0F", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucPreGACPutDataStFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucPreGACPutDataStFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucPreGACPutDataStFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\x9F\x6E", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucThirdPartyDataFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucThirdPartyDataFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucThirdPartyDataFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xFF\x81\x01", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucTornRecordFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucTornRecordFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucTornRecordFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x2A", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucDDCardTrack1Flg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucDDCardTrack1Flg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucDDCardTrack1Flg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x2B", aucBuff, aucBuff+unBuffLen, (int *)&unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucDDCardTrack2Flg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucDDCardTrack2Flg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucDDCardTrack2Flg = APP_TAG_NOT_EXIST;
		}
	}
	return EMV_OK;
}


// added by Gillian 2015/12/17
int nAppTransProc_DPAS(uchar ucTransPath, uchar *pucACType)
{
	int iRet = 0, nTempLen = 0, iLen = 0, i = 0;
	uchar ucPkIndex, aucAid[17]= {0}, ucBlackFile = 0, ucTemp[100] = {0};

	iRet = Clss_ReadData_DPAS();
	if(iRet)
	{
		cSendTransDataOutput_DPAS(0x07);
	}
	if (iRet == CLSS_RESELECT_APP)
	{
		iRet = Clss_DelCurCandApp_Entry();
		if (iRet == 0)
		{
			iRet = CLSS_RESELECT_APP;
		}
		return iRet;
	}
	else if (iRet)
	{
		return iRet;
	}
	if (gl_aucUserInterReqData[0] == MI_SEE_PHONE)
	{
		Inter_DisplayMsg(MSG_SEE_PHONE);
	}
	if (gl_aucOutcomeParamSet[0] == OC_TRY_AGAIN || gl_aucOutcomeParamSet[1] != OC_NA)
	{
		return App_Try_Again;
	}
	if (ucTransPath == CLSS_DPAS_EMV) // 0x06)
	{
		Clss_DelAllRevocList_DPAS();
		Clss_DelAllCAPK_DPAS();

		if (Clss_GetTLVDataList_DPAS((uchar*)"\x8F", 1, 1, &ucPkIndex, (uint *)&nTempLen) == 0)
		{
			if (Clss_GetTLVDataList_DPAS((uchar*)"\x4F", 1, sizeof(aucAid), aucAid, (uint *)&nTempLen) == 0)
			{

				AppSetClss_capk(ucPkIndex, aucAid, gl_ucAppType);// CAPK [1/4/2015 jiangjy]
				AppSetClss_RevLst(ucPkIndex, aucAid, gl_ucAppType);
			}
		}
	}
	iRet = Clss_TransProc_DPAS(ucBlackFile);

	cSendTransDataOutput_DPAS(0x07);
	if (gl_aucUserInterReqData[0] == MI_SEE_PHONE)
	{
		Inter_DisplayMsg(MSG_SEE_PHONE);
	}
	if (gl_aucOutcomeParamSet[0] == OC_TRY_AGAIN || gl_aucOutcomeParamSet[1] != OC_NA)
	{
		return App_Try_Again;
	}
	if ((gl_aucOutcomeParamSet[0]&0xF0) == CLSS_OC_APPROVED )
	{
		//Inter_DisplayMsg(MSG_CARD_READ_OK);
		if ((gl_aucOutcomeParamSet[3]&0xF0) == CLSS_CVM_ONLINE_PIN)
		{
			*pucACType = AC_ARQC ;
		}
		else
		{
			*pucACType = AC_TC ;
		}
		appRemovePicc();
	}
	else if ((gl_aucOutcomeParamSet[0]&0xF0) == CLSS_OC_ONLINE_REQUEST)
	{
		//Modified by Kevin Liu 20160801, remove card before prompt
		appRemovePicc();
		//Inter_DisplayMsg(MSG_CARD_READ_OK);
		*pucACType = AC_ARQC;
	}
	else
	{
		appRemovePicc();
		Inter_DisplayMsg(MSG_DECLINED);
		*pucACType = AC_AAC;
		if (iRet == EMV_OK)
		{
			iRet = CLSS_DECLINE;
		}
	}
	vAppSetTransACType(*pucACType);
	return iRet;
}

int AppSetClss_capk(uchar index, uchar *rid, uchar kerId)
{
	int i = 0, iRet = -1;

	for(i=0; i<MAX_KEY_NUM; i++)
	{
		if(memcmp(glCAPKeyList[i].RID, rid, 5) != 0)
		{
			continue;
		}
		if(index != glCAPKeyList[i].KeyID)
		{
			continue;
		}

		else if(kerId == KERNTYPE_VIS)
		{
			iRet = Clss_AddCAPK_Wave(&glCAPKeyList[i]);
		}
		else if(kerId == KERNTYPE_MC)
		{
		    iRet = Clss_AddCAPK_MC_MChip(&glCAPKeyList[i]);
		}
		else if(kerId == KERNTYPE_AE)
		{
		    iRet = Clss_AddCAPK_AE(&glCAPKeyList[i]);
		}
		else if(kerId == KERNTYPE_ZIP)
		{
		    iRet = Clss_AddCAPK_DPAS(&glCAPKeyList[i]);
		}
		else if(kerId == KERNTYPE_JCB)
		{
			iRet = Clss_AddCAPK_JCB(&glCAPKeyList[i]);
		}
		else
		{
			return ERR_TRAN_FAIL;
		}
	}

	return 0;
}


int AppSetClss_RevLst(uchar index, uchar *rid, uchar kerId)
{
	int iRet = 0;
	EMV_REVOCLIST tRevocList;

// 	memset(ucAid, 0, sizeof(ucAid));
// 	PubAsc2Bcd(rid, strlen(rid), ucAid);
	memcpy(tRevocList.ucRid, rid, 5);
	tRevocList.ucIndex = index;
	memcpy(tRevocList.ucCertSn, "\x00\x07\x11", 3);

	if (kerId == KERNTYPE_VIS)
	{
		Clss_AddRevocList_Wave(&tRevocList);
	}
//	else		//modified by kevinliu 2015/10/19
	else if (kerId == KERNTYPE_MC)
	{
		Clss_AddRevocList_MC_MChip(&tRevocList);
	}
	else if (kerId == KERNTYPE_AE)		//added by kevinliu 2015/10/19
	{
		Clss_AddRevocList_AE(&tRevocList);
	}
	else if (kerId == KERNTYPE_ZIP)		//added by kevinliu 2015/10/19
	{
	    Clss_AddRevocList_DPAS(&tRevocList);
	}
	else if(kerId == KERNTYPE_JCB)
	{
		Clss_AddRevocList_JCB(&tRevocList);
	}
	else
	{
		return ERR_TRAN_FAIL;
	}
	return 0;
}

void vAppSetTransACType(uchar ucType)
{
	gl_ucACType = ucType;
}

// display message [1/5/2015 jiangjy]
void Inter_DisplayMsg(unsigned int iMsg)
{
	switch(iMsg)
	{
	case MSG_CARD_READ_OK:
		SetClssLightStatus(CLSSLIGHTSTATUS_READCARDDONE);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD READ OK"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_REMOVE_CARD:
		SetClssLightStatus(CLSSLIGHTSTATUS_REMOVECARD);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS REMOVE CARD"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case ICC_BLOCK:
        SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD LOCKED"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_TRY_AGAIN:
		SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS TRY AGAIN"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_SEE_PHONE:
        SetClssLightStatus(CLSSLIGHTSTATUS_PROCESSING);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS SEE PHONE"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_APPROVED:
        SetClssLightStatus(CLSSLIGHTSTATUS_COMPLETE);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("APPROVED"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_DECLINED:
        SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("DECLINED"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_APPROVED_SIGN:
        SetClssLightStatus(CLSSLIGHTSTATUS_COMPLETE);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("APPROVED WITH SIG"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_OTHER_CARD:
        SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TRY OTHER CARD"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_INSERT_CARD:
        SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS INSERT CARD"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_AUTHORISING:
        SetClssLightStatus(CLSSLIGHTSTATUS_PROCESSING);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("AUTHORISING..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_CLEAR_DISPLAY:
		Gui_ClearScr();
		break;
	case MSG_TAP_CARD_AGAIN:
        SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TAP CARD AGAIN"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_TAP_CARD_WITH_AMOUNT:
        SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TAP CARD WITH AMOUNT"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_TAP_CARD_WITH_BALANCE:
        SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TAP CARD WITH BALANCE"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_TRY_ANOTHER_INTERFACE:
        SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS USE CONTACT"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	case MSG_END_APPLICATION:
        SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("END APPLICATION"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
		break;
	default:
		break;
	}
	//PubWaitKey(30);//delay
	DelayMs(200);
}


void appRemovePicc()
{
	uchar ucRet = 0; 
	int i = 0;

	SetClssLightStatus(CLSSLIGHTSTATUS_REMOVECARD);

	//  [4/11/2011 yingl]
	ucRet = PiccRemove('r', 0);
	if ((ucRet == 0) || (ucRet == 1)) 
	{  
		PiccLight(PICC_LED_ALL, 0);
		PiccClose();
		return;
	}

	i = 0;
	while (ucRet) 
	{
		ucRet = PiccRemove('r', 0); 
		if ((ucRet == 2) || (ucRet == 3))
		{
			break;
		} 
		else
		{
			if (i == 0) {
				Gui_ClearScr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS REMOVE CARD"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
				Beep();
				i++;
			}
			//		ScrPrint(0,0,0,"PiccRemove:%d %lu", ucRet, i++);
		}
	}
	PiccLight(PICC_LED_ALL, 0);
	PiccClose();
	Gui_ClearScr();
}

int AppConv_CreateOutCome(int nRet, uchar ucACType, CLSS_OUTCOME_DATA *pstOutComeData)
{
	uchar aucOutcomeParamSet[8] = {0};
	uchar ucCVMType = 0;
	uint unLen=0;
	
	if(pstOutComeData == NULL)
	{
		return EMV_PARAM_ERR;
	}
	memset(pstOutComeData, 0, sizeof(CLSS_OUTCOME_DATA));
	pstOutComeData->unPathType = ucAppGetTransPath();
	switch (ucAppGetAppType())
	{
	case KERNTYPE_VIS:
		if (!nRet)
		{
			nRet = Clss_GetCvmType_Wave(&pstOutComeData->ucCVMType);//  [1/22/2015 jiangjy]
			if (nRet)
			{
				pstOutComeData->ucTransRet = CLSS_DECLINED;
			}
			else
			{
				if(ucACType == AC_TC)
				{
					pstOutComeData->ucTransRet = CLSS_APPROVE;
				}
				else if(ucACType == AC_ARQC)//AC_ARQC
				{
					pstOutComeData->ucTransRet = CLSS_ONLINE_REQUEST;
				}
				else
				{
					pstOutComeData->ucTransRet = CLSS_DECLINED;
				}
			}

		}
		else if (nRet == EMV_DENIAL || nRet == CLSS_DECLINE)
		{
			pstOutComeData->ucTransRet = CLSS_DECLINED;
		}
		else if (nRet == CLSS_USE_CONTACT)
		{
			pstOutComeData->ucTransRet = CLSS_TRY_ANOTHER_INTERFACE;
		}
		else
		{
			pstOutComeData->ucTransRet = CLSS_END_APPLICATION;
		}
		break;
	case KERNTYPE_MC:
		memset(aucOutcomeParamSet, 0, sizeof(aucOutcomeParamSet));
		Clss_GetTLVDataList_MC("\xDF\x81\x29", 3, sizeof(aucOutcomeParamSet), aucOutcomeParamSet, &unLen);
		switch(aucOutcomeParamSet[0] & 0xF0)
		{
		case CLSS_OC_APPROVED:
			pstOutComeData->ucTransRet = CLSS_APPROVE;
			break;
		case CLSS_OC_DECLINED:
			pstOutComeData->ucTransRet = CLSS_DECLINED;
			break;
		case CLSS_OC_ONLINE_REQUEST:
			pstOutComeData->ucTransRet = CLSS_ONLINE_REQUEST;
			break;
		case CLSS_OC_TRY_ANOTHER_INTERFACE:
			pstOutComeData->ucTransRet = CLSS_TRY_ANOTHER_INTERFACE;
			break;
		default://CLSS_OC_END_APPLICATION
			pstOutComeData->ucTransRet = CLSS_END_APPLICATION;
			break;
		}
		switch(aucOutcomeParamSet[3]&0xF0)
		{
		case CLSS_OC_NO_CVM:
			pstOutComeData->ucCVMType = CLSS_CVM_NO;
			break;
		case CLSS_OC_OBTAIN_SIGNATURE:
			pstOutComeData->ucCVMType = CLSS_CVM_SIG;
			break;
		case CLSS_OC_ONLINE_PIN:
			pstOutComeData->ucCVMType = CLSS_CVM_ONLINE_PIN;
			break;
		case CLSS_OC_CONFIRM_CODE_VER:
			pstOutComeData->ucCVMType = CLSS_CVM_OFFLINE_PIN;
			break;
		default:
			pstOutComeData->ucCVMType = CLSS_CVM_NO;
			break;
		}
		break;

	case KERNTYPE_AE:			//added by kevinliu 2015/10/19
		if (!nRet)
		{
			nRet = Clss_GetCVMType_AE(&ucCVMType);
			if(ucACType == AC_TC)
			{
				pstOutComeData->ucTransRet = CLSS_APPROVE;
			}
			else if(ucACType == AC_ARQC)//AC_ARQC
			{
				pstOutComeData->ucTransRet = CLSS_ONLINE_REQUEST;
			}
			else
			{
				pstOutComeData->ucTransRet = CLSS_DECLINED;
			}
		}
		else if (nRet == EMV_DENIAL || nRet == CLSS_DECLINE)
		{
			pstOutComeData->ucTransRet = CLSS_DECLINED;
		}
		else if (nRet == CLSS_USE_CONTACT)
		{
			pstOutComeData->ucTransRet = CLSS_TRY_ANOTHER_INTERFACE;
		}
		else
		{
			pstOutComeData->ucTransRet = CLSS_END_APPLICATION;
		}

		switch(ucCVMType)
		{
		case RD_CVM_NO:
			pstOutComeData->ucCVMType = CLSS_CVM_NO;
			break;
		case RD_CVM_SIG:
			pstOutComeData->ucCVMType = CLSS_CVM_SIG;
			break;
		case RD_CVM_ONLINE_PIN:
			pstOutComeData->ucCVMType = CLSS_CVM_ONLINE_PIN;
			break;
		case RD_CVM_OFFLINE_PIN:
			pstOutComeData->ucCVMType = CLSS_CVM_OFFLINE_PIN;
			break;
		case RD_CVM_CONSUMER_DEVICE:
			pstOutComeData->ucCVMType = CLSS_CVM_CONSUMER_DEVICE;
			break;
		default:
			pstOutComeData->ucCVMType = CLSS_CVM_NO;
			break;
		}
		break;
	case KERNTYPE_ZIP:				//added by Gillian 2015/12/17
		memset(aucOutcomeParamSet, 0, sizeof(aucOutcomeParamSet));
		Clss_GetTLVDataList_DPAS("\xDF\x81\x29", 3, sizeof(aucOutcomeParamSet), aucOutcomeParamSet, &unLen);
		switch(aucOutcomeParamSet[0]&0xF0)
		{
		case CLSS_OC_APPROVED:
			pstOutComeData->ucTransRet = CLSS_APPROVE;
			break;
		case CLSS_OC_DECLINED:
			pstOutComeData->ucTransRet = CLSS_DECLINED;
			break;
		case CLSS_OC_ONLINE_REQUEST:
			pstOutComeData->ucTransRet = CLSS_ONLINE_REQUEST;
			break;
		case CLSS_OC_TRY_ANOTHER_INTERFACE:
			pstOutComeData->ucTransRet = CLSS_TRY_ANOTHER_INTERFACE;
			break;
		default://CLSS_OC_END_APPLICATION
			pstOutComeData->ucTransRet = CLSS_END_APPLICATION;
			break;
		}
		switch(aucOutcomeParamSet[3]&0xF0)
		{
		case CLSS_OC_NO_CVM:
			pstOutComeData->ucCVMType = CLSS_CVM_NO;
			break;
		case CLSS_OC_OBTAIN_SIGNATURE:
			pstOutComeData->ucCVMType = CLSS_CVM_SIG;
			break;
		case CLSS_OC_ONLINE_PIN:
			pstOutComeData->ucCVMType = CLSS_CVM_ONLINE_PIN;
			break;
		case CLSS_OC_CONFIRM_CODE_VER:
			pstOutComeData->ucCVMType = CLSS_CVM_OFFLINE_PIN;
			break;
		default:
			pstOutComeData->ucCVMType = CLSS_CVM_NO;
			break;
		}
		break;
	case KERNTYPE_JCB:
		memset(aucOutcomeParamSet, 0, sizeof(aucOutcomeParamSet));
		Clss_GetTLVDataList_JCB("\xDF\x81\x29", 3, sizeof(aucOutcomeParamSet), aucOutcomeParamSet, &unLen);
		switch(aucOutcomeParamSet[0] & 0xF0)
		{
		case CLSS_OC_APPROVED:
			pstOutComeData->ucTransRet = CLSS_APPROVE;
			break;
		case CLSS_OC_DECLINED:
			pstOutComeData->ucTransRet = CLSS_DECLINED;
			break;
		case CLSS_OC_ONLINE_REQUEST:
			pstOutComeData->ucTransRet = CLSS_ONLINE_REQUEST;
			break;
		case CLSS_OC_TRY_ANOTHER_INTERFACE:
			pstOutComeData->ucTransRet = CLSS_TRY_ANOTHER_INTERFACE;
			break;
		default://CLSS_OC_END_APPLICATION
			pstOutComeData->ucTransRet = CLSS_END_APPLICATION;
			break;
		}
		switch(aucOutcomeParamSet[3] & 0xF0)
		{
		case CLSS_OC_NO_CVM:
			pstOutComeData->ucCVMType = CLSS_CVM_NO;
			break;
		case CLSS_OC_OBTAIN_SIGNATURE:
			pstOutComeData->ucCVMType = CLSS_CVM_SIG;
			break;
		case CLSS_OC_ONLINE_PIN:
			pstOutComeData->ucCVMType = CLSS_CVM_ONLINE_PIN;
			break;
		case CLSS_OC_CONFIRM_CODE_VER:
			pstOutComeData->ucCVMType = CLSS_CVM_OFFLINE_PIN;
			break;
		default:
			pstOutComeData->ucCVMType = CLSS_CVM_NO;
			break;
		}
		break;
	default:
		break;
	}
	return nRet;

}

//display
void vAppDisplayMsgByRet(int nRet)
{
	switch (nRet)
	{
	//added by Kevin Liu 20160624 bug830
	case CLSS_TERMINATE:

	case CLSS_DECLINE:
	case EMV_DENIAL:
		Inter_DisplayMsg(MSG_DECLINED);
		break;
	case CLSS_USE_CONTACT:
		Inter_DisplayMsg(MSG_TRY_ANOTHER_INTERFACE);
		break;
	case ICC_RESET_ERR:
	case ICC_CMD_ERR:
		Inter_DisplayMsg(MSG_TRY_AGAIN);
		break;
	case CLSS_REFER_CONSUMER_DEVICE:
		Inter_DisplayMsg(MSG_SEE_PHONE);
		break;
	default:
		Inter_DisplayMsg(MSG_END_APPLICATION);
		break;
	}

	Beep();
	DelayMs(100);
	Beep();
	DelayMs(1000);
}

void vAppInitSchemeId(void)
{
	memset(&gl_stSchemeId,0, sizeof(gl_stSchemeId));
	gl_stSchemeId.ucSupportVISA = 1;
	gl_stSchemeId.ucSupportMC = 1;
	gl_stSchemeId.ucSupportAE = 1;
	gl_stSchemeId.ucSupportDPAS = 1;
	gl_stSchemeId.ucSupportJCB = 1;
}

int ProcError_Picc(int ret)
{  
	if (!ret) {
		return 0;
	}
	else {
		appRemovePicc();
	}
	
	if(ret == CLSS_CVMDECLINE)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TRANSACTION NOT PERMITTED"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == ICC_RESET_ERR||ret == ICC_CMD_ERR)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD READ ERROR"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == ICC_BLOCK)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD BLOCKED "), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == CLSS_CARD_EXPIRED)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("CARD EXPIRED "), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == EMV_RSP_ERR||ret == EMV_DATA_ERR||ret == CLSS_TERMINATE||ret == EMV_DENIAL||ret == CLSS_DECLINE)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TRANSACTION TERMINATED"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == CLSS_USE_CONTACT)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS USE CONTACT"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == EMV_TIME_OUT)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TIME OUT"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == EMV_USER_CANCEL)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("USER CANCEL"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == EMV_APP_BLOCK)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("APP BLOCKED"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == EMV_NOT_ACCEPT)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("NOT ACCEPTED"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == EMV_DENIAL)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TRANSACTION DCLINED"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == NO_TRANS_LOG)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("NO TRANS LOG"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == RECORD_NOTEXIST)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("RECORD NOT EXIST"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == RECORD_NOTEXIST)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("RECORD NOT EXIST"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == EMV_NO_APP)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("USE CONTACT"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	else if(ret == CLSS_PARAM_ERR)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PARAM ERR"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		Beep();
	}
	DelayMs(200);
	Beep();

	return ret;
}

void vAppInitPaymentData_MC(void)//KS.2 paypass 3.0.1 by zhoujie
{
	memset(gl_aucOutcomeParamSet, 0, sizeof(gl_aucOutcomeParamSet));
	gl_aucOutcomeParamSet[0] = OC_END_APPLICATION;//for s51.13
	gl_aucOutcomeParamSet[1] = OC_NA;//s51.3
	gl_aucOutcomeParamSet[2] = OC_NA;
	gl_aucOutcomeParamSet[3] = OC_NA;
	gl_aucOutcomeParamSet[4] |= 0x10;
	gl_aucOutcomeParamSet[5] = OC_NA;
	gl_aucOutcomeParamSet[6] = 0xFF;

	memset(gl_aucUserInterReqData, 0, sizeof(gl_aucUserInterReqData));
	gl_aucUserInterReqData[0] = MI_NA;
	gl_aucUserInterReqData[1] = MI_NA;
	memcpy(gl_aucUserInterReqData+2, MSG_HOLD_TIME_VALUE, 3);

	memset(gl_aucErrIndication, 0, sizeof(gl_aucErrIndication));
	gl_aucErrIndication[5] = MI_NA;

	vAppInitDiscData_MC();
}

uchar ucAppGetTransACType(void)
{
	return gl_ucACType;
}


//**************************************************************
uchar *App_GetFirstTLV(uchar *dat, uchar *datend, uchar *pucTag, uint *punLen)//  [1/9/2013 ZhouJie]
{
	uint i, j, unTempDataLen=0;
	uchar ucTagLen=0;
	uchar *pucDataStart=NULL, *pucDataEnd=NULL, *pucTempTag=NULL;
	if (dat == NULL || datend == NULL || pucTag == NULL || punLen == NULL)
	{
		return NULL;
	}
	pucDataStart = dat;
	pucDataEnd = datend;
	while (pucDataStart < pucDataEnd)
	{
		ucTagLen=0;
		pucTempTag = pucDataStart++;
		ucTagLen++;
		if (pucTempTag[0] == 0x00)
		{
			continue;
		}
		if ((pucTempTag[0] & 0x1F) == 0x1F)
		{
			ucTagLen++;
			while (pucDataStart < pucDataEnd && (*pucDataStart & 0x80))
			{
				pucDataStart++;
				ucTagLen++;
			}
			pucDataStart++;
		}
		if (pucDataStart >= pucDataEnd) // no length // G06-9260[11/24/2011 zhoujie]
		{
			return NULL;
		}
		if (*pucDataStart & 0x80)
		{
			i = (*pucDataStart & 0x7F);
			if (pucDataStart + i > pucDataEnd)
			{
				return NULL;
			}
			pucDataStart++;
			for (j = 0; j < i; j++)
			{
				unTempDataLen <<= 8;
				unTempDataLen += *pucDataStart++;
			}
		}
		else
		{
			unTempDataLen = *pucDataStart++;
		}
		memcpy(pucTag, pucTempTag, ucTagLen);
		*punLen = unTempDataLen;
		return pucDataStart;
	}
	return NULL;
}

unsigned char *App_SearchTLV(int DolList, unsigned char *pucTag, unsigned char *dat, unsigned char *datend, int *nLen)
{
	uint unTempLen;
	uchar aucTempTag[4];
	uchar *pucValue=NULL;
	uchar *pucDataStart, *pucDataEnd;
	if (dat == NULL || datend == NULL || pucTag == NULL)
	{
		return NULL;
	}
	memset(aucTempTag, 0, sizeof(aucTempTag));//for three byte of tag
	pucDataStart = dat;
	pucDataEnd = datend;
	while (pucDataStart < pucDataEnd) 
	{
		if (*pucDataStart == 0x00)
		{
			pucDataStart++;
			continue;
		}
		pucValue = App_GetFirstTLV(pucDataStart, pucDataEnd, aucTempTag, &unTempLen);
		if (pucValue == NULL)//can't find a TLV
		{
			return NULL;
		}
		if (!memcmp(aucTempTag, pucTag, ucAppGetTagLen(pucTag))) //successful
		{
			if (nLen != NULL) 
			{
				*nLen = unTempLen;
			}
			return pucValue;
		}
		pucDataStart = pucValue;
		if (aucTempTag[0] & 0x20) 
		{
			continue;
		}
		if (DolList) 
		{
			continue;
		}
		pucDataStart += unTempLen;//continue search next TLV
	}
	return NULL;
}
int nAppSaveTransDataOutput_MC(uchar ucMsgFlag)
{
	uint unLen=0, unBuffLen=0;
	uchar aucBuff[1024];
	if(ucMsgFlag & CLSS_DATA_OCPS)
	{
		Clss_GetTLVDataList_MC("\xDF\x81\x29", 3, sizeof(gl_aucOutcomeParamSet), gl_aucOutcomeParamSet, &unLen);
	}
	if(ucMsgFlag & CLSS_DATA_UIRD)
	{
		Clss_GetTLVDataList_MC("\xDF\x81\x16", 3, sizeof(gl_aucUserInterReqData), gl_aucUserInterReqData, &unLen);
	}
	if(ucMsgFlag & CLSS_DATA_DISD)
	{
		Clss_GetTLVDataList_MC("\xDF\x81\x15", 3, sizeof(gl_aucErrIndication), gl_aucErrIndication, &unLen);
		memset(aucBuff, 0, sizeof(aucBuff));
		Clss_GetTLVDataList_MC("\xFF\x81\x06", 3, sizeof(aucBuff), aucBuff, &unBuffLen);
		if (App_SearchTLV(0, (uchar *)"\x9F\x42", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucAppCurrCodeFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucAppCurrCodeFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucAppCurrCodeFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x04", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucBalBeforeGACFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucBalBeforeGACFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucBalBeforeGACFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x05", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucBalAfterGACFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucBalAfterGACFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucBalAfterGACFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x02", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucDSSum3Flg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucDSSum3Flg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucDSSum3Flg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x0B", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucDSSumStFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucDSSumStFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucDSSumStFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x15", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				Clss_GetTLVDataList_MC((uchar*)"\xDF\x81\x15", 3, sizeof(gl_aucErrIndication), gl_aucErrIndication, &unLen);
				gl_DiscretionayData.ucErrIndicFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucErrIndicFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucErrIndicFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x0E", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucPostGACPutDataStFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucPostGACPutDataStFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucPostGACPutDataStFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x0F", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucPreGACPutDataStFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucPreGACPutDataStFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucPreGACPutDataStFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\x9F\x6E", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucThirdPartyDataFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucThirdPartyDataFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucThirdPartyDataFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xFF\x81\x01", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucTornRecordFlg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucTornRecordFlg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucTornRecordFlg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x2A", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucDDCardTrack1Flg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucDDCardTrack1Flg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucDDCardTrack1Flg = APP_TAG_NOT_EXIST;
		}
		if (App_SearchTLV(0, (uchar *)"\xDF\x81\x2B", aucBuff, aucBuff+unBuffLen, &unLen))
		{
			if (unLen)
			{
				gl_DiscretionayData.ucDDCardTrack2Flg = APP_TAG_EXIST_WITHVAL;
			}
			else
			{
				gl_DiscretionayData.ucDDCardTrack2Flg = APP_TAG_EXIST_NOVAL;
			}
		}
		else
		{
			gl_DiscretionayData.ucDDCardTrack2Flg = APP_TAG_NOT_EXIST;
		}
	}
	return EMV_OK;
}

uchar ucAppGetTagLen(uchar *pucTag)
{
	uchar ucTagLen=0;
	int i;
	uchar szTmpTag[4];

	if (pucTag == NULL)
	{
		return 0;
	}

	//	if (pucTag[0] == 0x00)
	//	{
	//		return 0;
	//	}
	memset (szTmpTag, 0, sizeof(szTmpTag));
	for (i=0; i<4; i++)
	{
		if (pucTag[i] != 0x00)
		{
			memcpy(szTmpTag, &pucTag[i], 4-i);
			break;
		}
	}
	ucTagLen = 4-i;

	//	ucTagLen=0;
	//	if ((pucTag[0] & 0x1F) == 0x1F)
	//	{
	//		ucTagLen++;
	//		// add ucTagLen<4, invoid to check tag length over 4 bytes in case that tag variable is not initiated before set value
	//		// for example: pucTag is wrong set as 9F CC CC CC and the bytes ofter pucTag are not 0 [9/12/2014 ZhouJie]
	//		while (ucTagLen<4 && (pucTag[ucTagLen]&0x80))
	//		{
	//			ucTagLen++;
	//		}
	//		ucTagLen++;
	//	}
	//	else
	//	{
	//		ucTagLen=1;
	//	}
	return ucTagLen;
}


int cClssSendTransDataOutput_MC(uchar ucMsgFlag)
{
	return nAppSaveTransDataOutput_MC(ucMsgFlag);
}

