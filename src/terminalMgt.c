#include "terminalMgt.h"
#include "xui.h"
#include "http_handler.h"
#include "parsonHelper.h"
#include "printHelper.h"

extern int printTerminalDetails();
extern void SetSysCommParam(uchar ucPermission);

void terminalInfoMenu() {
	char terminalId[8 + 1] = "\0";
	char merchantId[15 + 1] = "\0";

	strcpy(terminalId, glPosParams.terminalId);
	strcpy(merchantId, glPosParams.merchantId);

	GUI_INPUTBOX_ATTR inputAttr = { 0 };
	inputAttr.bEchoMode = true;
	inputAttr.nMinLen = 8;
	inputAttr.nMaxLen = 8;
	inputAttr.eType = GUI_INPUT_MIX;

	clearScreen();
	if (GUI_OK != Gui_ShowInputBox("Terminal ID", gl_stTitleAttr, "Enter TID", gl_stLeftAttr, terminalId, gl_stCenterAttr, &inputAttr, 60)) {
		return;
	}

	inputAttr.bEchoMode = false;
	inputAttr.nMinLen = 0;
	inputAttr.nMaxLen = 15;

	clearScreen();
	Gui_ShowInputBox("Merchant ID", gl_stTitleAttr, "Enter  MID", gl_stLeftAttr, merchantId, gl_stCenterAttr, &inputAttr, 60);

	strcpy(glPosParams.terminalId, terminalId);
	strcpy(glPosParams.merchantId, merchantId);
	SavePosParams();

	showMessageDialog("Terminal Info", "Config saved", 1, 1000);
}


void tmsConfigMenu() {
	SetCurrTitle("TMS Config");
	int selected = 0;

	while (1) {
		if (GUI_OK != GetIPAddress("TMS IP", false, glPosParams.tmsIp.szIP)) {
			return;
		}

ENTER_PORT:
		if (GUI_OK != GetIPPort("TMS Port", false, glPosParams.tmsIp.szPort)) {
			continue;
		}

		clearScreen();
		selected = glPosParams.tmsProtocolFlag;
		if (GUI_OK != Gui_ShowAlternative("TMS HTTP Protocol", gl_stTitleAttr,
			"Select TMS protocol", gl_stCenterAttr, "Http", 0, "Https", 1, 30, &selected)) {
			goto ENTER_PORT;
		}
		glPosParams.tmsProtocolFlag = selected;
		break;
	}
	SavePosParams();
	showMessageDialog("TMS Config", "Config saved", 1, 10);
}

static void setNibssConfig() {
	int selected = 0;
	char param[20] = "\0";

	GUI_INPUTBOX_ATTR inputAttr = { 0 };
	inputAttr.bEchoMode = true;
	inputAttr.nMinLen = 2;
	inputAttr.nMaxLen = 3;
	inputAttr.eType = GUI_INPUT_NUM;

	while (1) {
		if (GUI_OK != GetIPAddress("Host IP", false, glPosParams.commConfig.stWirlessPara.stHost1.szIP)) {
			return;
		}

ENTER_PORT:
		if (GUI_OK != GetIPPort("Host Port", false, glPosParams.commConfig.stWirlessPara.stHost1.szPort)) {
			continue;
		}


		clearScreen();
		selected = glPosParams.switchPortFlag;
		if (GUI_OK != Gui_ShowAlternative("Connection Type", gl_stTitleAttr,
			"Select Connection Type", gl_stLeftAttr, "Open", 0, "SSL", 1, 30, &selected)) {
			goto ENTER_PORT;
		}
		glPosParams.switchPortFlag = selected;

		memset(param, 0, lengthOf(param));
		sprintf(param, "%d", glPosParams.requestTimeOutSec);
		clearScreen();
		if (GUI_OK != Gui_ShowInputBox("Request Timeout", gl_stTitleAttr, "Request Timeout(seconds)", gl_stLeftAttr, param, gl_stCenterAttr, &inputAttr, 60)) {
			goto ENTER_PORT;
		}
		glPosParams.requestTimeOutSec = atoi(param);

		memset(param, 0, lengthOf(param));
		sprintf(param, "%d", glPosParams.callHomeTimeMinutes);
		clearScreen();
#ifdef APP_DEBUG
	inputAttr.nMinLen = 1;
#endif // APP_DEBUG
		
		if (GUI_OK != Gui_ShowInputBox("Call Home Time", gl_stTitleAttr, "Call Home Time(minutes)", gl_stLeftAttr, param, gl_stCenterAttr, &inputAttr, 60)) {
			goto ENTER_PORT;
		}
		glPosParams.callHomeTimeMinutes = atoi(param);


		break;
	}

	memset(param, 0, lengthOf(param));
	sprintf(param, "%d", glPosParams.switchPortFlag);
	PutEnv("E_SSL", param);

	SavePosParams();
	glCommCfg = glPosParams.commConfig;
	CommSetCfgParam(&glCommCfg);

	resetCallHomeTimer();
	showMessageDialog("Host Parameters", "Saved", 1, 10);
}

void networkConfig() {
	GUI_MENU menu;
	GUI_MENUITEM menuItems[] = {
		{ "Host Parameters", 0, true, NULL },
		{ "Comm Parameters", 1, true, NULL },
		{ "\0", -1, false, NULL }
	};
	Gui_BindMenu("NETWORK PARAMS", gl_stTitleAttr, gl_stLeftAttr, menuItems, &menu);

	int selected = 0;
	while (1) {
		Gui_ClearScr();
		if (0 != Gui_ShowMenuList(&menu, GUI_MENU_DIRECT_RETURN, 60, &selected)) {
			break;
		}

		switch (selected) {
		case 0:
			setNibssConfig();
			break;
		case 1: 
			SetSysCommParam(1);
			break;
		}
	}

	Gui_ClearScr();
}


void downloadMenu() {
	if (strlen(glPosParams.terminalId) != 8) {
		showErrorDialog("Terminal ID not set", DEFAULT_PASSWORD_TIMEOUT);
		return;
	}
	
	char* protocol = glPosParams.tmsProtocolFlag == 0 ? "http" : "https";
	char terminalSerial[20] = "\0";
	ReadSN(terminalSerial);
	char* tmsPath = "/terminalConfiguration";

	char url[100] = "\0";
	sprintf(url, "%s://%s:%s%s?tid=%s&sn=%s", protocol, glPosParams.tmsIp.szIP, 
		glPosParams.tmsIp.szPort, tmsPath, glPosParams.terminalId, terminalSerial);
	logTrace("TMS Url: %d", url);
	
	MemoryStruct chunk = { 0 };
	int ret = sendHttpRequest(HTTP_GET, url, NULL, 0, NULL, 0, &chunk);
	if (ret != 0) {
		if (chunk.memory) {
			free(chunk.memory);
		}
		return;
	}
	logd(("Response received"));

	if (chunk.size <= 0) {
		showErrorDialog("Invalid Response Received", 10);
		if (chunk.memory) {
			free(chunk.memory);
		}
		return;
	}

	logDirect("Response", chunk.memory);

	JSON_Value* root = json_parse_string(chunk.memory);
	logd(("parsed root"));
	if (!root || (json_value_get_type(root) != JSONObject)) {
		showErrorDialog("Invalid Response Received", 10);
		free(chunk.memory);
		return;
	}

	free(chunk.memory);
	logd(("freed chunk.memory"));

	JSON_Object* json = json_value_get_object(root);
	logd(("Got base json object"));
	if (!json_object_dothas_value(json, "response.ResponseCode")) {
		showErrorDialog("Invalid Response Received", 10);
		json_value_free(root);
		return;
	}

	const char* responseCode = json_object_dotget_string(json, "response.ResponseCode");
	logd(("Response Code: %s", responseCode));

	if (!isSuccessResponse(responseCode)) {
		char responseMessage[50];
		showErrorDialog(json_object_dotget_string(json, "response.ResponseMessage"), 10);
		json_value_free(root);
		return;
	}

	if (!json_object_has_value(json, "HostKeys") || !json_object_dothas_value(json, "HostKeys.CombinedKey"))  {
		showErrorDialog("Host key not configured", 10);
		json_value_free(root);
		return;
	}

	getDotJsonString(json, "HostKeys.CombinedKey", glPosParams.hostZMK, lengthOf(glPosParams.hostZMK));
	getJsonString(json, "MerchantID",glPosParams.merchantId, lengthOf(glPosParams.merchantId));
	getJsonString(json, "MerchantName", glPosParams.merchantName, lengthOf(glPosParams.merchantName));

	if (json_object_dothas_value(json, "SimConfig.APN")) {
		getDotJsonString(json, "SimConfig.APN",
			glPosParams.commConfig.stWirlessPara.szAPN, lengthOf(glPosParams.commConfig.stWirlessPara.szAPN));
	}


	IP_ADDR ipInfo = { 0 };
	getJsonString(json, "IpAddress", ipInfo.szIP, lengthOf(ipInfo.szIP));
	getJsonString(json, "Port", ipInfo.szPort, lengthOf(ipInfo.szPort));

	glPosParams.commConfig.stTcpIpPara.stHost1 = ipInfo;
	glPosParams.commConfig.stWifiPara.stHost1 = ipInfo;
	glPosParams.commConfig.stWirlessPara.stHost1 = ipInfo;

	glPosParams.switchPortFlag = 1;
	PutEnv("E_SSL", "1");

	getJsonString(json, "Username", glPosParams.username, lengthOf(glPosParams.username));
	getJsonString(json, "Password", glPosParams.password, lengthOf(glPosParams.password));
	getJsonString(json, "Location", glPosParams.merchantLocation, lengthOf(glPosParams.merchantLocation));
	getJsonString(json, "InstitutionCode", glPosParams.institutionCode, lengthOf(glPosParams.institutionCode));
	getJsonString(json, "ConsultantCode", glPosParams.consultantCode, lengthOf(glPosParams.consultantCode));
	getJsonString(json, "receiptHeader", glPosParams.slipHeader, lengthOf(glPosParams.slipHeader));
	getJsonString(json, "receiptFooter", glPosParams.slipFooter, lengthOf(glPosParams.slipFooter));

	if (json_object_has_value(json, "logo")) {
		const char* logo = json_object_get_string(json, "logo");

		if (!strstr(logo, "Cannot")) {
			logDirect("Logo:\n", logo);
			int len = strlen(logo) / 2;
			uchar* logoBmp = malloc(len);
			memset(logoBmp, 0, len * sizeof(uchar));
			PubAsc2Bcd(logo, strlen(logo), logoBmp);
			if (0 == saveLogoBmpFile(logoBmp, len)) {
				logd(("Logo saved"));
			}
			else {
				logd(("Logo not saved"));
			}
		}
		else {
			logTrace("No logo returned");
		}
	}

	//jsonFree(json);
	json_value_free(root);

	SavePosParams();
	glCommCfg = glPosParams.commConfig;
	CommSetCfgParam(&glCommCfg);	
	DispPrinting();
	CommDial(DM_PREDIAL);
	printTerminalDetails();
	showInfo("Menu Download", 10, 1, "Download Successful");
}
