
#include "global.h"
#include "terminalMgt.h"
#include "xui.h"
#include "printHelper.h"
#include "http_handler.h"


/********************** Internal macros declaration ************************/
#define TIMER_TEMPORARY		4
#define TIMERCNT_MAX		48000
#define TIMER_CALLHOME      1

/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static void  SetIdleTimer(void);
static uchar ChkIdleTimer(int iSeconds);

static int initializeApp(void);

static int showMainMenu(int* selItem);
static void  adminMenu(void);
/********************** Internal variables declaration *********************/

/********************** external reference declaration *********************/
extern int event_main(ST_EVENT_MSG *pstEventMsg);
extern int route(int selection);
extern void prepTerminal(void);
extern void FirstRunProc();
extern int doNibssCallHome();
extern int repushTransactions(char silent);

const APPINFO AppInfo =
{
	APP_NAME,
	EDCAPP_AID,
	EDC_VER_INTERN _TERMTYPE_,
	"XPRESS PAYMENTS",
	"XPRESSPOS",
	"",
	0xFF,
	0xFF,
	0x01,
	""
};

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

int event_main(ST_EVENT_MSG *msg)
{
	SystemInit();
	return 0;
}

int main(void)
{
	initializeApp();

	logTrace("Before Main Loop!");

	//Set Call home timer 
	if (glPosParams.callHomeTimeMinutes > 0) {
		resetCallHomeTimer();
	}

	if (glPosParams.tranRecordCount > 0) {
		repushTransactions(TRUE);
	}

	int res = -1;
	int selItem = 0;
	while (1)
	{
		if (0 == (res = showMainMenu(&selItem))) {
			// ensure that after completing a route processing, it can't immediately go to call home
			continue;
		}

		if (KEYMENU == res || FNKEYMENU == res) {
			logTrace("should show admin menu");
			if (requestAdminPassword(USER_OPER_TIMEOUT) == 0) {
				adminMenu();
			}
			
			continue;
		}
		
		res = TimerCheck(TIMER_CALLHOME);
		logTrace("Call home timercheck: %d", res);
		if (!res) {
			logTrace("Calling home");
			doNibssCallHome();
			DispMessage("Checking notifications");
			repushTransactions(TRUE);
			resetCallHomeTimer();
		}


		SysIdle();
	}


	return 0;
}

int showMainMenu(int* selItem) {
	GUI_MENU menu;
	GUI_MENUITEM menuItems[] = {
		{ "Purchase", PURCHASE, true, NULL },
		{ "Pre-Authorisation", POS_PRE_AUTHORIZATION, true, NULL },
		{ "Sales Completion", POS_PRE_AUTH_COMPLETION, true, NULL },
		{ "Services", REPORTING, true, NULL },
		{ "\0", -1, false, NULL }
	};

	Gui_BindMenu(AppInfo.AppName, gl_stTitleAttr, gl_stLeftAttr, menuItems, &menu);
	Gui_ClearScr();
	int res = Gui_ShowMenuListWithoutButtons(&menu, 0, 3 * 60, selItem);
	if (GUI_OK != res) {
		return res;
	}
	route(*selItem);
	return 0;
}

void  adminMenu(void) {
	logTrace("In %s", __func__);
	GUI_MENU menu;
	GUI_MENUITEM menuItems[] = {
		{ "Terminal Info", TERMINAL_ID_CONFIG, true, terminalInfoMenu },
		{ "XMS Config", TMS_CONN_CONFIG, true, tmsConfigMenu },
		{ "Menu Download", TMS_MENU_DOWNLOAD, true, downloadMenu },
		{ "Network Parameters", COMM_SETTINGS, true,  networkConfig },
		{ "Prep Terminal", TERMINAL_NIBSS_KEY_EXCHANGE, true, prepTerminal },
		{ "Print Terminal Config", COM_PARAM_PRINT, true, printTerminalDetails },
		{ "\0", -1, false, NULL }
	};

	int selected = 0;
	SetCurrTitle("ADMIN MENU");
	Gui_BindMenu("ADMIN MENU", gl_stTitleAttr, gl_stLeftAttr, menuItems, &menu);

	while (1) {
		Gui_ClearScr();
		if (0 != Gui_ShowMenuList(&menu, GUI_MENU_DIRECT_RETURN, 60, &selected)) {
			break;
		}
	}

	Gui_ClearScr();
}

// 设置空闲计时。设置一个比较长的倒计时，以用于不止一种的空闲事件处理
// set a idle timer with a long period of time, for processing several idle events
void SetIdleTimer(void)
{
	TimerSet(TIMER_TEMPORARY, TIMERCNT_MAX);
}

void resetCallHomeTimer() {
	TimerSet(TIMER_CALLHOME, ((glPosParams.callHomeTimeMinutes * 60 * 1000) / 10));
}

// 检查空闲计时，看是否已经流过了指定的分钟数
// check if the timer counted the specific time(uint:minute)
uchar ChkIdleTimer(int iSeconds)
{
	logTrace(__func__);
	int	iCnt = TIMERCNT_MAX-TimerCheck(TIMER_TEMPORARY);
	
	PubASSERT(TIMERCNT_MAX > iSeconds*10);	//	ScrPrint(0,7,ASCII,"%d  ", iCnt/10);
	return (iCnt >= iSeconds*10);
}

int initializeApp() {
	logTrace(__func__);

	SystemInit();
	SetOffBase(NULL);

	FirstRunProc();
	LoadEdcLang();
	return 0;
}

// end of file