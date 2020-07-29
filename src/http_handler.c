#include "http_handler.h"
#include "xui.h"



int httpTimeOutSec = 60;

static int dialOnly() {
	//logTrace(__func__);
	if (glPosParams.commConfig.ucCommType == CT_GPRS || glPosParams.commConfig.ucCommType == CT_CDMA
		|| glPosParams.commConfig.ucCommType == CT_WCDMA) {

		WlInit(NULL);
		RouteSetDefault(11);

		int ret = WlPppLoginEx("*99***1#",
			glPosParams.commConfig.stWirlessPara.szAPN,
			glPosParams.commConfig.stWirlessPara.szUID,
			glPosParams.commConfig.stWirlessPara.szPwd,
			0xFF, 1000 * 60, 10);

		//logTrace("ret::%d", ret);
		if (ret != 0) {
			if (ret == -226 /*Dialing*/) {
				return dialOnly();//
			}
			else {
				return CommDial(DM_PREDIAL);
			}
		}
	}
	else {
		return CommDial(DM_PREDIAL);
	}

	return 0;
}


/**
* @param httpMethod
* @param hostURL
* @param postData
* @param post_data_len
* @param headers
* @param header_len
* @param chunk [out] @brief remember to always call free(chunk.memory);
* @return
*/
int sendHttpRequest(uchar httpMethod, const char* hostURL, const char* postData, size_t post_data_len, const char** headers, size_t header_len, MemoryStruct* chunk) {
	char params[100] = { 0 };
	char temp[1024] = { 0 };

	int ret = -1;

#ifdef _DEBUG//APP_DEBUG
	char Debug[2];
	Debug[0] = 1;
	Debug[1] = DEBUG_PRI_CHAR;
	HttpParaCtl(0, HTTP_CMD_SET_DEBUG, Debug, 2);
#endif // APP_DEBUG
	
	int dial_mode = DM_PREDIAL;
	DispDial();
	ret = dialOnly(); 
	if (ret != 0)
	{
		//Retry
		DispDial();
		if (0 != (ret = CommDial(dial_mode))) {
			DispCommErrMsg(ret);
			logTrace("CommDial failed");
			CommOnHook(FALSE);
			return ret;
		}
	}
	logd(("Dialing successful"));

	int sockfd = HttpCreate();
	if (sockfd < 0) {
		DispErrMsg(GetCurrTitle(), "HTTP library init failed", 10, DERR_BEEP);
		return -1;
	}
	logd(("socfd=%d", sockfd));


	memset(params, 0, lengthOf(params));
	params[0] =  (NULL != strstr(hostURL, "https://")) ? PROTO_HTTPS : PROTO_HTTP;
	HttpParaCtl(sockfd, HTTP_CMD_SET_PROTO, params, 1);

	memset(params, 0, lengthOf(params));
	sprintf(params, "%d", httpTimeOutSec);
	//strcat(params, "90"); //1.5 minutes	
	HttpParaCtl(sockfd, HTTP_CMD_SET_TIMEOUT, params, strlen(params));
	httpTimeOutSec = 90; //reset time for latter use.

	memset(params, 0, lengthOf(params));
	params[0] = CERT_ISSUER;
	params[1] = CERT_TIME;
	params[2] = CERT_BAD;
	params[3] = CERT_SELF_SIGN;
	params[4] = CERT_CA_ISSUER;
	HttpParaCtl(sockfd, HTTP_CMD_SET_CERT_REASON, params, 5);

	if (headers && (header_len > 0)) {
		memset(params, 0, sizeof(params));
		int i = 0;
		for (; i < header_len; i++) {
			strcat(params, headers[i]);
			strcat(params, "\r\n");
		}
		HttpParaCtl(sockfd, HTTP_CMD_ADD_NEW_FIELDS, params, strlen(params));
	}

	DispSend();

	int sl = 0; 
	if (httpMethod == HTTP_GET) {
		sl = HttpGet(sockfd, hostURL);
	} else {

		memset(params, 0, sizeof(params));
		strcpy(params, "application/json");
		HttpParaCtl(sockfd, HTTP_CMD_SET_CONTENT_TYPE, params, strlen(params));

		sl = HttpPost(sockfd, hostURL, postData, post_data_len);
	}

	if (sl<0) {
		logd(("HTTP Send failed = %d", sl));
		showCommError(sl);
		HttpClose(sockfd);
		//WlPppLogout();
		return -1;
	} else {
		logd(("HTTP Send status = %d", sl));
		if (sl >= 400) {
			showErrorDialog("404 - Not found", 30);
			HttpClose(sockfd);
			//WlPppLogout();
			return -1;
		}
	}


	DispReceive();
	logTrace("receiving\n");

	while (1) {
		sl = HttpRecvContent(sockfd, temp, lengthOf(temp) - 1);
		if (sl>0) {
			//logDirect("Received", temp);
			//logd(("Received length: %d", sl));
			chunk->memory = realloc(chunk->memory, (chunk->size + sl + 1) * sizeof(char));
			memset(chunk->memory + chunk->size, 0, (sl + 1) * sizeof(char));
			memcpy(chunk->memory + chunk->size, temp, sl);
			chunk->size += sl;
			chunk->memory[chunk->size + sl + 1] = '\0';
		}
		else if (sl == ERR_HTTP_NO_CONTENT) {
			logd(("EOF"));
			break;
		}
		else {
			logd(("recv failed"));
			DispErrMsg("Request Failed", "Receive Error", 10, DERR_BEEP);
			HttpClose(sockfd);
			//WlPppLogout();
			return -1;
		}
	}

	HttpClose(sockfd);
	//WlPppLogout();
	
	return 0;
}
