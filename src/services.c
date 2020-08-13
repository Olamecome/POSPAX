#include "global.h"
#include "xui.h"
#include "records.h"
#include "http_handler.h"

extern int doNibssEod();
extern void doRemoteDownload();

static void checkConnection() {
	char* url = "http://www.google.com";

	MemoryStruct chunk = { 0 };
	int ret = sendHttpRequest(HTTP_GET, url, NULL, 0, NULL, 0, &chunk);
	if (chunk.memory) {
		free(chunk.memory);
	}

	if (ret == 0) {
		//logTrace("Data Received:\n%s\nreceived length: %d", receivedData, reclen);
		PubBeepOk();
		showMessageDialog("Check Connection", "Connection OK", 1, 10);
	}
	else {
		DispErrMsg("Connection Error", NULL, 10, DERR_BEEP);
	}


	/*char params[5] = { 0 };
	char* receivedData = NULL;
	int reclen = 0;
	char temp[100] = { 0 };

	int ret = -1;

	char Debug[2];
	Debug[0] = 1; 
	Debug[1] = DEBUG_PRI_CHAR;   
	HttpParaCtl(0, HTTP_CMD_SET_DEBUG, Debug, 2);

	DispDial();
	if (0 != (ret = CommDial(DM_DIAL))) {
		showCommError(ret);
		return;
	}
	logd(("Dialing successful"));

	int sockfd = HttpCreate();
	if (sockfd < 0) {
		DispErrMsg("Check Connection", "HTTP library init failed", 10, DERR_BEEP);
		return;
	}
	logd(("socfd=%d", sockfd));
	
	memset(params, 0, lengthOf(params));
	params[0] = PROTO_HTTP;
	HttpParaCtl(sockfd, HTTP_CMD_SET_PROTO, params, 1);

	memset(params, 0, lengthOf(params));
	strcat(params, "60");
	HttpParaCtl(sockfd, HTTP_CMD_SET_TIMEOUT, params, strlen(params));

	memset(params, 0, lengthOf(params));
	params[0] = CERT_ISSUER;
	params[1] = CERT_TIME;
	params[2] = CERT_BAD;
	params[3] = CERT_SELF_SIGN;
	params[4] = CERT_CA_ISSUER;
	HttpParaCtl(sockfd, HTTP_CMD_SET_CERT_REASON, params, 5);
	
	DispSend();
	int sl = HttpGet(sockfd, url);  
	if (sl<0) { 
		logd(("HTTP Send GET failed =%d\n", sl));   
		showCommError(sl);
		CommOnHook(FALSE);
		HttpClose(sockfd);   
		return; 
	} else {
		logd(("HTTP Send GET status = %d\n", sl));
	}

	DispReceive();
	logTrace("receiving\n");

	while (1) {
		sl = HttpRecvContent(sockfd, temp, lengthOf(temp)-1);
		if (sl>0) {    
			//BuffDump(RecvBuf, sl, NULL, PRICHAR);
			//logd(("Received\n%s\nLength: %d", temp, sl));
			receivedData = realloc(receivedData, (reclen + sl + 1) * sizeof(char));
			memset(receivedData + reclen, 0, (sl + 1) * sizeof(char));
			memcpy(receivedData + reclen, temp, sl);
			reclen += sl;
			receivedData[reclen + sl + 1] = '\0';
		} else if(sl==ERR_HTTP_NO_CONTENT) {    
			logd(("EOF")); 
			break;
		} else {    
			logd(("recv failed\n"));
			DispErrMsg("Check Connection", "Connection Failed", 10, DERR_BEEP);
			if (receivedData) {
				free(receivedData);
			}
			CommOnHook(FALSE);
			HttpClose(sockfd);
			return;   
		}  
	}

	if (receivedData && reclen > 0) {
		//logTrace("Data Received:\n%s\nreceived length: %d", receivedData, reclen);
		showMessageDialog(GetCurrTitle(), "Connection OK", 1, 10);
	} else {
		DispErrMsg("Check Connection", "No response received", 10, DERR_BEEP);
	}

	if (receivedData) {
		free(receivedData);
	}

	CommOnHook(FALSE);
	HttpClose(sockfd);
	return;*/

}

static void reprint() {
	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	Prompt prompt = { 0 };
	getListItemPrompt(&prompt, "REPRINT", "Last|By STAN|By RRN|By Seq No|All");
	prompt.selectionOption = 0;

	while (1) {
		if (showPrompt(&prompt) != 0) {
			return;
		}

		int ret = requestSupervisorPassword(USER_OPER_TIMEOUT);
		if (ret != 0) {
			if (ret == APP_FAIL) {
				showErrorDialog("Access Denied", 10);
			}
			return;
		}

		Gui_ClearScr();
		switch (prompt.selectionOption) {
		case 0:
			reprintLast();
			break;
		case 1:
			reprintSTAN();
			break;
		case 2:
			reprintRRN();
			break;
		case 3:
			reprintSequenceNumber();
			break;
		case 4:
			reprintAll();
			break;
		}
	}
}

static void summaryReport() {
	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	Prompt prompt = { 0 };
	getListItemPrompt(&prompt, "SUMMARY REPORT", "Daily|Weekly|Monthly|Yearly");

	while (1) {
		prompt.selectionOption = 0;
		if (showPrompt(&prompt) != 0) {
			return;
		}

		int ret = requestSupervisorPassword(USER_OPER_TIMEOUT);
		if (ret != 0) {
			if (ret == APP_FAIL) {
				showErrorDialog("Access Denied", 10);
			}
			return;
		}

		Gui_ClearScr();
		switch (prompt.selectionOption) {
		case 0:
			dailySummary();
			break;
		case 1:
			weeklySummary();
			break;
		case 2:
			monthlySummary();
			break;
		case 3:
			yearlySummary();
			break;
		}
	}

}

static void endOfDay() {
	if (glPosParams.tranRecordCount <= 0) {
		showErrorDialog("No transaction found", USER_OPER_TIMEOUT);
		return -1;
	}

	Prompt prompt = { 0 };
	getListItemPrompt(&prompt, "EOD", "Today|Yesterday|Enter Date|Close Batch");
	prompt.selectionOption = 0;

	if (repushTransactions(TRUE) != 0) {
		return;
	}

	while (1) {
		if (showPrompt(&prompt) != 0) {
			return;
		}

		int ret = requestSupervisorPassword(USER_OPER_TIMEOUT);
		if (ret != 0) {
			if (ret == APP_FAIL) {
				showErrorDialog("Access Denied", 10);
			}
			return;
		}

		switch (prompt.selectionOption) {
		case 3:
			doNibssEod();
			closeBatch();
			break;
		default:
			processEOD(prompt.selectionOption);
		}
		
	}

}


int servicesMenu() {
	Prompt prompt = { 0 };
	getListItemPrompt(&prompt, "SERVICES", "Check Connection|Reprint|Summary Report|End of Day|Repush Transactions|Remote Download");
	prompt.selectionOption = 0;

	while (1) {
		if (showPrompt(&prompt) != 0) {
			return-1;
		}
		Gui_ClearScr();
		switch (prompt.selectionOption) {
		case 0:
			checkConnection();
			break;
		case 1:
			reprint();
			break;
		case 2:
			summaryReport();
			break;
		case 3:
			endOfDay();
			break;
		case 4:
			repushTransactions(FALSE);
			break;
		case 5:
			doRemoteDownload();
			break;
		}

	}

	return 0;
}