#include "global.h"
#include "Logger.h"
#include "util.h"
#include "xui.h"


extern int SetNetworkCommDetails(uchar *pucCommType, uchar ucAllowOptions);
int initializeComms(ushort uiShowSetup);


static void debugKeyInject() {

#ifdef APP_DEBUG
	ST_KEY_INFO     stTmp_Key;
	ST_KCV_INFO		stTmp_Kcv;
#endif

	//////////////////////////////////////////////////////////////////////////
	//temp
#ifdef APP_DEBUG
	logTrace("Doing APP_DEBUG key inject");
	PedErase();
	stTmp_Key.ucSrcKeyType = PED_TMK;
	stTmp_Key.ucSrcKeyIdx = 0;
	stTmp_Key.ucDstKeyType = PED_TMK;
	stTmp_Key.ucDstKeyIdx = MASTER_KEY_ID;
	memcpy(stTmp_Key.aucDstKeyValue,
		"\xAB\xCD\xEF\x01\x23\x45\x67\x89"\
		"\xAB\xCD\xEF\x01\x23\x45\x67\x89",
		16);
	stTmp_Key.iDstKeyLen = 16;
	stTmp_Kcv.iCheckMode = 0;
	PedWriteKey(&stTmp_Key, &stTmp_Kcv);

	stTmp_Key.ucSrcKeyType = PED_TMK;
	stTmp_Key.ucSrcKeyIdx = MASTER_KEY_ID;
	stTmp_Key.ucDstKeyType = PED_TPK;
	stTmp_Key.ucDstKeyIdx = DEF_PIN_KEY_ID;
	memcpy(stTmp_Key.aucDstKeyValue,
		"\xAB\xCD\xEF\x01\x23\x45\x67\x89"\
		"\xAB\xCD\xEF\x01\x23\x45\x67\x89",
		16);
	stTmp_Key.iDstKeyLen = 16;
	stTmp_Kcv.iCheckMode = 0;
	PedWriteKey(&stTmp_Key, &stTmp_Kcv);

	stTmp_Key.ucSrcKeyType = PED_TMK;
	stTmp_Key.ucSrcKeyIdx = MASTER_KEY_ID;
	stTmp_Key.ucDstKeyType = PED_TDK;
	stTmp_Key.ucDstKeyIdx = DEF_DATA_KEY_ID;
	memcpy(stTmp_Key.aucDstKeyValue,
		"\x12\x12\x12\x12\x12\x12\x12\x12"\
		"\x12\x12\x12\x12\x12\x12\x12\x12",
		16);
	stTmp_Key.iDstKeyLen = 16;
	stTmp_Kcv.iCheckMode = 0;
	PedWriteKey(&stTmp_Key, &stTmp_Kcv);
#endif
	//////////////////////////////////////////////////////////////////////////
}



static void initGUI(void) {
	logTrace(__func__);
	Gui_Init(_RGB_INT_(255, 255, 255), _RGB_INT_(0, 0, 0), NULL);

	unsigned char sTermInfo[30];
	//kbmute(0);
	GetTermInfo(sTermInfo);

	ST_FONT stFont[3] = { 0 };
	memcpy(stFont, gl_Font_Def, sizeof(gl_Font_Def));

	
	ST_FONT tempFonts[30] = { 0 };
	int ii = 0;
	int iRet = EnumFont(tempFonts, 30);
	for (ii = 0; ii<iRet; ii++)
	{
		logd(("Charset: %02d, Width: %02d, Height: %02d, Bold: %d, Italic: %d",
			tempFonts[ii].CharSet, tempFonts[ii].Width, tempFonts[ii].Height,
			tempFonts[ii].Bold, tempFonts[ii].Italic));
	}


	if (sTermInfo[19] & 0x02) {
		LOG_PRINTF(("IS COLOR"));

			stFont[0].Width = 8;
			stFont[0].Height = 16;

			stFont[1].Width = 12;
			stFont[1].Height = 24;
			stFont[1].Bold = 1;

			stFont[2].Width = 16;
			stFont[2].Height = 32;
			stFont[2].Bold = 0;
	}
	else {
		stFont[0].Width = 6;
		stFont[0].Height = 8;

		stFont[1].Width = 6;
		stFont[1].Height = 8;
		stFont[1].Bold = 0;

		stFont[2].Width = 8;
		stFont[2].Height = 16;
		stFont[2].Bold = 0;
	}

	ScrSelectFont(&stFont[0], NULL);
	ScrSelectFont(&stFont[1], NULL);
	ScrSelectFont(&stFont[2], NULL);

	Gui_LoadFont(GUI_FONT_SMALL, &stFont[0], NULL);
	Gui_LoadFont(GUI_FONT_NORMAL, &stFont[1], NULL);
	Gui_LoadFont(GUI_FONT_LARGE, &stFont[2], NULL);
}

static void loadDefaultPosParams() {
	logTrace(__func__);
	memset(&glPosParams, 0, sizeof(PosParams));
	
	glPosParams.ucPedMode = PED_INT_PCI;


	memset(&glPosParams.currency, 0, sizeof(CURRENCY_CONFIG));
	CURRENCY_CONFIG currency = { "NGN", "\x05\x66", "\x05\x66", 2, 0 };
	glPosParams.currency = currency;

#ifdef APP_DEBUG
	strncpy(glPosParams.tmsIp.szIP, "80.88.8.245", lengthOf(glPosParams.tmsIp.szIP));
	strncpy(glPosParams.tmsIp.szPort, "1205", lengthOf(glPosParams.tmsIp.szPort));
#else
	strncpy(glPosParams.tmsIp.szIP, "80.88.8.56", lengthOf(glPosParams.tmsIp.szIP));
	strncpy(glPosParams.tmsIp.szPort, "552", lengthOf(glPosParams.tmsIp.szPort));
#endif // APP_DEBUG
	glPosParams.tmsProtocolFlag = 0;

	//strncpy(glPosParams.switchIp.szIP, "196.6.103.73", lengthOf(glPosParams.switchIp.szIP));
	//strncpy(glPosParams.switchIp.szPort, "5043", lengthOf(glPosParams.switchIp.szPort));
	strncpy(glPosParams.supervisorPin, "1234", 4);
	strncpy(glPosParams.operatorPin, "1234", 4);
	strncpy(glPosParams.adminPass, "6798", 4);

	strncpy(glPosParams.switchHostName, "ctms.nibss-plc.com", lengthOf(glPosParams.switchHostName));
	strncpy(glPosParams.hostZMK, "DBEECACCB4210977ACE73A1D873CA59F", ASCII_KEY_SIZE);
	glPosParams.requestTimeOutSec = 60;
	glPosParams.callHomeTimeMinutes = 60;
	glPosParams.switchPortFlag = 1;
	PutEnv("E_SSL", "1");
	glPosParams.batchNo = 1;
	glPosParams.sequenceNo = 1;
	glPosParams.approvedReceiptCount = 2;
	glPosParams.declinedReceiptCount = 1;


	glPosParams.commConfig.ucCommType = CT_GPRS;
	glPosParams.commConfig.ucCommTypeBak = CT_NONE;
	glPosParams.commConfig.pfUpdWaitUI = DispWaitRspStatus;
	glPosParams.commConfig.ucTCPClass_BCDHeader = FALSE;

	strncpy(glPosParams.commConfig.stWirlessPara.stHost1.szIP, "196.6.103.73", lengthOf(glPosParams.commConfig.stWirlessPara.stHost1.szIP));
	strncpy(glPosParams.commConfig.stWirlessPara.stHost1.szPort, "5043", lengthOf(glPosParams.commConfig.stWirlessPara.stHost1.szPort));
	strncpy(glPosParams.commConfig.stWirlessPara.szAPN, "9mobile", lengthOf(glPosParams.commConfig.stWirlessPara.szAPN));
	strncpy(glPosParams.commConfig.stWirlessPara.szDNS, "8.8.8.8", lengthOf(glPosParams.commConfig.stWirlessPara.szDNS));
	glPosParams.commConfig.stWirlessPara.ucUsingSlot = 0;

	SavePosParams();
}

static void initializeEMV(void) {
	logTrace(__func__);
	LOG_PRINTF(("main() -> EmvTst Start!!!"));

	int iRc = EMVCoreInit();
	if (iRc == EMV_KEY_EXP)
	{
		logTrace("EMV_KEY_EXP == clear CAPKeys");
		EraseExpireCAPK();
	}


	iRc = EMVSetPCIModeParam(1, (uchar *)"4,5,6,7,8,9,10,11,12", 120000);
	LoadEmvDefault();
	AppSetMckParam(false);
}



// process for the first run
static char bFirstRun = 1;
void FirstRunProc()
{
	logTrace(__func__);
	initGUI();
	Gui_ClearScr();
	//Gui_DrawLogo(sXpress_bg, 0, 0);
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, _T("INITIALIZING APP"), gl_stCenterAttrAlt, GUI_BUTTON_NONE, 0, NULL);

	if (bFirstRun)
	{
		if (ExistSysFiles())
		{
			logTrace("System files exist");
			if (ValidSysFiles())
			{
				logTrace("System files are valid");
				LoadSysParam();
				LoadSysCtrlAll();
				LoadPosParams();
				if (!glPosParams.commConfig.pfUpdWaitUI) {
					glPosParams.commConfig.pfUpdWaitUI = DispWaitRspStatus;
					SavePosParams();
				}

				//glPosParams.commConfig = glPosParams.commConfig;
				//SaveSysParam();

				bFirstRun = 0;

				initializeEMV();
				glCommCfg = glPosParams.commConfig;
				initializeComms(0);
				return;
			}
			else
			{
				RemoveSysFiles();
			}
		}

		loadDefaultPosParams();
		LoadEdcDefault();
		InitTranLogFile();
		
		bFirstRun = 0;
	}


	initializeEMV();	// Init EMV kernel
	initializeComms(1);
	debugKeyInject();
}



int initializeComms(ushort uiShowSetup) {
	logTrace(__func__);
	int ret = 0;
	
	logTrace("uiShowSetup: %d", uiShowSetup);
	showNonModalDialog(GetCurrTitle(), "INITIALIZING COMMS");
	//Fix to enable CLOC info
	WlSelSim(0);
	ret = WlOpenPort();
	ret = WlSendCmd("AT+LBSSTART\r", NULL, 0, 3000, 100);
	WlClosePort();

	ret = SetNetworkCommDetails(&glPosParams.commConfig.ucCommType, uiShowSetup);
	if (ret != 0)
	{
		showCommError(ret);
		return -1;
	}

	DispDial();
	ret = CommDial(DM_PREDIAL);
	//DelayMs(2000);//give comms time to start up
	logTrace("CommDial ret::%d", ret);
	if (0 != ret) {
		showCommError(ret);
		return ret;
	}
	
	return 0;
}