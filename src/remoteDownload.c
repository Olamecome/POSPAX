#include "global.h"
#include "xui.h"
#include "http_handler.h"
#include "parsonHelper.h"
#include "file_handler.h"


#define APP_FILE "XPRESSPOS.bin"


extern const APPINFO AppInfo;
extern int httpTimeOutSec;
extern int downloadBinaryWithSocket();

int xpressCallHome();
static int downloadBinary();


void doRemoteDownload() {
	//downloadBinary();
	downloadBinaryWithSocket();
}

int startUpNewUpdateCheckAndInstall() {
	int ret = xpressCallHome();

	if (ret <= 0) {
		return 0;
	}

	if (ret == 1) {
		//new available
		if (showConfirmDialog("Remote Download", "Updates Available", "Install new update?", 1, 30) == 0) {
			return downloadBinaryWithSocket();
			//downloadBinary();
		}
	}

	return 0;
}

int doXpressCallHomeAndUpdateCheck() {
	int ret = xpressCallHome();

	if (ret <= 0) {
		return 0;
	}

	if (ret == 1) {
		//new available
	
		showInfo("REMOTE DOWNLOAD", 3, 1, "New updates available");
		return downloadBinary();
	}

	return 0;
}


int xpressCallHome() {
	int ret = -1;

	char url[100] = "\0";
	snprintf(url, sizeof(url), "%s://%s:%s/PostCallHomeData", glPosParams.tmsProtocolFlag ? "https" : "http",
		glPosParams.tmsIp.szIP, glPosParams.tmsIp.szPort);
	const char* headers[3] = { "Content-Type: application/json",
		"Accept: application/json",  "apiKey: A1b2C3D4e5f6G7h8I9j0K" };

	char terminalSerial[20] = "\0";
	ReadSN(terminalSerial);

	char cellInfo[30] = { 0 };
	WlInfo_T wlInfo = { 0 };
	if ((get_wl_info(&wlInfo) == 0) && wlInfo.CellInfo.gsm) {
		WlGSMCellInfo_T* cInfo = wlInfo.CellInfo.gsm;
		char signalDesc[30] = { 0 };
		int signalStrength = GetSignal_Status(signalDesc);
		//621,60,24892,1212, 80 => mcc, mnc, lac, ci, ss
		snprintf(cellInfo, lengthOf(cellInfo), "%s,%s,%s,%s,%d", cInfo->mcc,
			cInfo->mnc, cInfo->lac, cInfo->cell, signalStrength);
	}
	logd(("CELL INFO: %s", cellInfo));


	char *powerStatus = NULL;
	char batteryStatus[15] = "\0";
	int batt = BatteryCheck();

	switch (batt) {
	case 0:
		strcpy(batteryStatus, "Battery low");
		break;
	case 1:
		strcpy(batteryStatus, "25%");
		break;
	case 2:
		strcpy(batteryStatus, "50%");
		break;
	case 3:
		strcpy(batteryStatus, "75%");
		break;
	case 4:
		strcpy(batteryStatus, "100%");
		break;
	case 5:
		strcpy(batteryStatus, "Charging");
		break;
	case 6:
		strcpy(batteryStatus, "Fully Charged");
		break;
	}

	if ((batt >= 0) && (batt <= 4))
	{
		powerStatus = "ON";
	}
	else if ((batt == 5) || (batt == 6)) {
		powerStatus = "Charging";
	}


	char* printerStatus = NULL;

	switch (PrnStatus()) {
	case 0:
		printerStatus = "OK";
		break;
	case ERR_PRN_PAPEROUT:
		printerStatus = "No Paper";
		break;
	case ERR_PRN_BUSY:
		printerStatus = "Busy";
		break;
	case ERR_PRN_OVERHEAT:
		printerStatus = "Overheat";
		break;
	default:
		printerStatus = "Error";
	}

	JsonValue rootValue = json_value_init_object();
	Json json = json_value_get_object(rootValue);

	jsonPutString(json, "SerialNumber", terminalSerial);
	jsonPutString(json, "TerminalId", glPosParams.terminalId);
	jsonPutString(json, "Cloc", cellInfo);
	jsonPutString(json, "GpsLocation", "Not Available");
	jsonPutString(json, "PowerStatus", powerStatus);
	jsonPutString(json, "BatteryStatus", batteryStatus);
	jsonPutString(json, "PrinterStatus", printerStatus);
	jsonPutString(json, "AppId", AppInfo.AID);
	jsonPutString(json, "AppName", AppInfo.AppName);
	jsonPutString(json, "AppVersion", AppInfo.AppVer);

	char* request =  json_serialize_to_string(rootValue);
	logTrace("Request: %s", request);

	MemoryStruct chunk = { 0 };
	ret = sendHttpRequest(HTTP_POST, url, request, strlen(request), headers, 3, &chunk);

	json_free_serialized_string(request);
	json_value_free(rootValue);

	if (ret != 0) {
		if (chunk.memory) {
			free(chunk.memory);
		}
		return -1;
	}

	logTrace("Response: %s", chunk.memory);

	rootValue = json_parse_string(chunk.memory);
	if (!rootValue || (json_value_get_type(rootValue) != JSONObject)) {
		showErrorDialog("Invalid response received", 10);
		free(chunk.memory);
		return -1;
	}
	
	json = json_value_get_object(rootValue);

	char responseCode[2 + 1] = "\0";
	getJsonString(json, "ResponseCode", responseCode, sizeof(responseCode));

	if (!isSuccessResponse(responseCode)) {
		char responseMessage[100] = "\0";
		getJsonString(json, "ResponseMessage", responseMessage, sizeof(responseMessage));
		showErrorDialog(responseMessage, 10);
		json_value_free(rootValue);
		free(chunk.memory);
		return -1;
	}

	uchar hasUpdate = getJsonInt(json, "UpdateExist");
	json_value_free(rootValue);
	free(chunk.memory);

	return hasUpdate;
}


static int deleteApp() {
	int ret = DelAppFile(AppInfo.AppName);
	if (ret < 0) {
		logTrace("Delete app error: %d", ret);
	}

	return ret;
}

int processDownloadedFile(char* data, size_t size) {
	logTrace("File size: %d kb", size / 1000);

	if (0 != saveFileData(APP_FILE, (void*)data, size)) {
		showErrorDialog("Error saving file", TEN_SECONDS);
		return -1;
	}

	showInfo("Remote Download", 1, 1, "Download complete");

	showNonModalDialog("Installing App", "Please wait");

	int ret = FileToApp(APP_FILE);
	if (ret != 0) {
		logTrace("App Load Failed: %d", ret);
		char* error;
		switch (ret) {
		case -1:
			error = "App info error";
			break;
		case -2:
			error = "File not found";
			break;
		case -3:
			error = "Invalid signature";
			break;
		case -4:
			error = "App limit exceeded";
			break;
		case -5:
			error = "App mgr with sub-app name";
			break;
		case -6:
			error = "Invalid app type";
			break;
		case -7:
			error = "Write file error";
			break;
		case -8:
			error = "File size too small";
			break;
		case -14:
			error = "File size limit exceeded";
			break;
		case -15:
			error = "Not enough memory";
			break;
		default:
			error = "Error installing app";
		}

		showErrorDialog(error, TEN_SECONDS);
		return -1;
	}

	showMessageDialog("Remote Download", "Update installed", 3, 60);
	return 0;
}


int downloadBinary() {
	bool appCleared = false;

	char url[100] = "\0";
	snprintf(url, sizeof(url), "%s://%s:%s/RemoteDownload", glPosParams.tmsProtocolFlag ? "https" : "http", glPosParams.tmsIp.szIP, glPosParams.tmsIp.szPort);
	const char* headers[3] = { "Content-Type: application/json",
		"Accept: */*",  "apiKey: A1b2C3D4e5f6G7h8I9j0K" };

	JsonValue rootValue = json_value_init_object();
	Json json = json_value_get_object(rootValue);

	jsonPutString(json, "AppId", AppInfo.AID);
	jsonPutString(json, "AppVersion", AppInfo.AppVer);

	char* request = json_serialize_to_string(rootValue);
	logTrace("Request: %s", request);

	MemoryStruct chunk = { 0 };
	httpTimeOutSec = (60L * 5);
	int ret = sendHttpRequest(HTTP_POST, url, request, strlen(request), headers, 3, &chunk);

	json_free_serialized_string(request);
	json_value_free(rootValue);

	if (ret != 0) {
		if (chunk.memory) {
			free(chunk.memory);
		}
		return -1;
	}

	if (chunk.size <= 0) {
		if (chunk.memory) {
			free(chunk.memory);
		}

		showMessageDialog("Remote Download", "Update not available", 1, 10);
		return -1;
	}

	if (showConfirmDialog(NULL, "Remote Download", "Delete all App data?", 1, 10) == 0) {
		appCleared = true;
		deleteApp();
	}

	ret = processDownloadedFile(chunk.memory, chunk.size);
	free(chunk.memory);

	if (ret == 0) {
		showMessageDialog("Remote Download", "Reboot required", 1, 10);
		Reboot();
	}

	return ret;
}
