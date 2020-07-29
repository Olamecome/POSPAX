#include "global.h"
#include "parsonHelper.h"
#include "xui.h"
#include "utils.h"
#include "commproc.h"

extern int processDownloadedFile(char* data, size_t size);


static char spDispMessage[30 + 1];
static void specialDisplayReceive(int* counter) {
	char* dot = NULL;

	if (*counter <= 1) {
		dot = ".";
	}
	else if (*counter % 2 == 0) {
		dot = "..";
	}
	else {
		dot = "...";
	}
	
	memset(spDispMessage, 0, sizeof(spDispMessage));
	snprintf(spDispMessage, sizeof(spDispMessage), "Receiving%s", dot);
	Gui_ClearScr();
	Gui_DrawText(spDispMessage, gl_stCenterAttr, 0, 40);

	if (*counter % 3 == 0) {
		*counter = 1;
	}
	else {
		(*counter)++;
	}
}

static void adjustTMSComms() {
	COMM_CONFIG config = glCommCfg;
	IP_ADDR ipAddr = glPosParams.tmsIp;

	config.stWirlessPara.stHost1 = ipAddr;
	config.stWirlessPara.stHost2 = ipAddr;

	config.stWifiPara.stHost1 = ipAddr;
	config.stWifiPara.stHost2 = ipAddr;

	config.ucPortMode = glPosParams.tmsProtocolFlag;
	glCommCfg = config;
	CommSetCfgParam(&config);
}


int downloadBinaryWithSocket() {
	char* token = NULL;
	char* tokens[20] = { 0 };
	int len = 0, recvlen = 0;
	char* CACHE_CONTROL = "no-cache";
	const char* CONTENT_LENGTH_HEADER = "Content-Length";
	char data[0x2000] = { 0 };

	char *recvData = NULL;
	int content_len = 0;

	const char* REQUEST_PATH = "/RemoteDownload";  //"/posbridge/post";
	const char* httpMethod = "POST";

	char message[31 + 1] = { 0 };

	char* primaryHeaders[5] = {
		"Cache-Control: no-cache",
		"User-Agent: Terminal",
		"Content-Type: application/json",
		"Accept: */*",
		"apiKey: A1b2C3D4e5f6G7h8I9j0K"
	};
	int pry_header_len = 5;

	adjustTMSComms();
	CommDial(DM_PREDIAL);

	bool appCleared = false;

	JsonValue rootValue = json_value_init_object();
	Json json = json_value_get_object(rootValue);

	jsonPutString(json, "AppId", AppInfo.AID);
	jsonPutString(json, "AppVersion", AppInfo.AppVer);

	char* request = json_serialize_to_string(rootValue);
	logTrace("Request: %s", request);


	sprintf(data, "%s %s HTTP/1.1\r\n", httpMethod, REQUEST_PATH);
	sprintf(data + strlen(data), "Host: %s:%s\r\n", glPosParams.tmsIp.szIP, glPosParams.tmsIp.szPort);

	int i = 0;
	for (; i < pry_header_len; i++) {
		sprintf(data + strlen(data), "%s\r\n", primaryHeaders[i]);
	}

	sprintf(data + strlen(data), "Content-Length: %d\r\n", strlen(request));
	strcat(data, "\r\n");
	strcat(data, request);
	len = strlen(data);

	recvlen = sizeof(recvData);
	
	Gui_ClearScr();
	Gui_DrawText("Connecting", gl_stCenterAttr, 0, 40);
	int ret = dialHost();
	if (ret != 0) {
		DispCommErrMsg(ret);
		goto End;
	}


	Gui_ClearScr();
	Gui_DrawText("Checking for update", gl_stCenterAttr, 0, 40);

	ret = CommTxd(data, len, glPosParams.requestTimeOutSec);
	json_free_serialized_string(request);
	json_value_free(rootValue);

	if (ret != 0) {
		DispCommErrMsg(ret);
		goto End;
	}

	//DispReceive();
	char byyte;
	int total_read = 0;
	int disp_counter = 1;
	while (1)
	{
		specialDisplayReceive(&disp_counter);
		ret = CommRxd(&byyte, 1, glPosParams.requestTimeOutSec, &recvlen);
		if (ret < 0)
		{
			logError( "Receive failed, %d", ret);
			DispCommErrMsg(ret);
			goto End;
		}

		//logTrace("Read %d bytes", recvlen);
		recvData = realloc(recvData, total_read + recvlen);
		recvData[total_read] = byyte;
		total_read += recvlen;

		if (total_read > 10 && byyte == '\n')
		{
			if (memcmp(recvData + (total_read - 4), "\r\n\r\n", 4) == 0)
			{
				char statusCode[3 + 1] = { 0 };
				strncpy(statusCode, recvData + 9, 3);
				statusCode[3] = '\0';
				if (atoi(statusCode) >= 400)
				{
					char message[32 + 1] = "\0";
					if (atoi(statusCode) == 404) {
						snprintf(message, sizeof(message), "Update not available");
					}
					else {
						snprintf(message, sizeof(message), "Http error - %s", statusCode);
					}
					logTrace("Http Error. Status code: %s\n", statusCode);
					showErrorDialog(message, 30);
					goto End;
				}

				int num_tokens = 0;
				i = 0;
				token = strtok(recvData, "\r\n");
				if (token)
				{
					tokens[i] = calloc(strlen(token) + 1, sizeof(char));
					memcpy(tokens[i], token, strlen(token));
					i++;
					while ((token = strtok(NULL, "\r\n")))
					{
						tokens[i] = calloc(strlen(token) + 1, sizeof(char));
						memcpy(tokens[i], token, strlen(token));

						if (strncmp(CONTENT_LENGTH_HEADER, tokens[i], strlen(CONTENT_LENGTH_HEADER)) == 0)
						{
							char *temp = strtok(tokens[i], ": ");
							logTrace("Part 1: %s, ", temp);
							temp = strtok(NULL, ":");
							logTrace("Part 2: %s\n", temp);
							content_len = atoi(temp);
							logTrace("Content length: %d\n", content_len);
							break;
						}
						i++;
					}
				}

				num_tokens = i;
				for (i = 0; i < num_tokens; i++)
				{
					free(tokens[i]);
				}
				break;
			}
		}
	}

	recvData = calloc(content_len, sizeof(char));
	char buffer[0x2000] = { 0 };

	i = 0;
	total_read = 0;


	char termInfo[32] = { 0 };
	GetTermInfo(termInfo);
	uchar isColourScreen = termInfo[19] & 0x02;

	while (total_read < content_len) {

		CLEAR_STRING(message, sizeof(message));
		snprintf(message, sizeof(message), "Downloading %d%s", (total_read * 100)/content_len, !isColourScreen ? "%" : "%%%");
		Gui_ClearScr();
		Gui_DrawText(message, gl_stCenterAttr, 0, 40);

		ret = CommRxd(buffer, sizeof(buffer), glPosParams.requestTimeOutSec, &recvlen);
		// logTrace("RES::%d\n", res);
		if (ret < 0) {
			break;
		}

		//logTrace("Read %d bytes", recvlen);

		memcpy(recvData + total_read, buffer, recvlen);
		total_read += recvlen;
	}

	logTrace("\nTotal Read: %d\n", total_read);
	if (ret < 0 && total_read < content_len)
	{
		logError("Download failed, %d", ret);
		//DispCommErrMsg(ret);
		showErrorDialog("Download failed", 10);
		goto End;
	}
	else
	{
		logTrace("Data received successfully. Received len: %d, expected: %d\n\n", total_read, content_len);
	}

	if (content_len > 0)
	{
		logTrace("Total data len: %d, Content len: %d\n", total_read, content_len);
		processDownloadedFile(recvData, content_len);
		if (ret == 0) {
			showMessageDialog("Remote Download", "Reboot required", 1, 1);
			Reboot();
		}
	}

End:
	if (recvData)
	{
		free(recvData);
	}
	CommOnHook(true);
	resetCommCfg();
	return ret;
}