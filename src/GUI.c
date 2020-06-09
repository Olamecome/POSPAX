/************************************************************************
 0.1  Kim  20140425  init
 0.2  Kim  20140730 fix bugs, modify function Gui_ShowInfoPage, optimize the touchscreen processing
 0.3  Kim  20140806 fix bugs
 0.4  Kim  20140825 supported Arabic
 0.5  Kim  20140902 fix bug of Gui_ShowInfoPage
 0.6  Kim  20140902 fix bug of Gui_ShowInfoPage
 0.7  Kim  20140911 fix input box "*" flash
 0.8  Kim  20140916 fix a bug of msgbox
 0.9  Kim  20140927 add Gui_ShowMenu2(button style)(only for Knet), add GUI_FONT_OPAQUE
 0.10 Kim  20141124 supports Prolin2.4
 0.11 Kim  20141219 add status icon for up down on Prolin2.4
 0.12 Kim  20150113 fix a bug of right alignment(lost the last character)---temporary solution, it's a Xui bug
 ************************************************************************/
//TODO monitor D210 Touch screen
#include <posapi.h>
#include <posapi_all.h>
#include "GUI.h"
#include "Logger.h"
//#define _AR_MODE_

#ifdef _AR_MODE_
#include "arabic_farsi_api.h"
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define _GetReverseColor(_rgb)      (0xFFFFFFFF - ((_rgb) | 0xFF000000))
#define _IsIncluded(x, y, top, bottom, left, right) (((top) < (y) && (bottom) > (y) && (left) < (x) && (right) > (x)) ? 1 : 0)

#define FLASH_DELAY     50     //ms
#ifndef MAX
#define MAX(a, b)       ((a)>(b) ? (a):(b))
#endif
#define IsValid(value, start, end)      (((value) >= (start)) && ((value) <= (end)))

typedef struct _tagRect {
    int top;
    int left;
    int bottom;
    int right;
    int nValue;
} Rect;

typedef struct _tagRectMap {
    Rect *pRect;
    unsigned int no;
} RectMap;

typedef struct _tagKeyMap {
    int KeyValue;
    char *Table;
	char iProportion_x; // like the space, enter line, proportion of space key may cost 2 units, default is 1
	char iProportion_y;
} KeyMap;

typedef struct _tagKey
{
	KeyMap key[1024];
	int num;
}KEY_T;

typedef struct _tagCallbackEvent
{
	gui_callbacktype_t type;
	GUI_CALLBACK vFunc;
}CallbackEvent;

typedef struct _tagSignPoint
{
	unsigned short x;
	unsigned short y;
} SignPoint;

#define XUI_SIGN_MAX_ARRAY 		(76800)	/* 320*240=76800 */

typedef struct _tagSignData
{
	/* OUT array of point */
	SignPoint point_array[XUI_SIGN_MAX_ARRAY];
	/* OUT valid point count */
	unsigned int point_len;
} SignData;

static void ClearScr(const Rect *rect, char IsClearBg);
static void UpdateCanvasSize();

static void DrawText(const void *vRes, const void *vAttr, int top, int bottom, int left, int right, char isButton);
static void DrawLogo(const void *vRes, int top, int left);
static void DrawImage(const void *vRes, int top, int left);
static void DrawBgBox(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, unsigned int color);
static void DrawRect(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, unsigned int color);
static void DrawSign(const Rect *area);
static void DrawLine(int x1, int y1, int x2, int y2, unsigned int color);

static int GetInput(const unsigned char *pszPrompt, GUI_TEXT_ATTR stPromptAttr, Rect stPromptRect, unsigned char *pszContent, GUI_TEXT_ATTR stContentAttr, Rect stContentRect, const GUI_INPUTBOX_ATTR *pstType, const Rect *pstButtonRect, GUI_TEXT_ATTR stButtonAttr, int nButtonNo, int timeoutSec);

static int GetInputTime(Rect stCurrTimeRect, const Rect *pstTimeRect, GUI_TEXT_ATTR stTimeAttr, unsigned char *pszTime, const Rect *pstButtonRect, unsigned char nButtonsNo, GUI_TEXT_ATTR stButtonAttr, unsigned char isTime, int timeoutSec);

// Modified by Kim_LinHB 2014/10/8 v0.9
static int GetVirtualKey(const RectMap *pRectMap, unsigned int nMapSize);

static int GetMenuItem(const GUI_MENU *pstMenu, enum MENUSTYLE eMode, Rect stMenuRect, const Rect *pstButtonRect, unsigned char nButtonNo, const GUI_TEXT_ATTR *pstButtonAttr, int timeoutSec, int *piSelected);

// Modified by Kim_LinHB 2014-8-8 v0.3
static int GetInfoPage(const GUI_PAGE *pstPage, Rect stInfoRect, const Rect *pstButtonRect, unsigned char nButtonNo, const GUI_TEXT_ATTR *pstButtonAttr, unsigned char isMultiChapters, int timeoutSec);
static int GetFirstHiliIndex(const GUI_MENU *pstMenu);
static int GetNextHiliIndex(const GUI_MENU *pstMenu, int iCurrIndex, short nOffset);
static int PrepareScrMenu(const GUI_MENU *pstMenu, Rect stMenuRect, int nTotalLine, const Rect *pstItemsRect, int iStartIndex, int iHilightIndex, enum MENUSTYLE eMode, int *piScrItemNum, int *itemList);

static void PrepareScrPage(const GUI_PAGE *pstPage, Rect stPageRect, int iStartIndex, int iLineInPage);

static unsigned short GetStrPix(const unsigned char *pszSource, const ST_FONT *pstFont_S, const ST_FONT *pstFont_M, unsigned int uiSpaceWidth);
static void MapChar2Index(unsigned char ch, int *piRow, int *piCol);
static int MapKey(unsigned char ch);

static unsigned char IsValidTime(const unsigned char *psDateTime, const unsigned char *psMaskYYYYMMDDhhmmss);
static unsigned long Asc2Long(const unsigned char *psString, unsigned int uiLength);
unsigned char IsLeapYear(unsigned long ulYear);
static void AdjustDateTime(unsigned char *pszYYYYMMDDhhmmss, short nOffset, unsigned char ucMode);

// Added by Kim 2014-08-27 v0.4
int GetFontHeight(ST_FONT stFont_S, ST_FONT stFont_M);

// Added by Kim_LinHB 2014-7-31 v0.2
static unsigned char OpenTouchScreen();
static const Rect *FindMatchedButton(const TS_POINT_T *pPt, const Rect *pButtons, int nButtonNum);

static void PrepareRes();

static int SetKeybyIndex(int index, const char *text);
static int SetKeybyIndex_Text(KeyMap *key, const char *text);

static const char *GetKeyValuebyIndex(int index);

static int SetCallbackEvent(gui_callbacktype_t type, GUI_CALLBACK func);
static GUI_CALLBACK GetCallbackEvent(gui_callbacktype_t type);

static int SaveSignImg(const Rect *area, const unsigned char *pszOutputFile);
static int SaveSignRoute(const Rect *area, const SignData *data, const unsigned char *pszOutputFile);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static unsigned char sg_bStartup = TRUE;
static unsigned int sg_nDefBg;
static unsigned int sg_nDefColor;
static int sg_nScrWidth = 0, sg_nScrHeight = 0;
static unsigned char sg_isColorScreen;
static unsigned char sg_hasTouchScreen;
static unsigned char sg_ucTermialType;
static CallbackEvent sg_event[]={
	{GUI_CALLBACK_LISTEN_EVENT, NULL},
	{GUI_CALLBACK_UPDATE_TEXT, NULL},
};
#ifdef _AR_MODE_
static unsigned int sg_uiArabicFileID = AR_OPENFILE_ERROR;
#endif
static ST_FONT sgFontS[3] = { { CHARSET_WEST, 6, 8, 0, 0 }, { CHARSET_WEST, 8, 16, 0, 0 }, { CHARSET_WEST, 12, 24, 0, 0 }, };

static ST_FONT sgFontM[3] = { { CHARSET_WEST, 6, 8, 0, 0 }, { CHARSET_WEST, 8, 16, 0, 0 }, { CHARSET_WEST, 12, 24, 0, 0 }, };

static const KeyMap sgPhyKeyMap[16] = {
{ KEY1, "1QZ.qz", 1, 1}, { KEY2, "2ABCabc", 1, 1}, { KEY3, "3DEFdef", 1, 1}, { KEYCLEAR, "", 1, 2 },
{ KEY4, "4GHIghi", 1, 1 }, { KEY5, "5JKLjkl", 1, 1 }, { KEY6, "6MNOmno", 1, 1 }, { -1, "", 0, 0 },
{ KEY7, "7PRSprs", 1, 1 }, { KEY8, "8TUVtuv", 1, 1 }, { KEY9, "9WXYwxy", 1, 1 }, { KEYENTER, "", 1, 2 },
{ KEYCANCEL, "", 1, 1 }, { KEY0, "0,*# ~`!@$%^&-+=(){}[]<>_|\\:;\"\'?/", 1, 1 }, { KEYALPHA, "", 1, 1 }, { -1, "", 0, 0 } };

static KEY_T sg_key = {{{0,NULL, 0, 0}}, 0};

static int sg_nRow = 4;
static int sg_nHeightOfALine = 32;

static Rect sgTitleArea;

static Rect sgMsgContentArea;
static Rect sgMsgBoxButtons[3];

static Rect sgInputPromptArea;
static Rect sgInputContentArea;
static Rect sgInputBoxButtons[2];
static Rect sgKeypad[16];

static Rect sgCurrTimeArea;
static Rect sgDateArea[3];
static Rect sgTimeArea[2];
static Rect sgTimeBoxButtons[2];

static Rect sgMenuListArea;
static Rect sgMenuButtons[4];

static Rect sgInfoPageMenu;
static Rect sgInfoPageMenuOpt[2];
static Rect sgInfoPageArea;
static Rect sgInfoPageButtons[4];

static Rect sgAlternativePrompt;
static Rect sgAlternativeOptions[2];
static Rect sgAlternativeButtons[2];

static Rect sgSignatureArea;
static Rect sgSignatureButtons[3];

static char sg_isCalled = 0;

static SignData sgSignData = {{0}, 0};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// initialize Gui, clear the whole layout.
int Gui_Init(unsigned int nBgColor, unsigned int nColor, const unsigned char *pszArabicFile) {
    if (sg_bStartup) {
        unsigned char sTerminalInfo[32];

        GetTermInfo(sTerminalInfo);
        sg_ucTermialType = sTerminalInfo[0];

        sg_hasTouchScreen = sTerminalInfo[19] & 0x01;
        sg_isColorScreen = sTerminalInfo[19] & 0x02;
        // Added by Kim_LinHB 2014-8-6 v0.2 bug498
        //FIXME Kim when monitor fixed
        if(20 == sg_ucTermialType || 21 == sg_ucTermialType || 22 == sg_ucTermialType || 
			15 == sg_ucTermialType || 16 == sg_ucTermialType || 24 == sg_ucTermialType){
			// added by Kim 2014-8-14 v0.3 BroadCom
			sg_isColorScreen = 1;
			// Add End
		}

		UpdateCanvasSize(); // bug on 1.02.0001_20160517
		if(OpenTouchScreen() && sg_nScrHeight > sg_nScrWidth) {
			sg_nRow = 7;
		}
		else{
			sg_nRow = 4;
		}
		sg_nHeightOfALine = sg_nScrHeight / sg_nRow;
		SetKeybyIndex(KEYCANCEL, "CANCEL");
		SetKeybyIndex(KEYCLEAR, "CLEAR");
		SetKeybyIndex(KEYENTER, "ENTER");
		SetKeybyIndex(KEYMENU, "MENU");
		SetKeybyIndex(KEYFN, "FUNC");
		SetKeybyIndex(KEYALPHA, "ALPHA");
		SetKeybyIndex(KEYUP, "UP");
		SetKeybyIndex(KEYDOWN, "DN");
		SetKeybyIndex(GUI_KEYPREV, "1.PREV");
		SetKeybyIndex(GUI_KEYNEXT, "2.NEXT");
		PrepareRes();
        sg_bStartup = FALSE;
    }

    sg_nDefBg = nBgColor;
    if (sg_isColorScreen) {
        sg_nDefColor = nColor;
        CLcdSetBgColor(sg_nDefBg);
        CLcdSetFgColor(sg_nDefColor);
    }

    ClearScr(NULL, 1);

    UpdateCanvasSize();

    // Added by Kim 2014-08-25 v0.4
#ifdef _AR_MODE_
    if (pszArabicFile) {
        if (sg_uiArabicFileID != AR_OPENFILE_ERROR) {
            ArFontClose(sg_uiArabicFileID);
        }
        sg_uiArabicFileID = ArFontOpen((unsigned char *) pszArabicFile);
        if (sg_uiArabicFileID > AR_ERROR_CODE_MIN)  //arabic
                {
            ClearScr(NULL, 1);
            ScrGotoxy(0, 0);
            Lcdprintf("Error:%s error\nPls download ParamFile", pszArabicFile);
            DelayMs(5000);
            return GUI_ERR_INVALIDPARAM;
        }
    }
    else {
        ArFontClose(sg_uiArabicFileID);
        sg_uiArabicFileID = AR_OPENFILE_ERROR;
    }
#endif
    return GUI_OK;
}

int Gui_ClearScr() {
    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }
    ClearScr(NULL, 1);
    return GUI_OK;
}

int Gui_LoadFont(enum FONTSIZE eFontSize, const ST_FONT *pSingleCodeFont, const ST_FONT *pMultiCodeFont) {
    if (sg_bStartup) {
		logTrace("Gui_Init required");
        return GUI_ERR_NOTINIT;
    }

    // verify parameters
    if (!IsValid(eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE)) {
		logTrace("Invalid parameters");
        return GUI_ERR_INVALIDPARAM;
    }

	if (pSingleCodeFont) {
		logTrace("pSingleCodeFont");
		memcpy(&sgFontS[eFontSize], pSingleCodeFont, sizeof(ST_FONT));
	}
       
	if (pMultiCodeFont) {
		logTrace("pMultiCodeFont");
		memcpy(&sgFontM[eFontSize], pMultiCodeFont, sizeof(ST_FONT));
	}

    // Added by Kim 2014-08-25 v0.4
#ifdef _AR_MODE_
    if (pMultiCodeFont && pMultiCodeFont->CharSet == CHARSET_ARABIA) {
        int iNum = ArFontAmount(sg_uiArabicFileID);
        int iMatchedHeight = 3;
        int i;

        for (i = 0; i < iNum; ++i) {
            if (pMultiCodeFont->Height == ArFontHeight(sg_uiArabicFileID, i)) {
                iMatchedHeight = i;
                break;
            }
        }
        sgFontM[eFontSize].Height = iMatchedHeight;
    }
#endif
    return GUI_OK;
}

int Gui_DrawText(const unsigned char *pszText, const GUI_TEXT_ATTR stTextAttr, unsigned int x_percent, unsigned int y_percent) {
    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }

    // verify parameters
    if (!pszText || !IsValid(stTextAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(stTextAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE) || x_percent > 100 || y_percent > 100) {
        return GUI_ERR_INVALIDPARAM;
    }

    {
        // for getting background box
        int nWidth;
        int nHeight;
        int nLeft = x_percent * sg_nScrWidth / 100, nLeftBak = nLeft;

        nWidth = GetStrPix(pszText, &sgFontS[stTextAttr.eFontSize], &sgFontM[stTextAttr.eFontSize], 1);
        nHeight = GetFontHeight(sgFontS[stTextAttr.eFontSize], sgFontM[stTextAttr.eFontSize]);

        switch (stTextAttr.eAlign) {
            case GUI_ALIGN_RIGHT:
                nLeft = sg_nScrWidth - nWidth;
                break;
            case GUI_ALIGN_CENTER:
                nLeft += (sg_nScrWidth - nLeft - nWidth) / 2;
                break;
        }
        if (nLeft < nLeftBak)
            nLeft = nLeftBak;
        DrawText((void*) pszText, (void*) &stTextAttr, y_percent * sg_nScrHeight / 100, y_percent * sg_nScrHeight / 100 + nHeight, nLeft, nLeft + nWidth, 0);
    }
    return GUI_OK;
}

int Gui_DrawLogo(const unsigned char *psLogo, int x, int y) {
    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }

    // verify parameters
    if (!psLogo || x > sg_nScrWidth || y > sg_nScrHeight) {
        return GUI_ERR_INVALIDPARAM;
    }

    DrawLogo((void *) psLogo, y, x);
    return GUI_OK;
}

int Gui_DrawImage(const unsigned char *pszImagePath, unsigned int x_percent, unsigned int y_percent) {
    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }

    if (!sg_isColorScreen) {
        return GUI_ERR_UNSUPPORTED;
    }

    // verify parameters
    if (!pszImagePath || x_percent > 100 || y_percent > 100) {
        return GUI_ERR_INVALIDPARAM;
    }
    
    // Added By Kim 2015-03-18 bug667
    if(NULL == strstr(pszImagePath, ".bmp") && NULL == strstr(pszImagePath, ".BMP") &&
		NULL == strstr(pszImagePath, ".png") && NULL == strstr(pszImagePath, ".PNG")){
        return GUI_ERR_INVALIDPARAM;
    }

    DrawImage((void *) pszImagePath, y_percent * sg_nScrHeight / 100, x_percent * sg_nScrWidth / 100);
    return GUI_OK;
}

// Modified By Kim 2015-03-18 bug 669
int Gui_ShowMsgBox(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr, const unsigned char *pszContent, GUI_TEXT_ATTR stContentAttr, enum MSGBOXTYPE eType, int timeoutSec, int *pucKeyValue) {
    GUI_TEXT_ATTR stButtonAttr;
    unsigned char bChkTimer;
    int iRet = GUI_ERR_TIMEOUT;
    int iKey = NOKEY;
    RectMap stRectMap[1];

    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }

	if(sg_isCalled)
		return GUI_ERR_CANTCALLFROMCALLBACK;

	sg_isCalled = 1;

// verify parameter
    if (pszTitle && (!IsValid(stTitleAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(stTitleAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE))) {
        return GUI_ERR_INVALIDPARAM;
    }
    if (pszContent && (!IsValid(stContentAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(stContentAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE))) {
        return GUI_ERR_INVALIDPARAM;
    }
    if (!IsValid(eType, GUI_BUTTON_NONE, GUI_BUTTON_YandN)) {
        return GUI_ERR_INVALIDPARAM;
    }

    //title
    if (pszTitle) {
		DrawText((void *) pszTitle, (void *) &stTitleAttr, sgTitleArea.top, sgTitleArea.bottom, sgTitleArea.left, sgTitleArea.right, 0);
    }

    if (pszContent) {
		DrawText((void *) pszContent, (void *) &stContentAttr, sgMsgContentArea.top, sgMsgContentArea.bottom, sgMsgContentArea.left, sgMsgContentArea.right, 0);
    }

    stButtonAttr.eFontSize = GUI_FONT_SMALL;
    stButtonAttr.eStyle = stContentAttr.eStyle;
    stButtonAttr.eAlign = GUI_ALIGN_CENTER;

	memset(stRectMap, 0, sizeof(stRectMap));

    switch (eType) {
        case GUI_BUTTON_OK: 
			DrawBgBox(sgMsgBoxButtons[2].left, sgMsgBoxButtons[2].top, sgMsgBoxButtons[2].right, sgMsgBoxButtons[2].bottom, sg_nDefBg);
			DrawText((void *) GetKeyValuebyIndex(KEYENTER), (void *) &stButtonAttr, sgMsgBoxButtons[2].top, sgMsgBoxButtons[2].bottom, sgMsgBoxButtons[2].left, sgMsgBoxButtons[2].right, 1);
			sgMsgBoxButtons[2].nValue = KEYENTER;
			stRectMap[0].pRect = &sgMsgBoxButtons[2];
			stRectMap[0].no = 1;
            break;
        case GUI_BUTTON_CANCEL: 
			DrawBgBox(sgMsgBoxButtons[2].left, sgMsgBoxButtons[2].top, sgMsgBoxButtons[2].right, sgMsgBoxButtons[2].bottom, sg_nDefBg);
			DrawText((void *) GetKeyValuebyIndex(KEYCANCEL), (void *) &stButtonAttr, sgMsgBoxButtons[2].top, sgMsgBoxButtons[2].bottom, sgMsgBoxButtons[2].left, sgMsgBoxButtons[2].right, 1);
			sgMsgBoxButtons[2].nValue = KEYCANCEL;		
			stRectMap[0].pRect = &sgMsgBoxButtons[2];
			stRectMap[0].no = 1;
            break;
        case GUI_BUTTON_YandN: 
			DrawBgBox(sgMsgBoxButtons[0].left, sgMsgBoxButtons[0].top, sgMsgBoxButtons[0].right, sgMsgBoxButtons[0].bottom, sg_nDefBg);
			DrawText((void *) GetKeyValuebyIndex(KEYCANCEL), (void *) &stButtonAttr, sgMsgBoxButtons[0].top, sgMsgBoxButtons[0].bottom, sgMsgBoxButtons[0].left, sgMsgBoxButtons[0].right, 1);

			DrawBgBox(sgMsgBoxButtons[1].left, sgMsgBoxButtons[1].top, sgMsgBoxButtons[1].right, sgMsgBoxButtons[1].bottom, sg_nDefBg);
			DrawText((void *) GetKeyValuebyIndex(KEYENTER), (void *) &stButtonAttr, sgMsgBoxButtons[1].top, sgMsgBoxButtons[1].bottom, sgMsgBoxButtons[1].left, sgMsgBoxButtons[1].right, 1);

			stRectMap[0].pRect = &sgMsgBoxButtons[0];
			stRectMap[0].no = 2;
            break;
        default:
            break;
    }

    // get key
    if (timeoutSec >= 0) {
        bChkTimer = TRUE;
        TimerSet(GUI_TIMER_INDEX, (short) (timeoutSec * 10));
    }
    else {
        bChkTimer = FALSE;
    }

    while (1) {
		int iTouchStatus = 0;
		GUI_CALLBACK vFunc= NULL;
		if((vFunc = GetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT))){
			int callbackLen = 0;
			iRet = vFunc(GUI_CALLBACK_LISTEN_EVENT, NULL, &callbackLen);
			if(iRet != GUI_OK)
				break;
		}
        iTouchStatus = GetVirtualKey(stRectMap, sizeof(stRectMap)/sizeof(stRectMap[0]));
        iKey = 0;

        if (0 == kbhit()) {
            iKey = getkey();
            TimerSet(GUI_TIMER_INDEX, (short) (timeoutSec * 10));
        }
        else if(iTouchStatus > 0) {
            TimerSet(GUI_TIMER_INDEX, (short)(timeoutSec*10));
			iKey = iTouchStatus;
        }
        else if (bChkTimer && 0 == TimerCheck(GUI_TIMER_INDEX)) {
            iRet = GUI_ERR_TIMEOUT;
            iKey = NOKEY;
            break;
        }

        if (iKey != 0) {
            if (GUI_BUTTON_NONE == eType) {
                // Added by Kim_LinHB 20140916 v0.8 bug517
                if (KEYCANCEL == iKey) {
                    iRet = GUI_ERR_USERCANCELLED;
                    break;
                }
                iRet = GUI_OK;
                break;
            }
            else if (GUI_BUTTON_CANCEL == eType) {
                if (KEYCANCEL == iKey) {
                    iRet = GUI_ERR_USERCANCELLED;
                    break;
                }
            }
            else if (GUI_BUTTON_OK == eType) {
                if (KEYENTER == iKey) {
                    iRet = GUI_OK;
                    break;
                }
            }
            else if (GUI_BUTTON_YandN == eType) {
                if (KEYCANCEL == iKey) {
                    iRet = GUI_ERR_USERCANCELLED;
                    break;
                }
                else if (KEYENTER == iKey) {
                    iRet = GUI_OK;
                    break;
                }
            }
			if((vFunc = GetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT))){
				int callbackLen = sizeof(int);
				iRet = vFunc(GUI_CALLBACK_LISTEN_EVENT, &iKey, &callbackLen);
				if(iRet != GUI_OK){
					break;
				}
			}
        }
    }

    if (pucKeyValue) {
        *pucKeyValue = iKey;
    }
	sg_isCalled = 0;
    return iRet;
}

int Gui_ShowInputBox(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr, const unsigned char *pszPrompt, GUI_TEXT_ATTR stPromptAttr, unsigned char *pszContent, GUI_TEXT_ATTR stContentAttr, const GUI_INPUTBOX_ATTR *pstAttr, int timeoutSec) {
    GUI_TEXT_ATTR stButtonAttr;

    int iRet = GUI_ERR_TIMEOUT;

    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }

	if(sg_isCalled)
		return GUI_ERR_CANTCALLFROMCALLBACK;

	sg_isCalled = 1;

    if (!pstAttr)
        return GUI_ERR_INVALIDPARAM;

    // verify parameter
    if (pszTitle && (!IsValid(stTitleAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(stTitleAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE))) {
        return GUI_ERR_INVALIDPARAM;
    }
    if (pszPrompt && (!IsValid(stPromptAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(stPromptAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE))) {
        return GUI_ERR_INVALIDPARAM;
    }
    if (!IsValid(stContentAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(stContentAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE) || !IsValid(pstAttr->eType, GUI_INPUT_NUM, GUI_INPUT_MIX) || pstAttr->nMinLen > pstAttr->nMaxLen) {
        return GUI_ERR_INVALIDPARAM;
    }

    stButtonAttr.eStyle = stContentAttr.eStyle;
    stButtonAttr.eAlign = GUI_ALIGN_CENTER;

    if (pszTitle) {
        DrawText((void *) pszTitle, (void *) &stTitleAttr, sgTitleArea.top, sgTitleArea.bottom, sgTitleArea.left, sgTitleArea.right, 0);
    }

	stButtonAttr.eFontSize = GUI_FONT_SMALL;
    if(sg_hasTouchScreen && sg_nScrHeight > sg_nScrWidth) {
        iRet = GetInput(pszPrompt, stPromptAttr, sgInputPromptArea, pszContent, stContentAttr, sgInputContentArea, pstAttr, sgKeypad, stButtonAttr, 16, timeoutSec);
    }
    else {
        iRet = GetInput(pszPrompt, stPromptAttr, sgInputPromptArea, pszContent, stContentAttr, sgInputContentArea, pstAttr, sgInputBoxButtons, stButtonAttr, 2, timeoutSec);
    }

	sg_isCalled = 0;
    return iRet;
}

int Gui_ShowTimeBox(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr, unsigned char *pszTime, GUI_TEXT_ATTR stContentAttr, unsigned char isTime, int timeoutSec) {
    GUI_TEXT_ATTR stButtonAttr;

    int iRet = GUI_ERR_TIMEOUT;

    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }

	if(sg_isCalled)
		return GUI_ERR_CANTCALLFROMCALLBACK;

	sg_isCalled = 1;

    // verify parameters
    if (pszTitle && (!IsValid(stTitleAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(stTitleAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE))) {
        return GUI_ERR_INVALIDPARAM;
    }
    if (!IsValid(stContentAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(stContentAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE)) {
        return GUI_ERR_INVALIDPARAM;
    }

    stButtonAttr.eStyle = stContentAttr.eStyle;
    stButtonAttr.eAlign = GUI_ALIGN_CENTER;

    if (pszTitle) {
        DrawText((void *) pszTitle, (void *) &stTitleAttr, sgTitleArea.top, sgTitleArea.bottom, sgTitleArea.left, sgTitleArea.right, 0);
    }

	stButtonAttr.eFontSize = GUI_FONT_SMALL;
    if(sg_hasTouchScreen && sg_nScrHeight > sg_nScrWidth) {
        iRet = GetInputTime(sgCurrTimeArea, isTime?sgTimeArea:sgDateArea, stContentAttr, pszTime,sgKeypad, 16, stButtonAttr, isTime, timeoutSec);
    }
    else{
        iRet = GetInputTime(sgCurrTimeArea, isTime?sgTimeArea:sgDateArea, stContentAttr, pszTime, sgTimeBoxButtons, 2, stButtonAttr, isTime, timeoutSec);
    }

	sg_isCalled = 0;
    return iRet;
}

int Gui_BindMenu(const unsigned char *psztitle, GUI_TEXT_ATTR stTitleAttr, GUI_TEXT_ATTR stTextAttr, const GUI_MENUITEM *pstMenuItem, GUI_MENU *pstMenu) {
    int i = 0;

    // verify parameters
    if (pstMenu == NULL || pstMenuItem == NULL) {
        return GUI_ERR_INVALIDPARAM;
    }

    if (psztitle)
        sprintf(pstMenu->szTitle, "%.*s", sizeof(pstMenu->szTitle), psztitle);
    else
        memset(pstMenu->szTitle, 0, sizeof(pstMenu->szTitle));
    pstMenu->stTitleAttr = stTitleAttr;
    pstMenu->stItemAttr = stTextAttr;

    pstMenu->pstContent = (GUI_MENUITEM *) pstMenuItem;
    pstMenu->nSize = 0;
    while (1) {
        if (strcmp(pstMenuItem[i].szText, "") == 0) {
            break;
        }
        pstMenu->nSize = (i++) + 1;
    }
    return GUI_OK;
}

int Gui_ShowMenuList(const GUI_MENU *pstMenu, enum MENUSTYLE eMode, int timeoutSec, int *piSelValue) {
    int iRet = 0;

    GUI_TEXT_ATTR stButtonAttr;

    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }

	if(sg_isCalled)
		return GUI_ERR_CANTCALLFROMCALLBACK;

	sg_isCalled = 1;

    // verify parameters
    if (!pstMenu) {
        return GUI_ERR_INVALIDPARAM;
    }

    if (!IsValid(pstMenu->stItemAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(pstMenu->stItemAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE)) {
        return GUI_ERR_INVALIDPARAM;
    }

	stButtonAttr.eFontSize = GUI_FONT_SMALL;
    stButtonAttr.eStyle = 0;
    stButtonAttr.eAlign = GUI_ALIGN_CENTER;

    if (piSelValue && (*piSelValue > (int) pstMenu->nSize || *piSelValue < 0))
        *piSelValue = 0;

	iRet = GetMenuItem(pstMenu, eMode, sgMenuListArea, sgMenuButtons, 4, &stButtonAttr, timeoutSec, piSelValue);
	sg_isCalled = 0;
    return iRet;
}

int Gui_ShowMenuListWithoutButtons(const GUI_MENU *pstMenu, enum MENUSTYLE eMode, int timeoutSec, int *piSelValue) {
	int iRet = 0;

	if (sg_bStartup) {
		return GUI_ERR_NOTINIT;
	}

	if (sg_isCalled)
		return GUI_ERR_CANTCALLFROMCALLBACK;

	sg_isCalled = 1;

	// verify parameters
	if (!pstMenu) {
		return GUI_ERR_INVALIDPARAM;
	}

	if (!IsValid(pstMenu->stItemAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(pstMenu->stItemAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE)) {
		return GUI_ERR_INVALIDPARAM;
	}

	if (piSelValue && (*piSelValue > (int)pstMenu->nSize || *piSelValue < 0))
		*piSelValue = 0;

	Rect temp = sgMenuListArea;
	temp.bottom = sg_nScrHeight - 1;

	iRet = GetMenuItem(pstMenu, eMode, temp, NULL, 0, NULL, timeoutSec, piSelValue);
	sg_isCalled = 0;
	return iRet;
}

int Gui_ShowAlternative(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr, const unsigned char *pszPrompt, GUI_TEXT_ATTR stContentAttr, const unsigned char *pszOption1, int iValue1, const unsigned char *pszOption2, int iValue2, int timeoutSec, int *piSelOption) {
    GUI_TEXT_ATTR stButtonAttr;

    unsigned char bChkTimer;
    int iRet = GUI_ERR_TIMEOUT;
    int iKey = NOKEY;
    unsigned char szBuff[2][20];
    int iSelected;

    RectMap stRectMap[2];

    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }

	if(sg_isCalled)
		return GUI_ERR_CANTCALLFROMCALLBACK;

	sg_isCalled = 1;

    // verify parameter
    if (pszTitle && (!IsValid(stTitleAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(stTitleAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE))) {
        return GUI_ERR_INVALIDPARAM;
    }
    if ((!IsValid(stContentAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(stContentAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE))) {
        return GUI_ERR_INVALIDPARAM;
    }

    if (!pszOption1 || !pszOption2 || !piSelOption || iValue1 == iValue2) {
        return GUI_ERR_INVALIDPARAM;
    }

    if (*piSelOption != iValue1 && *piSelOption != iValue2) {
        return GUI_ERR_INVALIDPARAM;
    }

    sprintf(szBuff[0], "%d.%s", iValue1, pszOption1);
    sprintf(szBuff[1], "%d.%s", iValue2, pszOption2);

    //title
    if (pszTitle) {
        DrawText((void *) pszTitle, (void *) &stTitleAttr, sgTitleArea.top, sgTitleArea.bottom, sgTitleArea.left, sgTitleArea.right, 0);
    }

    if (pszPrompt) {
        DrawText((void *) pszPrompt, (void *) &stContentAttr, sgAlternativePrompt.top, sgAlternativePrompt.bottom, sgAlternativePrompt.left, sgAlternativePrompt.right, 0);
    }

    stButtonAttr.eFontSize = GUI_FONT_SMALL;
    stButtonAttr.eStyle = stContentAttr.eStyle;
    stButtonAttr.eAlign = GUI_ALIGN_CENTER;

	DrawBgBox(sgAlternativeButtons[0].left, sgAlternativeButtons[0].top, sgAlternativeButtons[0].right, sgAlternativeButtons[0].bottom, sg_nDefBg);
	DrawBgBox(sgAlternativeButtons[0].left, sgAlternativeButtons[0].top, sgAlternativeButtons[0].right, sgAlternativeButtons[0].bottom, sg_nDefBg);

	DrawText((void *) GetKeyValuebyIndex(KEYCANCEL), (void *) &stButtonAttr, sgAlternativeButtons[0].top, sgAlternativeButtons[0].bottom, sgAlternativeButtons[0].left, sgAlternativeButtons[0].right, 1);
	DrawText((void *) GetKeyValuebyIndex(KEYENTER), (void *) &stButtonAttr, sgAlternativeButtons[1].top, sgAlternativeButtons[1].bottom, sgAlternativeButtons[1].left, sgAlternativeButtons[1].right, 1);

    sgAlternativeOptions[0].nValue = 1000 + iValue1;
    sgAlternativeOptions[1].nValue = 1000 + iValue2;

	iSelected = *piSelOption + 1000;

    {
        int j;
        GUI_TEXT_ATTR stReversal = stButtonAttr;
        if (stReversal.eStyle & GUI_FONT_REVERSAL) {
            stReversal.eStyle &= ~GUI_FONT_REVERSAL;
        }
        else {
            stReversal.eStyle |= (GUI_FONT_REVERSAL | GUI_FONT_OPAQUE);
        }
        for (j = 0; j < 2; ++j) { // just 2 options
            if (sgAlternativeOptions[j].nValue == iSelected) {
                DrawText((void *) szBuff[j], (void *) &stReversal, sgAlternativeOptions[j].top, sgAlternativeOptions[j].bottom, sgAlternativeOptions[j].left, sgAlternativeOptions[j].right, 1);
            }
            else {
                DrawText((void *) szBuff[j], (void *) &stButtonAttr, sgAlternativeOptions[j].top, sgAlternativeOptions[j].bottom, sgAlternativeOptions[j].left, sgAlternativeOptions[j].right, 1);
            }
        }
    }

    if (timeoutSec >= 0) {
        bChkTimer = TRUE;
        TimerSet(GUI_TIMER_INDEX, (short) timeoutSec * 10);
    }
    else {
        bChkTimer = FALSE;
    }

    memset(stRectMap, 0, sizeof(stRectMap));
    stRectMap[0].pRect = sgAlternativeButtons;
    stRectMap[0].no = 2;

    stRectMap[1].pRect = sgAlternativeOptions;
    stRectMap[1].no = 2;

    while (1) {
		int iTouchStatus = 0;
		GUI_CALLBACK vFunc = NULL;
		if((vFunc = GetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT))){
			int callbackLen = 0;
			iRet = vFunc(GUI_CALLBACK_LISTEN_EVENT, NULL, &callbackLen);
			if(iRet != GUI_OK)
				break;
		}
        iTouchStatus = GetVirtualKey(stRectMap, sizeof(stRectMap)/sizeof(stRectMap[0]));
        iKey = 0;

        if (0 == kbhit()) {
            TimerSet(GUI_TIMER_INDEX, (short) timeoutSec * 10);
            iKey = getkey();
        }
        else if(iTouchStatus > 0) {
            TimerSet(GUI_TIMER_INDEX, (short)(timeoutSec*10));
			iKey = iTouchStatus;
        }
        else if (bChkTimer && 0 == TimerCheck(GUI_TIMER_INDEX)) {
            iKey = GUI_ERR_TIMEOUT;
        }
        else {
            continue;
        }

        if (KEYCANCEL == iKey) {
            iRet = GUI_ERR_USERCANCELLED;
            break;
        }
        else if (GUI_ERR_TIMEOUT == iKey) {
            iRet = GUI_ERR_TIMEOUT;
            break;
        }
        else if (KEYENTER == iKey) {
            iRet = GUI_OK;
            break;
        }
		else if (KEYALPHA == iKey) {
			iSelected = (iSelected == sgAlternativeOptions[0].nValue ? sgAlternativeOptions[1].nValue : sgAlternativeOptions[0].nValue);
		}
		else if (sgAlternativeOptions[0].nValue == iKey || sgAlternativeOptions[1].nValue == iKey){
			iSelected = iKey;
		}

        //ClearScr(&pstTimeRect[ucSelected]);

        {
            int j;
            GUI_TEXT_ATTR stReversal = stButtonAttr;
            if (stReversal.eStyle & GUI_FONT_REVERSAL) {
                stReversal.eStyle &= ~GUI_FONT_REVERSAL;
            }
            else {
                stReversal.eStyle |= (GUI_FONT_REVERSAL | GUI_FONT_OPAQUE);
            }
            for (j = 0; j < 2; ++j) {
                if (sgAlternativeOptions[j].nValue == iSelected) {
                    DrawText((void *) szBuff[j], (void *) &stReversal, sgAlternativeOptions[j].top, sgAlternativeOptions[j].bottom, sgAlternativeOptions[j].left, sgAlternativeOptions[j].right, 1);
                }
                else {
					ClearScr(&sgAlternativeOptions[j], 1);
					DrawBgBox(sgAlternativeOptions[j].left, sgAlternativeOptions[j].top, sgAlternativeOptions[j].right, sgAlternativeOptions[j].bottom, sg_nDefBg);
                    DrawText((void *) szBuff[j], (void *) &stButtonAttr, sgAlternativeOptions[j].top, sgAlternativeOptions[j].bottom, sgAlternativeOptions[j].left, sgAlternativeOptions[j].right, 1);
                }
            }
        }
    }

    if (GUI_OK == iRet)
        *piSelOption = iSelected - 1000;
	sg_isCalled = 0;
    return iRet;
}

int Gui_ShowInfoPage(const GUI_PAGE *pstPage, unsigned char isMultiChapters, int timeoutSec) {
    unsigned int i;
    int iRet = 0;

    GUI_TEXT_ATTR stButtonAttr;

    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }

	if(sg_isCalled)
		return GUI_ERR_CANTCALLFROMCALLBACK;

	sg_isCalled = 1;

    // verify parameters
    if (!pstPage) {
        return GUI_ERR_INVALIDPARAM;
    }
    if (pstPage->szTitle[0] != 0 && (!IsValid(pstPage->stTitleAttr.eAlign , GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(pstPage->stTitleAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE))) {
        return GUI_ERR_INVALIDPARAM;
    }
    for (i = 0; i < pstPage->nLine; ++i) {
        if (!IsValid(pstPage->pstContent[i].stLineAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT) || !IsValid(pstPage->pstContent[i].stLineAttr.eFontSize, GUI_FONT_SMALL, GUI_FONT_LARGE)) {
            return GUI_ERR_INVALIDPARAM;
        }
    }

    stButtonAttr.eFontSize = GUI_FONT_SMALL;
    stButtonAttr.eStyle = 0;
    stButtonAttr.eAlign = GUI_ALIGN_CENTER;

	if(sg_isColorScreen)
		iRet = GetInfoPage(pstPage, sgInfoPageArea, sgInfoPageButtons, 4, &stButtonAttr, isMultiChapters, timeoutSec);
	else
		iRet = GetInfoPage(pstPage, sgInfoPageArea, NULL, 0, NULL, isMultiChapters, timeoutSec);

	sg_isCalled = 0;
    return iRet;
}

int Gui_CreateInfoPage(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr, const GUI_PAGELINE *pstContent, unsigned int nLine, GUI_PAGE *pstPage) {
    if (!pstPage) {
        return GUI_ERR_INVALIDPARAM;
    }

    if (pszTitle)
        sprintf(pstPage->szTitle, "%.*s", sizeof(pstPage->stTitleAttr), pszTitle);
    else
        pstPage->szTitle[0] = 0;
    pstPage->stTitleAttr = stTitleAttr;
    pstPage->nLine = nLine;

    pstPage->pstContent = (GUI_PAGELINE *) pstContent;
    return GUI_OK;
}

int Gui_ShowSignatureBoard(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr,
	const unsigned char *pszOutputFile,
	char nMode, int timeoutSec){
	GUI_TEXT_ATTR stButtonAttr;
	unsigned char bChkTimer;
	int iRet = GUI_ERR_TIMEOUT;
	int iKey = NOKEY;
	RectMap stRectMap[1];
	int i;
	TS_ATTR_T signMode = {1};
	TS_ATTR_T normalMode = {0};

	if (sg_bStartup) {
		return GUI_ERR_NOTINIT;
	}

	if (!sg_hasTouchScreen) {
		return GUI_ERR_UNSUPPORTED;
	}

	// verify parameter
	if (pszTitle && (!IsValid(stTitleAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT))) {
		return GUI_ERR_INVALIDPARAM;
	}
	if(!pszOutputFile){// || (nMode != 0 && nMode != 1)){
		return GUI_ERR_INVALIDPARAM;
	}

	//Prepare Resources

	if(sg_isCalled)
		return GUI_ERR_CANTCALLFROMCALLBACK;

	sg_isCalled = 1;

	//title
	if(pszTitle)
	DrawText((void *) pszTitle, (void *) &stTitleAttr, sgTitleArea.top, sgTitleArea.bottom, sgTitleArea.left, sgTitleArea.right, 0);

	stButtonAttr.eFontSize = GUI_FONT_SMALL;
	stButtonAttr.eStyle = GUI_FONT_STD;
	stButtonAttr.eAlign = GUI_ALIGN_CENTER;

	for (i = 0; i < sizeof(sgSignatureButtons) / sizeof(sgSignatureButtons[0]); ++i) { // total buttons on screen, 3 or 16
		DrawBgBox(sgSignatureButtons[i].left, sgSignatureButtons[i].top, sgSignatureButtons[i].right, sgSignatureButtons[i].bottom, sg_nDefBg);

		if(sgSignatureButtons[i].nValue != 0) {
			DrawRect(sgSignatureButtons[i].left, sgSignatureButtons[i].top, sgSignatureButtons[i].right, sgSignatureButtons[i].bottom, sg_nDefColor);
			DrawRect(sgSignatureButtons[i].left+ 1, sgSignatureButtons[i].top+ 1, sgSignatureButtons[i].right-1, sgSignatureButtons[i].bottom-1, sg_nDefColor);
		}

		switch (sgSignatureButtons[i].nValue) {
		case KEYENTER:
			DrawText((void *) GetKeyValuebyIndex(KEYENTER), (void *) &stButtonAttr, sgSignatureButtons[i].top, sgSignatureButtons[i].bottom, sgSignatureButtons[i].left, sgSignatureButtons[i].right, 1);
			break;
		case KEYCANCEL:
			DrawText((void *) GetKeyValuebyIndex(KEYCANCEL), (void *) &stButtonAttr, sgSignatureButtons[i].top, sgSignatureButtons[i].bottom, sgSignatureButtons[i].left, sgSignatureButtons[i].right, 1);
			break;
		case KEYCLEAR:
			DrawText((void *) GetKeyValuebyIndex(KEYCLEAR), (void *) &stButtonAttr, sgSignatureButtons[i].top, sgSignatureButtons[i].bottom, sgSignatureButtons[i].left, sgSignatureButtons[i].right, 1);
			break;
		}
	}

	DrawBgBox(sgSignatureArea.left, sgSignatureArea.top, sgSignatureArea.right, sgSignatureArea.bottom, sg_nDefBg);
	DrawRect(sgSignatureArea.left-1, sgSignatureArea.top-1, sgSignatureArea.right+1, sgSignatureArea.bottom+1, sg_nDefColor);

	if (timeoutSec >= 0) {
		bChkTimer = TRUE;
		TimerSet(GUI_TIMER_INDEX, (short) (timeoutSec * 10));
	}
	else {
		bChkTimer = FALSE;
	}

	memset(&sgSignData, 0, sizeof(SignData));

	memset(stRectMap, 0, sizeof(stRectMap));

	stRectMap[0].pRect = sgSignatureButtons;
	stRectMap[0].no = 3;

	TouchScreenAttrSet(&signMode);
	
	while (1) {
		int iTouchStatus = 0;

		GUI_CALLBACK vFunc= NULL;
		if((vFunc = GetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT))){
			int callbackLen = 0;
			iRet = vFunc(GUI_CALLBACK_LISTEN_EVENT, NULL, &callbackLen);
			if(iRet != GUI_OK)
				break;
		}

		iKey = 0;
		iTouchStatus = GetVirtualKey(stRectMap, sizeof(stRectMap)/sizeof(stRectMap[0]));
		DrawSign(&sgSignatureArea);

		if (0 == kbhit()) {
			iKey = getkey();
			TimerSet(GUI_TIMER_INDEX, (short) (timeoutSec * 10));
		}
		else if(iTouchStatus > 0) {
			TimerSet(GUI_TIMER_INDEX, (short)(timeoutSec*10));
			iKey = iTouchStatus;
		}
		else if (bChkTimer && 0 == TimerCheck(GUI_TIMER_INDEX)) {
			iRet = GUI_ERR_TIMEOUT;
			iKey = NOKEY;
			break;
		}

		if (iKey != 0) {
			if (KEYCANCEL == iKey) {
				iRet = GUI_ERR_USERCANCELLED;
				break;
			}
			else if (KEYENTER == iKey) {
				if(0 == nMode) {
					iRet = SaveSignImg(&sgSignatureArea, pszOutputFile);
				}
				else if(1 == nMode){
					iRet = SaveSignRoute(&sgSignatureArea, &sgSignData, pszOutputFile);
				}
				break;
			}
			else if (KEYCLEAR == iKey){
				TimerSet(GUI_TIMER_INDEX, (short)(timeoutSec*10));
				//XuiSigBoardSetStat(sg_sign_area.pSignatureBoard, &sg_sign_area.stAttr);
				ClearScr(&sgSignatureArea, 0);
				memset(&sgSignData, 0, sizeof(SignData));
				continue;
			}
		}
	}
	TouchScreenAttrSet(&normalMode);
	sg_isCalled = 0;

	return iRet;
}

int Gui_GetImageSize(const unsigned char *File, unsigned int *pWidth, unsigned int *pHeight) {
    int fd;

    if (!pHeight && !pWidth) {
        return GUI_ERR_INVALIDPARAM;
    }

    fd = open((char *) File, O_RDWR);
    if (fd < 0) {
        return GUI_ERR_INVALIDPARAM;
    }

    if (strstr(File, ".bmp") || strstr(File, ".BMP")) {
        unsigned char wtmp[4] = { '0' };
        unsigned char htmp[4] = { '0' };

        seek(fd, 0x12, SEEK_SET);
        read(fd, wtmp, 4);
        read(fd, htmp, 4);
        close(fd);

        //modified by Kim 2014-8-14 v0.3
        *pWidth = (unsigned int) ((unsigned int) (wtmp[0] << 24) + (wtmp[1] << 16) + (wtmp[2] << 8) + wtmp[3]);
        *pHeight = (unsigned int) ((unsigned int) (htmp[0] << 24) + (htmp[1] << 16) + (htmp[2] << 8) + htmp[3]);
    }
    else if (strstr(File, ".png") || strstr(File, ".PNG")) {
        unsigned char wtmp[4] = { '0' };
        unsigned char htmp[4] = { '0' };

        seek(fd, 0x10, SEEK_SET);
        read(fd, wtmp, 4);
        read(fd, htmp, 4);
        close(fd);

        //modified by Kim 2014-8-14 v0.3
        *pWidth = (unsigned int) ((unsigned int) (wtmp[0] << 24) + (wtmp[1] << 16) + (wtmp[2] << 8) + wtmp[3]);
        *pHeight = (unsigned int) ((unsigned int) (htmp[0] << 24) + (htmp[1] << 16) + (htmp[2] << 8) + htmp[3]);
    }
    return GUI_OK;
}

int Gui_GetScrWidth(void){
	if (sg_bStartup) {
		return GUI_ERR_NOTINIT;
	}
	return sg_nScrWidth;
}

int Gui_GetScrHeight(void){
	if (sg_bStartup) {
		return GUI_ERR_NOTINIT;
	}
	return sg_nScrHeight;
}


int Gui_RegCallback(gui_callbacktype_t type, GUI_CALLBACK func){
	if (sg_bStartup) {
		return GUI_ERR_NOTINIT;
	}
	return SetCallbackEvent(type, func);
}


int Gui_UpdateTitle(const char *pszTitle, GUI_TEXT_ATTR stTitleAttr){
    if (sg_bStartup) {
        return GUI_ERR_NOTINIT;
    }

// verify parameters
    if (pszTitle && (!IsValid(stTitleAttr.eAlign, GUI_ALIGN_LEFT, GUI_ALIGN_RIGHT))) {
        return GUI_ERR_INVALIDPARAM;
    }
	DrawText(pszTitle, &stTitleAttr, 0, sg_nRow, 0, sg_nScrWidth, 0);
	return GUI_OK;
}

int Gui_UpdateKey(int index, const char* text){
	int iRet;
	if (sg_bStartup) {
		return GUI_ERR_NOTINIT;
	}

	iRet = SetKeybyIndex(index, text);
	return iRet;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static int GetInput(const unsigned char *pszPrompt, GUI_TEXT_ATTR stPromptAttr, Rect stPromptRect, unsigned char *pszContent, GUI_TEXT_ATTR stContentAttr, Rect stContentRect, const GUI_INPUTBOX_ATTR *pstType, const Rect *pstButtonRect, GUI_TEXT_ATTR stButtonAttr, int nButtonNo, int timeoutSec) {
    unsigned char szWorkBuff[255];
    unsigned char bClearInitData = pstType->bEchoMode ? 1 : 0;

    unsigned char bChkTimer;
    int iKey = GUI_ERR_TIMEOUT;
    unsigned char ucTimerFlag = FALSE; // Added by Kim_LinHB 2014/9/11 v0.7

    RectMap stRectMap[1];

    int iLen, iIndex, iLastKey;
    int i;

    if (pszPrompt) {
        DrawText(pszPrompt, (void *) &stPromptAttr, stPromptRect.top, stPromptRect.bottom, stPromptRect.left, stPromptRect.right, 0);
    }

    memset(szWorkBuff, 0, sizeof(szWorkBuff));
    if (strlen(pszContent) > 0) {
        strcpy((char *) szWorkBuff, pszContent);
    }

    iLen = strlen((char *) szWorkBuff);
    if (iLen > 0) {
        MapChar2Index(szWorkBuff[iLen - 1], &iLastKey, &iIndex);
    }
    else {
        iLastKey = -1;
        iIndex = 0;
    }

    if ((GUI_INPUT_NUM == pstType->eType || GUI_INPUT_MIX == pstType->eType) && pstType->bSensitive) {
        unsigned char szSensitive[255];

        memset(szSensitive, 0, sizeof(szSensitive));
        memset(szSensitive, '*', strlen(szWorkBuff));
        DrawText((void *) szSensitive, (void *) &stContentAttr, stContentRect.top, stContentRect.bottom, stContentRect.left, stContentRect.right, 0);
    }
    else {
        int nOffset = strlen(szWorkBuff) - (stContentRect.right - stContentRect.left) / sgFontS[stContentAttr.eFontSize].Width;
		if (nOffset < 0)
			nOffset = 0;

		DrawText((void *) (szWorkBuff + nOffset), (void *) &stContentAttr, stContentRect.top, stContentRect.bottom, stContentRect.left, stContentRect.right, 0);
    }

    for (i = 0; i < nButtonNo; ++i) { // total buttons on screen, 3 or 16
        DrawBgBox(pstButtonRect[i].left, pstButtonRect[i].top, pstButtonRect[i].right, pstButtonRect[i].bottom, sg_nDefBg);

        if(pstButtonRect[i].nValue != 0) {
            DrawRect(pstButtonRect[i].left, pstButtonRect[i].top, pstButtonRect[i].right, pstButtonRect[i].bottom, sg_nDefColor);
            DrawRect(pstButtonRect[i].left+ 1, pstButtonRect[i].top+ 1, pstButtonRect[i].right-1, pstButtonRect[i].bottom-1, sg_nDefColor);
        }

        if(KEY0 <= pstButtonRect[i].nValue && pstButtonRect[i].nValue <= KEY9) {
            unsigned char szCH[4 + 1] = {0};
            if( GUI_INPUT_MIX == pstType->eType) {
				strncpy(szCH, sgPhyKeyMap[i].Table, 4);
            }
            else {
				strncpy(szCH, sgPhyKeyMap[i].Table, 1);
            }
            DrawText((void *)szCH, (void *)&stButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
        }
        else
        {
            switch (pstButtonRect[i].nValue) {
                case KEYENTER:
                    DrawText((void *) GetKeyValuebyIndex(KEYENTER), (void *) &stButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
                    break;
                case KEYCANCEL:
                    DrawText((void *) GetKeyValuebyIndex(KEYCANCEL), (void *) &stButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
                    break;
                case KEYCLEAR:
                    DrawText((void *) GetKeyValuebyIndex(KEYCLEAR), (void *) &stButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
                    break;
                case KEYALPHA:
                    DrawText((void *) GetKeyValuebyIndex(KEYALPHA), (void *) &stButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
                    break;
            }
        }
    }

    if (timeoutSec >= 0) {
        bChkTimer = TRUE;
        TimerSet(GUI_TIMER_INDEX, (short) timeoutSec * 10);
    }
    else {
        bChkTimer = FALSE;
    }

    memset(stRectMap, 0, sizeof(stRectMap));
    stRectMap[0].pRect = (Rect *)pstButtonRect;
    stRectMap[0].no = nButtonNo;

    while (1) {
		GUI_CALLBACK vFunc = NULL;
        int iTouchStatus = 0;
        iKey = 0;

        if (ucTimerFlag && 0 == TimerCheck(GUI_TIMER_INPUT_TEMP) &&  // Modified by Kim_LinHB 2014-09-03 v0.5
                (GUI_INPUT_NUM == pstType->eType || GUI_INPUT_MIX == pstType->eType) && pstType->bSensitive) {
            unsigned char szSensitive[255];

            memset(szSensitive, 0, sizeof(szSensitive));
            memset(szSensitive, '*', strlen(szWorkBuff));
#ifdef _AR_MODE_
            if (sg_uiArabicFileID != AR_OPENFILE_ERROR) // Added by Kim_LinHB 2014-09-01 v0.4
                ClearScr(&stContentRect, 1);
#endif
            DrawText((void *) szSensitive, (void *) &stContentAttr, stContentRect.top, stContentRect.bottom, stContentRect.left, stContentRect.right, 0);
            ucTimerFlag = FALSE;
        }

		if((vFunc = GetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT))){
			int callbackLen = 0;
			iKey = vFunc(GUI_CALLBACK_LISTEN_EVENT, NULL, &callbackLen);
			if(iKey != GUI_OK)
				break;
		}
		iTouchStatus = GetVirtualKey(stRectMap, sizeof(stRectMap)/sizeof(stRectMap[0]));

        if (0 == kbhit()) {
            TimerSet(GUI_TIMER_INDEX, (short) timeoutSec * 10);
            iKey = getkey();
        }
        else if(iTouchStatus > 0) {
            TimerSet(GUI_TIMER_INDEX, (short)timeoutSec * 10);
			iKey = iTouchStatus;
        }
        else if (bChkTimer && 0 == TimerCheck(GUI_TIMER_INDEX)) {
            iKey = GUI_ERR_TIMEOUT;
        }
        else {
            continue;
        }

        if (KEYCANCEL == iKey) {
            return GUI_ERR_USERCANCELLED;
        }
        else if (GUI_ERR_TIMEOUT == iKey) {
            return GUI_ERR_TIMEOUT;
        }
        else if (KEYENTER == iKey) {
            char bOKToReturn = TRUE;
            bClearInitData = FALSE;
            if (iLen < (int) pstType->nMinLen) {
                Beep(); // Added by Kim 2014/12/07 bug529
                continue;
            }
            if (bOKToReturn) {
                if (pszContent)
                    sprintf((char *) pszContent, "%s", szWorkBuff);
                return GUI_OK;
            }
        }
        else if (iKey >= KEY0 && iKey <= KEY9) {
            if (bClearInitData) {   // clear in buffer
                memset(szWorkBuff, 0, sizeof(szWorkBuff));
                iLen = 0;
                iLastKey = -1;
                bClearInitData = FALSE;
                ClearScr(&stContentRect, 1);
            }
            // save key in data
            iLastKey = MapKey((unsigned char) iKey);
            iIndex = 0;

            if (iLen < (int) pstType->nMaxLen) {
                szWorkBuff[iLen++] = (unsigned char) iKey;
                szWorkBuff[iLen] = 0;
#ifdef _AR_MODE_
                if (sg_uiArabicFileID != AR_OPENFILE_ERROR) // Added by Kim_LinHB 2014-09-01 v0.4
                    ClearScr(&stContentRect, 1);
#endif
                TimerSet(GUI_TIMER_INPUT_TEMP, 4); // just for sensitive case

                if ((GUI_INPUT_NUM == pstType->eType || GUI_INPUT_MIX == pstType->eType) && pstType->bSensitive) {
                    unsigned char szSensitive[255];

                    memset(szSensitive, 0, sizeof(szSensitive));
                    strcpy(szSensitive, szWorkBuff);
                    ucTimerFlag = TRUE;

                    if (strlen(szWorkBuff) > 0) {
                        memset(szSensitive, '*', strlen(szWorkBuff) - 1);
                    }
                    DrawText((void *) szSensitive, (void *) &stContentAttr, stContentRect.top, stContentRect.bottom, stContentRect.left, stContentRect.right, 0);
                    continue; // Added by Kim_LinHB 2014-09-03 v0.5
                }
            }
            else {
                Beep(); // Added by Kim 2014/12/07 bug529
            }
        }
        else if (iKey == KEYCLEAR) {
            if (bClearInitData) {   // clear in buffer // Modified by Kim_LinHB 2014-8-8 v0.3
                memset(szWorkBuff, 0, sizeof(szWorkBuff));
                iLen = 1;
                iLastKey = -1;
                bClearInitData = FALSE;
                ClearScr(&stContentRect, 1);
            }

            if (iLen <= 0) {
                Beep(); // Added by Kim 2014/12/07 bug529
                continue;
            }
            szWorkBuff[--iLen] = 0;
            if (iLen > 0) {
                MapChar2Index(szWorkBuff[iLen - 1], &iLastKey, &iIndex);
            }
            else {
                iLastKey = -1;
                iIndex = 0;
            }
            ClearScr(&stContentRect, 1);
        }
        else if (iKey == KEYALPHA || iKey == KEYF2) {
            if (pstType->eType != GUI_INPUT_MIX) {
                continue;
            }
            if (bClearInitData) {   // clear in buffer // Modified by Kim_LinHB 2014-8-8 v0.3
                memset(szWorkBuff, 0, sizeof(szWorkBuff));
                iLen = 0;
                iLastKey = -1;
                bClearInitData = FALSE;
                ClearScr(&stContentRect, 1);
            }

            if (iLastKey < 0 || iLen < 1) {
                continue;
            }
#ifdef _AR_MODE_
            if (sg_uiArabicFileID != AR_OPENFILE_ERROR) // Added by Kim_LinHB 2014-09-01 v0.4
                ClearScr(&stContentRect, 1);
#endif

            if (GUI_INPUT_MIX == pstType->eType) {
				iIndex = (iIndex + 1) % strlen(sgPhyKeyMap[iLastKey].Table);
				szWorkBuff[iLen - 1] = sgPhyKeyMap[iLastKey].Table[iIndex];

                TimerSet(GUI_TIMER_INPUT_TEMP, 4); // just for sensitive case

                if (pstType->bSensitive) {
                    unsigned char szSensitive[255];

                    memset(szSensitive, 0, sizeof(szSensitive));
                    strcpy(szSensitive, szWorkBuff);
                    ucTimerFlag = TRUE;

                    if (strlen(szWorkBuff) > 0) {
                        memset(szSensitive, '*', strlen(szWorkBuff) - 1);
                    }
                    DrawText((void *) szSensitive, (void *) &stContentAttr, stContentRect.top, stContentRect.bottom, stContentRect.left, stContentRect.right, 0);
                    continue; // Added by Kim_LinHB 2014-09-03 v0.5
                }
            }
        }

        if ((GUI_INPUT_NUM == pstType->eType || GUI_INPUT_MIX == pstType->eType) && pstType->bSensitive) {
            unsigned char szSensitive[255];

            //if(1 >= TimerCheck(GUI_TIMER_INPUT_TEMP)){
            memset(szSensitive, 0, sizeof(szSensitive));
            memset(szSensitive, '*', strlen(szWorkBuff));
            DrawText((void *) szSensitive, (void *) &stContentAttr, stContentRect.top, stContentRect.bottom, stContentRect.left, stContentRect.right, 0);
            //}
        }
        else {
            int nOffset = strlen(szWorkBuff) - (stContentRect.right - stContentRect.left) / sgFontS[stContentAttr.eFontSize].Width;
			if (nOffset < 0)
				nOffset = 0;
			DrawText((void *) (szWorkBuff + nOffset), (void *) &stContentAttr, stContentRect.top, stContentRect.bottom, stContentRect.left, stContentRect.right, 0);
        }
    }
    return GUI_OK;
}

static int GetInputTime(Rect stCurrTimeRect, const Rect *pstTimeRect, GUI_TEXT_ATTR stTimeAttr, unsigned char *pszTime, const Rect *pstButtonRect, unsigned char nButtonsNo, GUI_TEXT_ATTR stButtonAttr, unsigned char isTime, int timeoutSec) {
    unsigned char sTimeBCD[7];
    unsigned char szCurrTime[12 + 1];
    unsigned char szSettingTime[14 + 1];
    const unsigned char szSetTimeFormat[] = "YYYYMMDDhhmm";
    unsigned char sLastTime[2] = { 0x00, 0x00 };
    unsigned char szBuff[3][4 + 1]; // year ....  just for display
    unsigned char nMin[3] = { 0, 0, 0 };
    unsigned char nMax[3] = { 2, 2, 2 };
    unsigned char nOffset[3] = { 8, 10, 12 };

    unsigned char bChkTimer;
    int iKey = GUI_ERR_TIMEOUT;
    unsigned char bClearInitData = TRUE;

    unsigned char ucSelected = 0;
    unsigned char nCurrLen = 0;

    RectMap stRectMap[2];

    int nTotal = isTime ? 2 : 3;
    int i;

    if (!isTime) { //year
        nMin[0] = 2;
        nMax[0] = 4;
        nOffset[0] = 0;
        nOffset[1] = 4;
        nOffset[2] = 6;
    }

    GetTime(sTimeBCD);
    sprintf(szSettingTime, "20%02X%02X%02X%02X%02X%02X", sTimeBCD[0], sTimeBCD[1], sTimeBCD[2], sTimeBCD[3], sTimeBCD[4], sTimeBCD[5]);

    if (!isTime) {
        sprintf(szCurrTime, "20%02X-%02X-%02X", sTimeBCD[0], sTimeBCD[1], sTimeBCD[2]);
        sprintf(szBuff[0], "20%02X", sTimeBCD[0]);
        sprintf(szBuff[1], "%02X", sTimeBCD[1]);
        sprintf(szBuff[2], "%02X", sTimeBCD[2]);
    }
    else {
        sprintf(szCurrTime, "%02X:%02X", sTimeBCD[3], sTimeBCD[4]);
        sprintf(szBuff[0], "%02X", sTimeBCD[3]);
        sprintf(szBuff[1], "%02X", sTimeBCD[4]);
    }
    DrawText(szCurrTime, (void *) &stTimeAttr, stCurrTimeRect.top, stCurrTimeRect.bottom, stCurrTimeRect.left, stCurrTimeRect.right, 0);

    for (i = 0; i < nButtonsNo; ++i) { // cancel & ok
        DrawBgBox(pstButtonRect[i].left, pstButtonRect[i].top, pstButtonRect[i].right, pstButtonRect[i].bottom, sg_nDefBg);

        if(pstButtonRect[i].nValue != 0) {
            DrawRect(pstButtonRect[i].left, pstButtonRect[i].top, pstButtonRect[i].right, pstButtonRect[i].bottom, sg_nDefColor);
            DrawRect(pstButtonRect[i].left+ 1, pstButtonRect[i].top+ 1, pstButtonRect[i].right-1, pstButtonRect[i].bottom-1, sg_nDefColor);
        }

		if(KEY0 <= pstButtonRect[i].nValue && pstButtonRect[i].nValue <= KEY9) {
			unsigned char szCH[4 + 1] = {0};
			strncpy(szCH, sgPhyKeyMap[i].Table, 1);
			DrawText((void *)szCH, (void *)&stButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
		}
		else
		{
			switch (pstButtonRect[i].nValue) {
			case KEYENTER:
				DrawText((void *) GetKeyValuebyIndex(KEYENTER), (void *) &stButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
				break;
			case KEYCANCEL:
				DrawText((void *) GetKeyValuebyIndex(KEYCANCEL), (void *) &stButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
				break;
			case KEYCLEAR:
				DrawText((void *) GetKeyValuebyIndex(KEYCLEAR), (void *) &stButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
				break;
			case KEYALPHA:
				DrawText((void *) GetKeyValuebyIndex(KEYALPHA), (void *) &stButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
				break;
			}
		}
    }

    {
        int j;
        GUI_TEXT_ATTR stReversal = stTimeAttr;
        if (stReversal.eStyle & GUI_FONT_REVERSAL) {
            stReversal.eStyle &= ~GUI_FONT_REVERSAL;
        }
        else {
            stReversal.eStyle |= (GUI_FONT_REVERSAL | GUI_FONT_OPAQUE);
        }
        for (j = 0; j < nTotal; ++j) {
            if ( j == ucSelected) {
                DrawText((void *) szBuff[j], (void *) &stReversal, pstTimeRect[j].top, pstTimeRect[j].bottom, pstTimeRect[j].left, pstTimeRect[j].right, 1);
            }
            else {
                DrawText((void *) szBuff[j], (void *) &stTimeAttr, pstTimeRect[j].top, pstTimeRect[j].bottom, pstTimeRect[j].left, pstTimeRect[j].right, 1);
            }
        }
    }

    if (timeoutSec >= 0) {
        bChkTimer = TRUE;
        TimerSet(GUI_TIMER_INDEX, (short) timeoutSec * 10);
    }
    else {
        bChkTimer = FALSE;
    }

    nCurrLen = nMax[ucSelected];

    memset(sLastTime, 0, sizeof(sLastTime));

    memset(stRectMap, 0, sizeof(stRectMap));
    stRectMap[0].pRect = (Rect *)pstTimeRect;
    stRectMap[0].no = nTotal;

    stRectMap[1].pRect = (Rect *)pstButtonRect;
    stRectMap[1].no = nButtonsNo;

    while (1) {
		GUI_CALLBACK vFunc = NULL;
        int iTouchStatus = 0;
        iKey = 0;

        GetTime(sTimeBCD);
        if (memcmp(sLastTime, &sTimeBCD[3], 2) != 0) {
            memcpy(sLastTime, &sTimeBCD[3], 2);
            if (!isTime) {
                sprintf(szCurrTime, "20%02X-%02X-%02X", sTimeBCD[0], sTimeBCD[1], sTimeBCD[2]);
            }
            else {
                sprintf(szCurrTime, "%02X:%02X", sTimeBCD[3], sTimeBCD[4]);
            }
            DrawText(szCurrTime, (void *) &stTimeAttr, stCurrTimeRect.top, stCurrTimeRect.bottom, stCurrTimeRect.left, stCurrTimeRect.right, 0);
        }

		if((vFunc = GetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT))){
			int callbackLen = 0;
			iKey = vFunc(GUI_CALLBACK_LISTEN_EVENT, NULL, &callbackLen);
			if(iKey != GUI_OK)
				break;
		}

		iTouchStatus = GetVirtualKey(stRectMap, sizeof(stRectMap)/sizeof(stRectMap[0]));

        if (0 == kbhit()) {
            TimerSet(GUI_TIMER_INDEX, (short) timeoutSec * 10);
            iKey = getkey();
        }
        else if(iTouchStatus > 0) {
            TimerSet(GUI_TIMER_INDEX, (short)timeoutSec * 10);
			iKey = iTouchStatus;
        }
        else if (bChkTimer && 0 == TimerCheck(GUI_TIMER_INDEX)) {
            iKey = GUI_ERR_TIMEOUT;
        }
        else {
            continue;
        }

        if (KEYCANCEL == iKey) {
            return GUI_ERR_USERCANCELLED;
        }
        else if (GUI_ERR_TIMEOUT == iKey) {
            return GUI_ERR_TIMEOUT;
        }
        else if (KEYENTER == iKey) {
            if (nCurrLen < nMax[ucSelected]) {
                Beep();
            }
            else {
                unsigned char szTmpTime[15 + 1];
                strcpy(szTmpTime, szSettingTime);
                memcpy(szTmpTime + nOffset[ucSelected], szBuff[ucSelected], nMax[ucSelected]);
                if (0 == IsValidTime(szTmpTime, szSetTimeFormat)) {
                    if (!isTime) {
                        szTmpTime[8] = 0;
                        if (pszTime)
                            strcpy(pszTime, szTmpTime + 2);
                    }
                    else {
                        szTmpTime[12] = 0;
                        if (pszTime)
                            strcpy(pszTime, szTmpTime + 8);
                    }
                    return GUI_OK;
                }
            }
        }
        else if (KEYALPHA == iKey || KEYF2 == iKey || iKey >= KEYALPHA +1000) { //for keypad input
			char toSwitch = 1;
			ClearScr(&pstTimeRect[ucSelected], 1);
			DrawBgBox(pstTimeRect[ucSelected].left, pstTimeRect[ucSelected].top, pstTimeRect[ucSelected].right, pstTimeRect[ucSelected].bottom, sg_nDefBg);
			DrawText((void *)szBuff[ucSelected], (void *)&stTimeAttr,
				pstTimeRect[ucSelected].top, pstTimeRect[ucSelected].bottom, pstTimeRect[ucSelected].left, pstTimeRect[ucSelected].right, 1);

			if(iKey >= KEYALPHA +1000){
				toSwitch = (iKey - KEYALPHA -1000) != ucSelected;
			}
			if (toSwitch && nCurrLen >= nMax[ucSelected]) {
				unsigned char szTmpTime[15 + 1];
				strcpy(szTmpTime, szSettingTime);

				memcpy(szTmpTime + nOffset[ucSelected], szBuff[ucSelected], nMax[ucSelected]);
				if (0 == IsValidTime(szTmpTime, szSetTimeFormat)) {
					ClearScr(&pstTimeRect[ucSelected], 1);
					if(iKey >= KEYALPHA +1000)
						ucSelected = iKey - KEYALPHA -1000;
					else
						ucSelected = ucSelected + 1 >= nTotal ? 0 : ucSelected + 1;
					nCurrLen = nMax[ucSelected];
					strcpy(szSettingTime, szTmpTime);
					bClearInitData = TRUE;
				}
				else {
					nCurrLen = nMax[ucSelected];
					sprintf(szBuff[ucSelected], "%.*s", nMax[ucSelected], szSettingTime + nOffset[ucSelected]);
				}
			}
		}
        else if (KEY0 <= iKey && iKey <= KEY9) {
            if (bClearInitData) {   // clear in buffer
                szBuff[ucSelected][nMin[ucSelected]] = 0;
                nCurrLen = nMin[ucSelected];
                bClearInitData = FALSE;
            }

            if (nCurrLen < nMax[ucSelected]) {
                szBuff[ucSelected][nCurrLen++] = (unsigned char) iKey;
                szBuff[ucSelected][nCurrLen] = 0;
            }

            if (nCurrLen == nMax[ucSelected]) {
                unsigned char szTmpTime[15 + 1];
                strcpy(szTmpTime, szSettingTime);

                memcpy(szTmpTime + nOffset[ucSelected], szBuff[ucSelected], nMax[ucSelected]);
                if (0 == IsValidTime(szTmpTime, szSetTimeFormat)) {
                    ClearScr(&pstTimeRect[ucSelected], 1);
                    ucSelected = ucSelected + 1 >= nTotal ? 0 : ucSelected + 1;
                    nCurrLen = nMax[ucSelected];
                    strcpy(szSettingTime, szTmpTime);
                }
                else {
                    sprintf(szBuff[ucSelected], "%.*s", nMax[ucSelected], szSettingTime + nOffset[ucSelected]);
                }
                bClearInitData = TRUE;
            }
        }
        else if (iKey == KEYCLEAR) {
            bClearInitData = FALSE;
            if (nCurrLen <= nMin[ucSelected]) {
                continue;
            }
            szBuff[ucSelected][--nCurrLen] = 0;
        }

        ClearScr(&pstTimeRect[ucSelected], 1);

        {
            int j;
            GUI_TEXT_ATTR stReversal = stTimeAttr;
            if (stReversal.eStyle & GUI_FONT_REVERSAL) {
                stReversal.eStyle &= ~GUI_FONT_REVERSAL;
            }
            else {
                stReversal.eStyle |= (GUI_FONT_REVERSAL | GUI_FONT_OPAQUE);
            }
            for (j = 0; j < nTotal; ++j) { //date 3 or time 2
                if (j == ucSelected) {
                    DrawText((void *) szBuff[j], (void *) &stReversal, pstTimeRect[j].top, pstTimeRect[j].bottom, pstTimeRect[j].left, pstTimeRect[j].right, 1);
                }
                else {
                    DrawText((void *) szBuff[j], (void *) &stTimeAttr, pstTimeRect[j].top, pstTimeRect[j].bottom, pstTimeRect[j].left, pstTimeRect[j].right, 1);
                }
            }
        }
    }

    return GUI_OK;
}

static void ClearScr(const Rect *rect, char IsClearBg) {
    if (!sg_isColorScreen) {
        if (!rect) {
            ScrCls();
        }
        else {
            ScrClrRect(rect->left, rect->top, rect->right, rect->bottom);
        }
    }

    if (!sg_bStartup && sg_isColorScreen) {
        ST_LCD_INFO stLcdInfo;
        CLcdGetInfo (&stLcdInfo);

        if(!rect) {
            CLcdClrRect(0, 0, stLcdInfo.width-1, stLcdInfo.height-1, 0);
            if(IsClearBg)
            CLcdSetBgColor(sg_nDefBg);
        }
        else {
            Rect stTmp;
            memcpy(&stTmp, rect, sizeof(Rect));
            if (stTmp.left < 0) {
                stTmp.left = 0;
            }
            if ((unsigned int)stTmp.right >= stLcdInfo.width) {
                stTmp.right = stLcdInfo.width - 1;
            }
            if (stTmp.top < 0) {
                stTmp.top = 0;
            }
            if ((unsigned int)stTmp.bottom >= stLcdInfo.height) {
                stTmp.bottom = stLcdInfo.height - 1;
            }
            if(IsClearBg)
            DrawBgBox(stTmp.left, stTmp.top, stTmp.right, stTmp.bottom, sg_nDefBg);
            CLcdClrRect(stTmp.left, stTmp.top, stTmp.right, stTmp.bottom, 0);
        }
    }
}

static void UpdateCanvasSize() {
	if(sg_isColorScreen) {
        ST_LCD_INFO stLcdInfo;
        CLcdGetInfo(&stLcdInfo);
        sg_nScrWidth = stLcdInfo.width;
        sg_nScrHeight = stLcdInfo.height;
    }
	else{
		ScrGetLcdSize(&sg_nScrWidth, &sg_nScrHeight);
	}
}

static void DrawText(const void *vRes, const void *vAttr, int top, int bottom, int left, int right, char isButton) {
    unsigned char szOneLine[1024];
    GUI_TEXT_ATTR stTextAttr;
    int nLeft = left, nTop = top;
    unsigned char *pStart = (unsigned char *) vRes, *pEnd = pStart;
    unsigned int nLine = 1;

    memcpy(&stTextAttr, vAttr, sizeof(GUI_TEXT_ATTR));

#ifdef _AR_MODE_
    if (sgFontM[stTextAttr.eFontSize].CharSet != CHARSET_ARABIA && AR_OPENFILE_ERROR == sg_uiArabicFileID) {
#endif
        ScrSelectFont(&sgFontS[stTextAttr.eFontSize], &sgFontM[stTextAttr.eFontSize]);

        if (!sg_isColorScreen) {
            if (stTextAttr.eStyle & GUI_FONT_REVERSAL) {
                int i = 0;
                ScrAttrSet(sg_nDefBg < 0xFF808080);
                for (i = top; i < bottom; i += 2) {
                    ScrDrawRect(left, i, right, i + 1);
                }
                ScrAttrSet(sg_nDefBg >= 0xFF808080);
            }
            else {
                ScrAttrSet(sg_nDefBg < 0xFF808080);
                ScrClrRect(left, top, right, bottom);
            }
        }
        else if (stTextAttr.eStyle & GUI_FONT_OPAQUE) {
            if (stTextAttr.eStyle & GUI_FONT_REVERSAL) {
                DrawBgBox(left, top, right, bottom, _GetReverseColor(sg_nDefBg));
                CLcdSetFgColor(_GetReverseColor(sg_nDefColor));
            }
            else {
                DrawBgBox(left, top, right, bottom, sg_nDefBg);
                CLcdSetFgColor(sg_nDefColor);
            }
        }
        else{
            // Bug 689
            CLcdSetFgColor(sg_nDefColor);
        }

        while (pEnd = strchr(pStart, '\n')) { // count total lines
            pStart = pEnd + 1;
            ++nLine;
        }

        pStart = pEnd = (unsigned char *) vRes;

        nTop += (int) (bottom - top - nLine * GetFontHeight(sgFontS[stTextAttr.eFontSize], sgFontM[stTextAttr.eFontSize])) / 2;
        if (nTop < top)
            nTop = top;

        do {
            int width;
            pEnd = strchr(pStart, '\n');
            if (pEnd) {
                memcpy(szOneLine, pStart, pEnd - pStart);
                szOneLine[pEnd - pStart] = 0;
            }
            else {
                strcpy(szOneLine, pStart);
            }
			if(!isButton){
				GUI_CALLBACK vFunc = NULL;
				if((vFunc = GetCallbackEvent(GUI_CALLBACK_UPDATE_TEXT))){
					struct _Gui_Callback_Text stText = {
						0,0,stTextAttr.eFontSize, NULL
					};
					int callbackLen = sizeof(stText);

					stText.pStr = szOneLine;
					vFunc(GUI_CALLBACK_UPDATE_TEXT, &stText, &callbackLen);
					if(stText.y != nTop){
						nTop += stText.y;
					}
				}
			}

			//fix by Kim bug 814 816 817
			width = GetStrPix((unsigned char *) szOneLine, &sgFontS[stTextAttr.eFontSize], &sgFontM[stTextAttr.eFontSize], 0);

            nLeft = left;

            switch (stTextAttr.eAlign) {
                case GUI_ALIGN_RIGHT:
                    nLeft = right - width;
                    break;
                case GUI_ALIGN_CENTER:
                    nLeft += (right - left - width) / 2;
                    break;
            }

            if (nLeft < left)
                nLeft = left;

            if (pStart != vRes) {
                nTop += GetFontHeight(sgFontS[stTextAttr.eFontSize], sgFontM[stTextAttr.eFontSize]);
                if (nTop >= bottom)
                    break;
            }

            if(!sg_isColorScreen) {
                szOneLine[sg_nScrWidth/sgFontS[stTextAttr.eFontSize].Width] = 0;
                ScrTextOut(nLeft, nTop, (unsigned char *)szOneLine);
            }
            else {
//      if(stTextAttr.eStyle & GUI_FONT_BOLD){
//          CLcdTextOut(nLeft, nTop-1, (char *)vRes);
//          CLcdTextOut(nLeft, nTop, (char *)vRes);
//          CLcdTextOut(nLeft, nTop + 1, (char *)vRes);
//      }
//      else
                {
                    CLcdTextOut(nLeft, nTop, (char *)szOneLine);
                }
            }
            if (pEnd) {
                pStart = pEnd + 1;
            }
        } while (pEnd);

        if (sg_isColorScreen) {
            CLcdSetFgColor(sg_nDefColor);
        }
        else {
            ScrAttrSet(sg_nDefBg < 0xFF808080);
        }
#ifdef _AR_MODE_
    }
    else { // CHARSET_ARABIA
// Added by Kim_LinHB 2014-8-7 v0.3
        ArScrFontSelect(sg_uiArabicFileID, sgFontM[stTextAttr.eFontSize].Height);

        if (!sg_isColorScreen) {
            ArScrReverse(stTextAttr.eStyle & GUI_FONT_REVERSAL, 0);
        }
        else if (stTextAttr.eStyle & GUI_FONT_OPAQUE) {
            if (stTextAttr.eStyle & GUI_FONT_REVERSAL) {
                DrawBgBox(left, top, right, bottom, _GetReverseColor(sg_nDefBg));
                CLcdSetFgColor(_GetReverseColor(sg_nDefColor));
            }
            else {
                DrawBgBox(left, top, right, bottom, sg_nDefBg);
                CLcdSetFgColor(sg_nDefColor);
            }
        }

        while (pEnd = strchr(pStart, '\n')) { // count total lines
            pStart = pEnd + 1;
            ++nLine;
        }

        pStart = pEnd = (unsigned char *) vRes;

        nTop += (int) (bottom - top - nLine * GetFontHeight(sgFontS[stTextAttr.eFontSize], sgFontM[stTextAttr.eFontSize])) / 2;
        if (nTop < top)
            nTop = top;

        do {
            int width;
            pEnd = strchr(pStart, '\n');
            if (pEnd) {
                memcpy(szOneLine, pStart, pEnd - pStart);
                szOneLine[pEnd - pStart] = 0;
            }
            else {
                strcpy(szOneLine, pStart);
            }
            width = ArScrGetStrWidth(szOneLine);

            nLeft = left;

            switch (stTextAttr.eAlign) {
                case GUI_ALIGN_LEFT:
                    nLeft = right - width;
                    break;
                case GUI_ALIGN_CENTER:
                    nLeft += (right - left - width) / 2;
                    break;
            }

            if (nLeft < left)
                nLeft = left;

            if (pStart != vRes) {
                nTop += GetFontHeight(sgFontS[stTextAttr.eFontSize], sgFontM[stTextAttr.eFontSize]);
                if (nTop >= bottom)
                    break;
            }

//szOneLine[sg_nScrWidth/sgFontS[stTextAttr.eFontSize].Width] = 0;
            ArScrAlign(AR_ALIGN_LEFT);
            ArScrLeftIndent(0);
            ArScrRightIndent(0);
            ArScrDoubleWidth(0);
            ArScrDoubleHeight(0);
            ArScrLineSpacing(0);
            ArScrLineHeightOptimize(0, 0);
//FIXME Kim wait until farsi lib update, special process
//20141110 Kim updated, it will not happen cuz R&D rejected it
// Added by Kim_LinHB 2014-09-01 v0.4
            if (nLeft < 96) {
                ArScrAlign(AR_ALIGN_LEFT);
                ArScrPrintAtXY(nLeft, nTop, szOneLine);
            }
            else if ((nLeft < 96 * 2 - 8) && nLeft >= 96) {
                ArScrAlign(AR_ALIGN_CENTER);
                ArScrPrintAtXY(nLeft - 96, nTop, szOneLine);
            }
            else {
                ArScrAlign(AR_ALIGN_RIGHT);
                ArScrRightIndent(sg_nScrWidth - nLeft - width);
                ArScrPrintAtXY(0, nTop, szOneLine);
            }
// Add End

            if (pEnd) {
                pStart = pEnd + 1;
            }
        } while (pEnd);

        if (sg_isColorScreen) {
            CLcdSetFgColor(sg_nDefColor);
        }
        else {
            ScrAttrSet(GUI_BLACK == sg_nDefBg);
        }
    }
#endif
}
static void DrawLogo(const void *vRes, int top, int left) {
    if (sg_isColorScreen) {
        CLcdSetFgColor(sg_nDefColor);
    }
    ScrGotoxyEx(left, top);
    ScrDrLogo((unsigned char *) vRes);
}

static void DrawImage(const void *vRes, int top, int left) {
    if (sg_isColorScreen) {
        CLcdBgDrawImg(left, top, (char *)vRes);
    }
}

static void DrawBgBox(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, unsigned int color) {
    if (sg_isColorScreen) {
        if (right >= (unsigned int) sg_nScrWidth)
            right = sg_nScrWidth - 1;

        if (bottom >= (unsigned int) sg_nScrHeight)
            bottom = sg_nScrHeight - 1;

        CLcdBgDrawBox(left, top, right, bottom, color);
    }
}

static void DrawRect(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, unsigned int color) {
    if (right >= (unsigned int) sg_nScrWidth)
        right = sg_nScrWidth - 1;

    if (bottom >= (unsigned int) sg_nScrHeight)
        bottom = sg_nScrHeight - 1;

    if (sg_isColorScreen) {
        CLcdDrawRect(left, top, right, bottom, color);
    }
    else {
        ScrDrawRect(left, top, right, bottom);
    }
}

static void DrawSign(const Rect *area){
	static TS_POINT_T sgPoint = {0, 0};
	TS_POINT_T stPoint;
	if(1 == TouchScreenRead(&stPoint, 1) && 100 == stPoint.pressure) {
		if(area && _IsIncluded(stPoint.x, stPoint.y, area->top, area->bottom, area->left, area->right) && sgPoint.x != stPoint.x && sgPoint.y != stPoint.y){
			if(_IsIncluded(sgPoint.x, sgPoint.y,  area->top, area->bottom, area->left, area->right)){
				DrawLine(sgPoint.x, sgPoint.y, stPoint.x, stPoint.y, sg_nDefColor);
				DrawLine(sgPoint.x, sgPoint.y +1 , stPoint.x+1, stPoint.y +1 , sg_nDefColor);
				DrawLine(sgPoint.x+1, sgPoint.y, stPoint.x+1, stPoint.y, sg_nDefColor);
			}
			memcpy(&sgPoint, &stPoint, sizeof(TS_POINT_T));
			sgSignData.point_array[sgSignData.point_len].x = stPoint.x;
			sgSignData.point_array[sgSignData.point_len].y = stPoint.y;
			++sgSignData.point_len;
			CLcdDrawPixel(stPoint.x, stPoint.y, sg_nDefColor);
			CLcdDrawPixel(stPoint.x, stPoint.y +1 , sg_nDefColor);
			CLcdDrawPixel(stPoint.x+1, stPoint.y, sg_nDefColor);
		}
	}
	else{
		memset(&sgPoint, 0, sizeof(TS_POINT_T));
	}
};

static void DrawLine(int x1, int y1, int x2, int y2, unsigned int color)
{
    int i = 0;
    int count = 0;
    int calo;
    int c = 0;
	int is_x_increase = (x1 - x2) < 0 ? 1 : -1;
	int is_y_increase = (y1 - y2) < 0 ? 1: -1;

    if (x1 == x2)
    {
        if (y1 == y2)
        {
            CLcdDrawPixel(x1, y1, color);
        }
        else
        {
            if (y1 - y2 > 0)
            {
                c = y1;
                y1 = y2;
                y2 = c;
                /*
                y1 ^= y2;
                y2 ^= y1;
                y1 ^= y2;
                */
            }
            for (c = y1; c <= y2; c++)
            {
                CLcdDrawPixel(x1,c, color);
            }
        }
        return;
    }
    else if (y1 == y2)
    {
        if (x1 - x2 > 0)
        {
            c = x1;
            x1 = x2;
            x2 = c;
         }
         for (c = x1; c <= x2; c++)
         {
            CLcdDrawPixel(c,y1, color);
         }
         return;
    }
    
    calo = (x1 - x2) / (y1 - y2) == 0 ? 1 : 0;
    count = (x1 - x2) / (y1 - y2) == 0 ? y1 - y2 : x1 - x2;
      
    for (i = 0; i <=count || i <= count * (-1) ; i++)
    {
        if (calo == 0)
        {
           CLcdDrawPixel(x1 + i * is_x_increase, y1 + i * (y2 - y1) / (x2 - x1) * is_x_increase, color);
        }
        else
        {
           CLcdDrawPixel(x1 + i * (x2 - x1) / (y2 - y1) * is_y_increase, y1 + i * is_y_increase, color);
        }
    }
    return;
}

// Modified by Kim_LinHB 2014-7-31 v0.2
// return key value
// Modified by Kim_LinHB 2014/10/8 v0.9
static int GetVirtualKey(const RectMap *pRectMap, unsigned int nMapSize)
{
	// 0 : idle     1 : pressed     2 : pressing    3 : move    4 : released
    //const unsigned short nLongPressCnt = 10; //1 sec
    static TS_POINT_T sg_pt;
    static unsigned char sg_pt_state = 0;
    static Rect sg_rect_bak = {0};
    TS_POINT_T stPoint;

    unsigned int iLoop;
    Rect *pRect = NULL;

    if(!sg_hasTouchScreen || !pRectMap || 0 == nMapSize) {
        return 0;
    }

    switch(sg_pt_state) {
        case 0:
// Hided by Kim_LinHB 2014-8-4
        if(1 == TouchScreenRead(&stPoint, 1) && 100 == stPoint.pressure) {
//TouchScreenFlush();
            sg_pt_state = 1;
            //TimerSet(GUI_TIMER_INDEX, nLongPressCnt);// for long press

            memcpy(&sg_pt, &stPoint, sizeof(TS_POINT_T));
            memset(&sg_rect_bak, 0, sizeof(Rect));
        }
//else keep 0
        break;
        case 1:
        if(1 == TouchScreenRead(&stPoint, 1) && 100 == stPoint.pressure) {
            TouchScreenFlush();

            if(0 == memcmp(&sg_pt, &stPoint, sizeof(TS_POINT_T))) {
                //if(0 == TimerCheck(GUI_TIMER_INDEX)) {
                    //sg_pt_state = 2;
                    //TimerSet(GUI_TIMER_INDEX, nLongPressCnt); // for long press
                    break;
                //}
// else keep 1 continue to updating UI
            }
            else {
                sg_pt_state = 3;
// continue to updating UI & state
            }

            for(iLoop = 0; iLoop < nMapSize; ++iLoop) {
                if(pRectMap[iLoop].pRect) {
                    pRect = (Rect *)FindMatchedButton(&stPoint, pRectMap[iLoop].pRect, pRectMap[iLoop].no);
                    if(pRect) {
                        break;
                    }
                }
            }
            if(pRect) {
                if(memcmp(pRect, &sg_rect_bak, sizeof(Rect))) { // moving
                    DrawBgBox(pRect->left, pRect->top, pRect->right, pRect->bottom, _GetReverseColor(sg_nDefBg));
                    DrawBgBox(sg_rect_bak.left, sg_rect_bak.top, sg_rect_bak.right, sg_rect_bak.bottom, sg_nDefBg);
                    memcpy(&sg_rect_bak, pRect, sizeof(Rect));
                    sg_pt_state = 3;
                }
                else { // moving, but in the same button rect
                    DrawBgBox(pRect->left, pRect->top, pRect->right, pRect->bottom, _GetReverseColor(sg_nDefBg));
                    if(0 == TimerCheck(GUI_TIMER_INDEX)) {
                        sg_pt_state = 2;
                        //TimerSet(GUI_TIMER_INDEX, nLongPressCnt); // for long press
                    }
                    else {
                        sg_pt_state = 1; // update if state is 3
                    }
                }
            }
            else
            {
				if(!_IsIncluded(stPoint.x, stPoint.y, sg_rect_bak.top, sg_rect_bak.bottom, sg_rect_bak.left, sg_rect_bak.right)) {
					DrawBgBox(sg_rect_bak.left, sg_rect_bak.top, sg_rect_bak.right, sg_rect_bak.bottom, sg_nDefBg);
				}
				else {
					DrawBgBox(sg_rect_bak.left, sg_rect_bak.top, sg_rect_bak.right, sg_rect_bak.bottom, _GetReverseColor(sg_nDefBg));
				}
            }

            memcpy(&sg_pt, &stPoint, sizeof(TS_POINT_T));
        }
        else {
            sg_pt_state = 4;
        }
        break;
        case 2:
        if(1 == TouchScreenRead(&stPoint, 1) && 100 == stPoint.pressure) {
           // TimerSet(GUI_TIMER_INDEX, nLongPressCnt); // for long press
            TouchScreenFlush();

            if(0 == memcmp(&sg_pt, &stPoint, sizeof(TS_POINT_T))) {
                sg_pt_state = 2;
                break;
// keep 2
            }
            else {
                sg_pt_state = 1;
// continue to updating UI & state
            }

            for(iLoop = 0; iLoop < nMapSize; ++iLoop) {
                pRect = (Rect *)FindMatchedButton(&stPoint, pRectMap[iLoop].pRect, pRectMap[iLoop].no);
                if(pRect) {
                    break;
                }
            }
            if(pRect) {
                if(memcmp(pRect, &sg_rect_bak, sizeof(Rect))) { // moving
                    DrawBgBox(pRect->left, pRect->top, pRect->right, pRect->bottom, _GetReverseColor(sg_nDefBg));
                    DrawBgBox(sg_rect_bak.left, sg_rect_bak.top, sg_rect_bak.right, sg_rect_bak.bottom, sg_nDefBg);
                    memcpy(&sg_rect_bak, pRect, sizeof(Rect));
                    sg_pt_state = 1;
                    break;
                }
                else { // moving, but in the same button rect
                    sg_pt_state = 2;// update if state is 1
                    break;
                }
            }
            else
			{
				if(!_IsIncluded(stPoint.x, stPoint.y, sg_rect_bak.top, sg_rect_bak.bottom, sg_rect_bak.left, sg_rect_bak.right)) {
					DrawBgBox(sg_rect_bak.left, sg_rect_bak.top, sg_rect_bak.right, sg_rect_bak.bottom, sg_nDefBg);
				}
				else {
					DrawBgBox(sg_rect_bak.left, sg_rect_bak.top, sg_rect_bak.right, sg_rect_bak.bottom, _GetReverseColor(sg_nDefBg));
				}
            }

            memcpy(&sg_pt, &stPoint, sizeof(TS_POINT_T));
        }
        else {
            sg_pt_state = 4;
        }
        break;
        case 3:
        if(1 == TouchScreenRead(&stPoint, 1) && 100 == stPoint.pressure) {
            //TimerSet(GUI_TIMER_INDEX, nLongPressCnt);
            TouchScreenFlush();

            if(0 == memcmp(&sg_pt, &stPoint, sizeof(TS_POINT_T))) {
                sg_pt_state = 1;
// stop moving, continue to updating UI & state
            }
            else {
                sg_pt_state = 3;
// continue to updating UI & state
            }

            for(iLoop = 0; iLoop < nMapSize; ++iLoop) {
                pRect = (Rect *)FindMatchedButton(&stPoint, pRectMap[iLoop].pRect, pRectMap[iLoop].no);
                if(pRect) {
                    break;
                }
            }
            if(pRect) {
                if(memcmp(pRect, &sg_rect_bak, sizeof(Rect))) { // moving
                    DrawBgBox(pRect->left, pRect->top, pRect->right, pRect->bottom, _GetReverseColor(sg_nDefBg));
                    DrawBgBox(sg_rect_bak.left, sg_rect_bak.top, sg_rect_bak.right, sg_rect_bak.bottom, sg_nDefBg);
                    memcpy(&sg_rect_bak, pRect, sizeof(Rect));
                    sg_pt_state = 3;
                }
                else {
                    DrawBgBox(pRect->left, pRect->top, pRect->right, pRect->bottom, _GetReverseColor(sg_nDefBg));
                    sg_pt_state = 1; // update if state is 3
                }
            }
            else
            {
                if(!_IsIncluded(stPoint.x, stPoint.y, sg_rect_bak.top, sg_rect_bak.bottom, sg_rect_bak.left, sg_rect_bak.right)) {
					DrawBgBox(sg_rect_bak.left, sg_rect_bak.top, sg_rect_bak.right, sg_rect_bak.bottom, sg_nDefBg);
				}
				else {
					DrawBgBox(sg_rect_bak.left, sg_rect_bak.top, sg_rect_bak.right, sg_rect_bak.bottom, _GetReverseColor(sg_nDefBg));
				}
            }

            memcpy(&sg_pt, &stPoint, sizeof(TS_POINT_T));
        }
        else {
            sg_pt_state = 4;
        }
        break;
		case 4:
			if(_IsIncluded(sg_pt.x, sg_pt.y, sg_rect_bak.top, sg_rect_bak.bottom, sg_rect_bak.left, sg_rect_bak.right)){
				int keyValue = sg_rect_bak.nValue;
				memset(&sg_pt, 0, sizeof(TS_POINT_T));
				sg_pt_state = 0;
				if(pRect) {
					DrawBgBox(pRect->left, pRect->top, pRect->right, pRect->bottom, sg_nDefBg);
				}
				else {
					DrawBgBox(sg_rect_bak.left, sg_rect_bak.top, sg_rect_bak.right, sg_rect_bak.bottom, sg_nDefBg);
					memset(&sg_rect_bak, 0, sizeof(Rect));
				}
				return keyValue;
			}
			else{
				memset(&sg_pt, 0, sizeof(TS_POINT_T));
				memset(&sg_rect_bak, 0, sizeof(Rect));
				sg_pt_state = 0;
			}
    }
	
//for test
//ScrPrint(0,0,0, "%d:[%d,%d]-%d", sg_pt_state, sg_pt.x, sg_pt.y, sg_pt.pressure);
//ScrPrint(0,2,0, "%d:%d:%d:%d", sg_rect_bak.top, sg_rect_bak.bottom, sg_rect_bak.left, sg_rect_bak.right);

	return 0;
}

/*special return value 1*/
static int GetMenuItem(const GUI_MENU *pstMenu, enum MENUSTYLE eMode, Rect stMenuRect, const Rect *pstButtonRect, unsigned char nButtonNo, const GUI_TEXT_ATTR *pstButtonAttr, int timeoutSec, int *piSelected) {
	int i;
	int iStartIndex;
	int iEndIndex;
	int ScrIndex[33];  // 3 * 33 = 99
	int iCurScr = 0;
	int iHilightIndex;

	unsigned char ucStartKey;
	int iScrItemNum;
	int itemList[33];
	unsigned char bChkTimer;
	int iKey = GUI_ERR_TIMEOUT;

	RectMap stRectMap[2];

	int nTotalLine = 0;
	Rect stItemsRect[15];

	if (!pstMenu) {
		return GUI_ERR_INVALIDPARAM;
	}

	ClearScr(&stMenuRect, 1);
	memset(stItemsRect, 0, sizeof(stItemsRect));

	//title
	if (pstMenu->szTitle[0] != 0) {
		DrawText(pstMenu->szTitle, (void *)&pstMenu->stTitleAttr, sgTitleArea.top, sgTitleArea.bottom, sgTitleArea.left, sgTitleArea.right, 0);
	}

	if (pstMenu->pstContent == NULL) {
		return GUI_ERR_INVALIDPARAM;
	}

	nTotalLine = (stMenuRect.bottom - stMenuRect.top) / GetFontHeight(sgFontS[pstMenu->stItemAttr.eFontSize], sgFontM[pstMenu->stItemAttr.eFontSize]);

	for (i = 0; i < sizeof(stItemsRect) / sizeof(stItemsRect[0]) && i < nTotalLine; ++i) {
		stItemsRect[i].top = stMenuRect.top + i * GetFontHeight(sgFontS[pstMenu->stItemAttr.eFontSize], sgFontM[pstMenu->stItemAttr.eFontSize]);
		stItemsRect[i].bottom = stMenuRect.top + (i + 1) * GetFontHeight(sgFontS[pstMenu->stItemAttr.eFontSize], sgFontM[pstMenu->stItemAttr.eFontSize]);
		stItemsRect[i].left = stMenuRect.left;
		stItemsRect[i].right = stMenuRect.right;
		stItemsRect[i].nValue = 1000 + i;
	}

	//Find the first menu item in each screen.
	for (i = 0; i < sizeof(ScrIndex) / sizeof(ScrIndex[0]); ++i) {
		ScrIndex[i] = -1;
	}

	for (i = 0; i < sizeof(itemList) / sizeof(itemList[0]); ++i) {
		itemList[i] = 0;
	}

	if (eMode & GUI_MENU_0_START) {
		ucStartKey = KEY0;
	}
	else {
		ucStartKey = KEY1;
	}

	//Locate first available item
	iStartIndex = GetFirstHiliIndex(pstMenu);
	if (iStartIndex < 0) {
		return GUI_ERR_INVALIDPARAM;
	}

	iHilightIndex = iStartIndex;

	ScrIndex[0] = iStartIndex;
	i = 1;
	while (iStartIndex < (int)pstMenu->nSize) {
		iEndIndex = PrepareScrMenu(pstMenu, stMenuRect, nTotalLine, NULL, iStartIndex, iHilightIndex, eMode, &iScrItemNum, itemList);
		ScrIndex[i++] = iEndIndex;
		iStartIndex = iEndIndex;
	}

	iCurScr = 0;
	iStartIndex = ScrIndex[iCurScr];
	iHilightIndex = iStartIndex;

	if (piSelected && *piSelected > 0) {
		for (i = 0; pstMenu->nSize; ++i) {
			if (pstMenu->pstContent[i].bVisible && *piSelected == pstMenu->pstContent[i].nValue) {
				int j;

				for (j = 0; j < sizeof(ScrIndex) / sizeof(ScrIndex[0]) && ScrIndex[j + 1] != -1; ++j) {
					if (ScrIndex[j] <= *piSelected && *piSelected < ScrIndex[j + 1]) {
						iCurScr = j;
						iStartIndex = ScrIndex[iCurScr];
						iHilightIndex = i;
						break;
					}
				}
				break;
			}
		}
	}

	memset(stRectMap, 0, sizeof(stRectMap));
	stRectMap[0].pRect = (Rect *)pstButtonRect;
	stRectMap[0].no = nButtonNo;

	stRectMap[1].pRect = stItemsRect;
	stRectMap[1].no = sizeof(stItemsRect) / sizeof(stItemsRect[0]);

	while (1) {
		unsigned char bFindingItem = FALSE;
		if (pstButtonRect && pstButtonAttr && nButtonNo > 0) {
			for (i = 0; i < nButtonNo; ++i) {
				ClearScr(&pstButtonRect[i], 0);
				DrawBgBox(pstButtonRect[i].left, pstButtonRect[i].top, pstButtonRect[i].right, pstButtonRect[i].bottom, sg_nDefBg);
				if (pstButtonRect[i].nValue != 0) {
					DrawRect(pstButtonRect[i].left, pstButtonRect[i].top, pstButtonRect[i].right, pstButtonRect[i].bottom, sg_nDefColor);
					DrawRect(pstButtonRect[i].left + 1, pstButtonRect[i].top + 1, pstButtonRect[i].right - 1, pstButtonRect[i].bottom - 1, sg_nDefColor);
				}
				switch (pstButtonRect[i].nValue) {
				case KEYENTER:
					DrawText((void *)GetKeyValuebyIndex(KEYENTER), (void *)pstButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
					break;
				case KEYCANCEL:
					DrawText((void *)GetKeyValuebyIndex(KEYCANCEL), (void *)pstButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
					break;
				case GUI_KEYPREV:
					DrawText((void *)GetKeyValuebyIndex(KEYUP), (void *)pstButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
					break;
				case GUI_KEYNEXT:
					DrawText((void *)GetKeyValuebyIndex(KEYDOWN), (void *)pstButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
					break;
				}
			}
		}
		if (iStartIndex != ScrIndex[iCurScr])
			ClearScr(&stMenuRect, 1);
		iStartIndex = ScrIndex[iCurScr];
		iEndIndex = PrepareScrMenu(pstMenu, stMenuRect, nTotalLine, stItemsRect, iStartIndex, iHilightIndex, eMode, &iScrItemNum, itemList);

		if (iHilightIndex > ScrIndex[iCurScr]) {
			ScrSetIcon(ICON_UP, OPENICON);
		}
		else {
			ScrSetIcon(ICON_UP, CLOSEICON);
		}

		if ((ScrIndex[iCurScr + 1] >= (int)pstMenu->nSize) && (iHilightIndex == iEndIndex - 1)) {
			ScrSetIcon(ICON_DOWN, CLOSEICON);
		}
		else {
			ScrSetIcon(ICON_DOWN, OPENICON);
		}

		//////////////////////////////////////////////////////////////////////////
		// get key & touch screen
		if (timeoutSec >= 0) {
			bChkTimer = TRUE;
			TimerSet(GUI_TIMER_INDEX, (short)timeoutSec * 10);
		}
		else {
			bChkTimer = FALSE;
		}
		while (1) {
			int iTouchStatus = 0;
			GUI_CALLBACK vFunc = GetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT);
			if (vFunc) {
				int callbackLen = 0;
				iKey = vFunc(GUI_CALLBACK_LISTEN_EVENT, NULL, &callbackLen);
				if (iKey != GUI_OK)
					break;
			}

			iTouchStatus = GetVirtualKey(stRectMap, sizeof(stRectMap) / sizeof(stRectMap[0]));
			/*if(pstButtonRect) {
			if(!bFindingItem && pRectHeaderPointer == stItemsRect) {
			iEndIndex = PrepareScrMenu(pstMenu, stMenuRect, nTotalLine, stItemsRect, iStartIndex, -1, eMode, &iScrItemNum, itemList);
			bFindingItem = TRUE;
			}
			iTouchStatus = GetVirtualKey(stRectMap, sizeof(stRectMap)/sizeof(stRectMap[0]));
			}*/
			iKey = 0;

			if (0 == kbhit()) {
				TimerSet(GUI_TIMER_INDEX, (short)timeoutSec * 10);
				iKey = getkey();
				break;
			}
			else if (iTouchStatus > 0) {
				iEndIndex = PrepareScrMenu(pstMenu, stMenuRect, nTotalLine, stItemsRect, iStartIndex, -1, eMode, &iScrItemNum, itemList);
				TimerSet(GUI_TIMER_INDEX, (short)timeoutSec * 10);
				if (iTouchStatus >= 1000) {
					iHilightIndex = itemList[iTouchStatus - 1000];
				}
				iKey = iTouchStatus;
				break;
			}
			else if (bChkTimer && 0 == TimerCheck(GUI_TIMER_INDEX)) {
				iKey = GUI_ERR_TIMEOUT;
				break;
			}
			else {
				continue;
			}
		}
		//////////////////////////////////////////////////////////////////////////

		if (GUI_KEYPREV == iKey) {
			iHilightIndex = iStartIndex;
			iKey = KEYUP;
		}
		else if (GUI_KEYNEXT == iKey) {
			iHilightIndex = iEndIndex - 1;
			iKey = KEYDOWN;
		}

		if (GUI_ERR_TIMEOUT == iKey) {
			ScrSetIcon(ICON_UP, CLOSEICON);
			ScrSetIcon(ICON_DOWN, CLOSEICON);
			return GUI_ERR_TIMEOUT;
		}
		else if (KEYCANCEL == iKey) {
			ScrSetIcon(ICON_UP, CLOSEICON);
			ScrSetIcon(ICON_DOWN, CLOSEICON);
			return GUI_ERR_USERCANCELLED;
		}

		if (iKey == KEYENTER) {
			ScrSetIcon(ICON_UP, CLOSEICON);
			ScrSetIcon(ICON_DOWN, CLOSEICON);

			if (piSelected) {
				*piSelected = pstMenu->pstContent[iHilightIndex].nValue;
			}

			if (pstMenu->pstContent[iHilightIndex].vFunc) {
				GUI_CALLBACK CurrCallback = GetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT);
				SetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT, NULL);
				sg_isCalled = 0;
				pstMenu->pstContent[iHilightIndex].vFunc();
				sg_isCalled = 1;
				if (eMode & GUI_MENU_DIRECT_RETURN) {
					ScrSetIcon(ICON_UP, CLOSEICON);
					ScrSetIcon(ICON_DOWN, CLOSEICON);
					return GUI_OK;
				}
				ClearScr(&stMenuRect, 1);
				SetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT, CurrCallback);
			}
			else {
				return GUI_OK;
			}
		}
		else if (iKey == KEYUP || iKey == KEYF1) {
			int iLastIndex = iHilightIndex;
			iHilightIndex = GetNextHiliIndex(pstMenu, iHilightIndex, -1);
			if (iLastIndex == iHilightIndex)
				Beep(); // Added by Kim 2014/12/07 bug529
						//If Still can show
			if (iHilightIndex >= iStartIndex) {
				continue;
			}
			else {
				if (iCurScr > 0) {
					for (i = iCurScr; i > 0; i++) {
						if (iHilightIndex >= ScrIndex[i - 1] && iHilightIndex < ScrIndex[i]) {
							iCurScr = i - 1;
							break;
						}
					}
				}
				continue;
			}
		}
		else if (iKey == KEYDOWN || iKey == KEYF2) {
			int iLastIndex = iHilightIndex;
			iHilightIndex = GetNextHiliIndex(pstMenu, iHilightIndex, 1);
			if (iLastIndex == iHilightIndex)
				Beep(); // Added by Kim 2014/12/07 bug529

			for (i = iCurScr + 1; i < sizeof(ScrIndex) / sizeof(ScrIndex[0]); ++i) {
				if (ScrIndex[i] == -1) {
					iCurScr = i - 1;
					break;
				}
				if (iHilightIndex >= ScrIndex[i - 1] && iHilightIndex < ScrIndex[i]) {
					iCurScr = i - 1;
					break;
				}
			}
			continue;
		}
		else if (iKey == KEYMENU || iKey == FNKEYMENU) {
			return iKey;
		}
		else if (!(eMode & GUI_MENU_MANUAL_INDEX) && iKey >= ucStartKey && iKey < ucStartKey + iScrItemNum) {
			iHilightIndex = itemList[iKey - ucStartKey];
			ScrSetIcon(ICON_UP, CLOSEICON);
			ScrSetIcon(ICON_DOWN, CLOSEICON);
			if (piSelected) {
				*piSelected = pstMenu->pstContent[iHilightIndex].nValue;
			}
			if (pstMenu->pstContent[iHilightIndex].vFunc) {
				GUI_CALLBACK CurrCallback = GetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT);
				SetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT, NULL);
				sg_isCalled = 0;
				pstMenu->pstContent[iHilightIndex].vFunc();
				sg_isCalled = 1;
				if (eMode & GUI_MENU_DIRECT_RETURN) {
					ScrSetIcon(ICON_UP, CLOSEICON);
					ScrSetIcon(ICON_DOWN, CLOSEICON);
					return GUI_OK;
				}
				ClearScr(&stMenuRect, 1);
				SetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT, CurrCallback);
			}
			else {
				return GUI_OK;
			}
		}
	}
	return GUI_OK;
}

// Modified by Kim_LinHB 2014-8-8 v0.3
static int GetInfoPage(const GUI_PAGE *pstPage, Rect stInfoRect, const Rect *pstButtonRect, unsigned char nButtonNo, const GUI_TEXT_ATTR *pstButtonAttr, unsigned char isMultiChapters, int timeoutSec) {
    int i;
    unsigned int iStartIndex;
    int ScrIndex[33];  // 3 * 33 = 99
    int iCurScr = 0;

    unsigned char bChkTimer;
    int iKey = GUI_ERR_TIMEOUT;

    int nTotalLine = 0;
	char isPopUp = FALSE;

    RectMap stRectMap[4];

    if (pstPage == NULL) {
        return GUI_ERR_INVALIDPARAM;
    }

    ClearScr(&stInfoRect, 1);

//title
    if (pstPage->szTitle[0] != 0) {
        DrawText(pstPage->szTitle, (void *) &pstPage->stTitleAttr, sgTitleArea.top, sgTitleArea.bottom, sgTitleArea.left, sgTitleArea.right, 0);
    }

    if (pstButtonAttr && nButtonNo > 0) {
        for (i = 0; i < nButtonNo; ++i) { // cancel & ok(next) & clear(previous)
            DrawBgBox(pstButtonRect[i].left, pstButtonRect[i].top, pstButtonRect[i].right, pstButtonRect[i].bottom, sg_nDefBg);
            if(sg_hasTouchScreen) {
                if(pstButtonRect[i].nValue != 0) {
                    DrawRect(pstButtonRect[i].left, pstButtonRect[i].top, pstButtonRect[i].right, pstButtonRect[i].bottom, sg_nDefColor);
                    DrawRect(pstButtonRect[i].left+ 1, pstButtonRect[i].top+ 1, pstButtonRect[i].right-1, pstButtonRect[i].bottom-1, sg_nDefColor);
                }
            }
// Modified by Kim_LinHB 20140916 v0.8 bug522
            switch (pstButtonRect[i].nValue) {
                case KEYENTER:
                    DrawText((void *) GetKeyValuebyIndex(KEYENTER), (void *) pstButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
                    break;
                case KEYCANCEL:
                    DrawText((void *) GetKeyValuebyIndex(KEYCANCEL), (void *) pstButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
                    break;
				case KEYUP: 
					DrawText((void *) GetKeyValuebyIndex(KEYUP), (void *) pstButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
					break;
				case KEYDOWN: 
					DrawText((void *) GetKeyValuebyIndex(KEYDOWN), (void *) pstButtonAttr, pstButtonRect[i].top, pstButtonRect[i].bottom, pstButtonRect[i].left, pstButtonRect[i].right, 1);
					break;
            }
        }
    }

    nTotalLine = stInfoRect.bottom - stInfoRect.top;

    if (pstPage->pstContent == NULL) {
		return GUI_ERR_INVALIDPARAM;
    }

//Find the first menu item in each screen.
    for (i = 0; i < sizeof(ScrIndex) / sizeof(ScrIndex[0]); ++i) {
        ScrIndex[i] = -1;
    }

    i = 0;
    ScrIndex[0] = 0;
// Modified by Kim v0.5
    while (ScrIndex[i] < (int) pstPage->nLine) {
        unsigned int j = 0;
        unsigned int uiTotalHeight = 0;
        for (j = ScrIndex[i]; j < pstPage->nLine; ++j) {
            uiTotalHeight += GetFontHeight(sgFontS[pstPage->pstContent[j].stLineAttr.eFontSize], sgFontM[pstPage->pstContent[j].stLineAttr.eFontSize]);
            if (uiTotalHeight == nTotalLine) {
                ScrIndex[i + 1] = j + 1;
                break;
            }
            else if ((int) uiTotalHeight > nTotalLine) {
                ScrIndex[i + 1] = j;
                break;
            }
        }
        if (pstPage->nLine == j) {
            break;
        }
        ++i;
    }

    memset(stRectMap, 0, sizeof(stRectMap));
    stRectMap[0].pRect = (Rect *)pstButtonRect;
    stRectMap[0].no = nButtonNo;

    stRectMap[1].pRect = &sgInfoPageMenu;
    stRectMap[1].no = 1;

	if(isMultiChapters && sg_isColorScreen){
		stRectMap[2].pRect = &sgInfoPageMenu;
		stRectMap[2].no = 1;
		DrawBgBox(sgInfoPageMenu.left, sgInfoPageMenu.top, sgInfoPageMenu.right, sgInfoPageMenu.bottom, sg_nDefBg);
		if(sg_hasTouchScreen) {
			DrawRect(sgInfoPageMenu.left, sgInfoPageMenu.top, sgInfoPageMenu.right, sgInfoPageMenu.bottom, sg_nDefColor);
			DrawRect(sgInfoPageMenu.left+ 1, sgInfoPageMenu.top+ 1, sgInfoPageMenu.right-1, sgInfoPageMenu.bottom-1, sg_nDefColor);
		}
		DrawText((void *) GetKeyValuebyIndex(KEYMENU), (void *) pstButtonAttr, sgInfoPageMenu.top, sgInfoPageMenu.bottom, sgInfoPageMenu.left, sgInfoPageMenu.right, 1);

		stRectMap[3].pRect = sgInfoPageMenuOpt;
		stRectMap[3].no = 2;
	}

    iCurScr = 0;
    iStartIndex = 0;
    while (1) {
        unsigned int iEndIndex = ScrIndex[iCurScr + 1];
        if (iStartIndex != ScrIndex[iCurScr] || KEYMENU == iKey)
            ClearScr(&stInfoRect, 1);
        iStartIndex = ScrIndex[iCurScr];
        if (-1 == iEndIndex)
            iEndIndex = pstPage->nLine;
        PrepareScrPage(pstPage, stInfoRect, iStartIndex, iEndIndex - iStartIndex); // Modified by Kim_LinHB 9/9/2014 v0.6

		if(sg_isColorScreen && isPopUp){
			for (i = 0; i < 2; ++i) { // cancel & ok(next) & clear(previous)
				DrawBgBox(sgInfoPageMenuOpt[i].left, sgInfoPageMenuOpt[i].top, sgInfoPageMenuOpt[i].right, sgInfoPageMenuOpt[i].bottom, sg_nDefBg);
				if(sg_hasTouchScreen) {
					if(sgInfoPageMenuOpt[i].nValue != 0) {
						DrawRect(sgInfoPageMenuOpt[i].left, sgInfoPageMenuOpt[i].top, sgInfoPageMenuOpt[i].right, sgInfoPageMenuOpt[i].bottom, sg_nDefColor);
						DrawRect(sgInfoPageMenuOpt[i].left+ 1, sgInfoPageMenuOpt[i].top+ 1, sgInfoPageMenuOpt[i].right-1, sgInfoPageMenuOpt[i].bottom-1, sg_nDefColor);
					}
				}
				// Modified by Kim_LinHB 20140916 v0.8 bug522
				switch (sgInfoPageMenuOpt[i].nValue) {
				case GUI_KEYPREV:
					DrawText((void *) GetKeyValuebyIndex(GUI_KEYPREV), (void *) pstButtonAttr, sgInfoPageMenuOpt[i].top, sgInfoPageMenuOpt[i].bottom, sgInfoPageMenuOpt[i].left, sgInfoPageMenuOpt[i].right, 1);
					break;
				case GUI_KEYNEXT:
					DrawText((void *) GetKeyValuebyIndex(GUI_KEYNEXT), (void *) pstButtonAttr, sgInfoPageMenuOpt[i].top, sgInfoPageMenuOpt[i].bottom, sgInfoPageMenuOpt[i].left, sgInfoPageMenuOpt[i].right, 1);
					break;
				}
			}
		}

        if (iCurScr > 0) {
            ScrSetIcon(ICON_UP, OPENICON);
        }
        else {
            ScrSetIcon(ICON_UP, CLOSEICON);
        }

        if (ScrIndex[iCurScr + 1] >= (int) pstPage->nLine || ScrIndex[iCurScr + 1] < 0) {
            ScrSetIcon(ICON_DOWN, CLOSEICON);
        }
        else {
            ScrSetIcon(ICON_DOWN, OPENICON);
        }

//////////////////////////////////////////////////////////////////////////
// get key & touch screen
        if (timeoutSec >= 0) {
            bChkTimer = TRUE;
            TimerSet(GUI_TIMER_INDEX, (short) timeoutSec * 10);
        }
        else {
            bChkTimer = FALSE;
        }
        while (1) {
			int stMapNo = isMultiChapters ? (isPopUp ? 4 : 3) : 2;
			int iTouchStatus = 0;
			GUI_CALLBACK vFunc = NULL;
			if((vFunc = GetCallbackEvent(GUI_CALLBACK_LISTEN_EVENT))){
				int callbackLen = 0;
				iKey = vFunc(GUI_CALLBACK_LISTEN_EVENT, NULL, &callbackLen);
				if(iKey != GUI_OK)
					break;
			}

            iTouchStatus = GetVirtualKey(stRectMap, stMapNo);
            iKey = 0;

            if (0 == kbhit()) {
                TimerSet(GUI_TIMER_INDEX, (short) timeoutSec * 10);
                iKey = getkey();
                break;
            }
            else if(iTouchStatus > 0) {
                TimerSet(GUI_TIMER_INDEX, (short)timeoutSec * 10);
                iKey = iTouchStatus;
				break;
            }
            else if (bChkTimer && 0 == TimerCheck(GUI_TIMER_INDEX)) {
                iKey = GUI_ERR_TIMEOUT;
                break;
            }
            else {
                continue;
            }
        }
//////////////////////////////////////////////////////////////////////////

        if (KEYCANCEL == iKey) {
            ScrSetIcon(ICON_UP, CLOSEICON);
            ScrSetIcon(ICON_DOWN, CLOSEICON);
            return GUI_ERR_USERCANCELLED;
        }
        else if (GUI_ERR_TIMEOUT == iKey) {
            ScrSetIcon(ICON_UP, CLOSEICON);
            ScrSetIcon(ICON_DOWN, CLOSEICON);
            return GUI_ERR_TIMEOUT;
        }
        else if (KEYENTER == iKey) {
            ScrSetIcon(ICON_UP, CLOSEICON);
            ScrSetIcon(ICON_DOWN, CLOSEICON);
			return GUI_OK;
        }
		else if(isMultiChapters && KEYMENU == iKey){
			isPopUp = ~isPopUp;
		}
		else if(isPopUp && (GUI_KEYPREV == iKey || KEY1 == iKey)){
			ScrSetIcon(ICON_UP, CLOSEICON);
			ScrSetIcon(ICON_DOWN, CLOSEICON);
			return GUI_OK_PREVIOUS;
		}
		else if(isPopUp && (GUI_KEYNEXT == iKey || KEY2 == iKey)){
			ScrSetIcon(ICON_UP, CLOSEICON);
			ScrSetIcon(ICON_DOWN, CLOSEICON);
			return GUI_OK_NEXT;
		}

        if (KEYUP == iKey || KEYF1 == iKey) {
			if(isMultiChapters &&  0 == iCurScr && -1 == ScrIndex[1] ){
				ScrSetIcon(ICON_UP, CLOSEICON);
				ScrSetIcon(ICON_DOWN, CLOSEICON);
				return GUI_OK_PREVIOUS;
			}
            if (iCurScr > 0) {
                --iCurScr;
            }
            else {
                Beep(); // Added by Kim 2014/12/07 bug529
            }
        }
        else if (KEYF2 == iKey || KEYDOWN == iKey) {
			if(isMultiChapters && 0 == iCurScr && -1 == ScrIndex[1] ){
				ScrSetIcon(ICON_UP, CLOSEICON);
				ScrSetIcon(ICON_DOWN, CLOSEICON);
				return GUI_OK_NEXT;
			}

            if (0 < ScrIndex[iCurScr + 1] && ScrIndex[iCurScr + 1] < (int) pstPage->nLine) {
                ++iCurScr;
//iStartIndex = iEndIndex;
            }
            else {
                Beep(); // Added by Kim 2014/12/07 bug529
            }
        }
    }
    return GUI_OK; // not effective
}

static int GetFirstHiliIndex(const GUI_MENU *pstMenu) {
    unsigned int i;
    for (i = 0; i < pstMenu->nSize; ++i) {
        if (pstMenu->pstContent[i].bVisible) {
            return i;
        }
    }

    return -1;
}

static int GetNextHiliIndex(const GUI_MENU *pstMenu, int iCurrIndex, short nOffset) {
    if (nOffset < 0) {
        int iLow = GetFirstHiliIndex(pstMenu);
        int i;

        if (iCurrIndex == iLow) {
            return iCurrIndex;
        }

        for (i = iCurrIndex - 1; i >= iLow; --i) {
            if (pstMenu->pstContent[i].bVisible && ++nOffset >= 0) {
                return i;
            }
        }
        return iLow;
    }
    else {
        unsigned int i = 0, iLastIndex = pstMenu->nSize;

        for (i = 0; i < pstMenu->nSize; ++i) {
            if (pstMenu->pstContent[i].bVisible) {
                iLastIndex = i;
            }
        }

        for (i = iCurrIndex + 1; i < pstMenu->nSize; ++i) {
            if (pstMenu->pstContent[i].bVisible && --nOffset <= 0) {
                return i;
            }
        }
        return iLastIndex;
    }
}

static int PrepareScrMenu(const GUI_MENU *pstMenu, Rect stMenuRect, int nTotalLine, const Rect *pstItemsRect, int iStartIndex, int iHilightIndex, enum MENUSTYLE eMode, int *piScrItemNum, int *itemList) {
    int iCurLine = 0;
    int iNumIndex;
    int i;
    int iCurItem;
    char buf[30 * 2 + 1], buf1[30 * 2 + 1];
    char menu_text[40][30 * 2 + 1];
    int iHilightLine;
    int iEndIndex;

    int k = 0;

    memset(buf, 0x00, sizeof(buf));
    memset(buf1, 0x00, sizeof(buf));

    *piScrItemNum = 0;

    if (eMode & GUI_MENU_0_START) {
        iNumIndex = 0;
    }
    else {
        iNumIndex = 1;
    }

    iCurItem = iStartIndex;

    iEndIndex = pstMenu->nSize;

    for (i = iCurItem; i < (int) pstMenu->nSize; ++i) {
        if (!pstMenu->pstContent[i].bVisible) {
            continue;
        }

        if (i == iHilightIndex) {
            iHilightLine = iCurLine;
        }

        itemList[k++] = i;

        if (eMode & GUI_MENU_MANUAL_INDEX) {
            sprintf(buf, "%d.%s", pstMenu->pstContent[i].nValue, pstMenu->pstContent[i].szText);
        }
        else {
            sprintf(buf, "%d.%s", iNumIndex, pstMenu->pstContent[i].szText);
        }

        ++iNumIndex;

        strcpy(menu_text[iCurLine], buf);
        ++(*piScrItemNum);
        ++iCurLine;
        if (iCurLine == nTotalLine) {
            iEndIndex = i + 1;
            break;
        }
    }

    if (pstItemsRect) {
        i = 0;
        if (!(eMode & GUI_MENU_0_START)) {
            --iNumIndex;
        }

        while (i < iNumIndex) {
            if ((i == iHilightLine)) {
                GUI_TEXT_ATTR stTextAttr = pstMenu->stItemAttr;
                if (stTextAttr.eStyle & GUI_FONT_REVERSAL) {
                    stTextAttr.eStyle &= ~GUI_FONT_REVERSAL;
                }
                else {
                    stTextAttr.eStyle |= (GUI_FONT_REVERSAL | GUI_FONT_OPAQUE);
                }
//ClearScr(&stForClean);
                DrawText(menu_text[i], (void *) &stTextAttr, pstItemsRect[i].top, pstItemsRect[i].bottom, pstItemsRect[i].left, pstItemsRect[i].right, 1);
            }
            else {
//ClearScr(&stForClean);
				ClearScr(&pstItemsRect[i], 1);
				DrawBgBox(pstItemsRect[i].left, pstItemsRect[i].top, pstItemsRect[i].right, pstItemsRect[i].bottom, sg_nDefBg);

                DrawText(menu_text[i], (void *) &pstMenu->stItemAttr, pstItemsRect[i].top, pstItemsRect[i].bottom, pstItemsRect[i].left, pstItemsRect[i].right, 1);
            }
            ++i;
        }
    }

    for (i = iEndIndex; i < (int) pstMenu->nSize; ++i) {
        if (pstMenu->pstContent[i].bVisible)
            break;
    }

    return i;
}

static void PrepareScrPage(const GUI_PAGE *pstPage, Rect stPageRect, int iStartIndex, int iLineInPage) {
    int i;
    int top = stPageRect.top;

    for (i = iStartIndex; i < (int) pstPage->nLine && i < (iStartIndex + iLineInPage); ++i) {
        int iHeight = GetFontHeight(sgFontS[pstPage->pstContent[i].stLineAttr.eFontSize], sgFontM[pstPage->pstContent[i].stLineAttr.eFontSize]);
        DrawText(pstPage->pstContent[i].szLine, (void *) &pstPage->pstContent[i].stLineAttr, top, top + iHeight, stPageRect.left, stPageRect.right, 0);
        top += iHeight;
    }

}

static unsigned char IsSingle(char p) {
    if (~(p >> 8) == 0)
        return 0; //4z1m2;JG::WV
    return 1;
}

static unsigned short GetStrPix(const unsigned char *pszSource, const ST_FONT *pstFont_S, const ST_FONT *pstFont_M, unsigned int uiSpaceWidth) {
#ifdef _AR_MODE_
    if (pstFont_M && CHARSET_ARABIA == pstFont_M->CharSet && sg_uiArabicFileID != AR_OPENFILE_ERROR) {
        int nWidth = ArScrGetStrWidth((unsigned char *) pszSource);
        if (nWidth >= 0) {
            return nWidth;
        }
        else {
            return 0;
        }
    }
    else {
#endif
        unsigned int sum = 0, i = 0;
        for (i = 0; i < strlen(pszSource); /*NULL*/) {
            if (IsSingle(pszSource[i])) {
                ++i;
                sum += (pstFont_S ? pstFont_S->Width : 0) + uiSpaceWidth;
            }
            else {
                i += 2;
                sum += (pstFont_M ? pstFont_M->Width : 0) + uiSpaceWidth;
            }
        }
        return (unsigned short) sum;
#ifdef _AR_MODE_
    }
#endif
}

void MapChar2Index(uchar ch, int *piRow, int *piCol) {
    int i;
    char *p;

    for (i = 0; i < 16; ++i) // 16 buttons on keyboard
            {
				for (p = (char *) sgPhyKeyMap[i].Table; *p; ++p) {
            if (*p == toupper(ch)) {
                *piRow = i;
				*piCol = p - sgPhyKeyMap[i].Table;
                break;
            }
        }
    }
}

static int MapKey(unsigned char ch) {
    int i;
    for (i = 0; i < 16; ++i) { // 16 buttons on keyboard
		if (sgPhyKeyMap[i].Table[0] == ch)
            return i;
    }
    return -1;
}

static unsigned char IsValidTime(const unsigned char *psDateTime, const unsigned char *psMaskYYYYMMDDhhmmss) {
    unsigned int uiYear = 0, uiMonth = 0, uiDay = 0, uiHour = 0, uiMinute = 0;
    unsigned char ucOffset = 0;

    if (psMaskYYYYMMDDhhmmss == NULL) {
        return -1;                          //Mask error
    }

    if (strlen(psMaskYYYYMMDDhhmmss) == 0 || strlen(psMaskYYYYMMDDhhmmss) > 14 || (strlen(psMaskYYYYMMDDhhmmss) % 2) == 1) {
        return -1;                          //Mask error
    }

//processing Year
    if (memcmp(psMaskYYYYMMDDhhmmss, "YYYY", 4) == 0) {
        uiYear = Asc2Long(psDateTime, 4);
        ucOffset += 4;
        if (uiYear < 1900 || uiYear > 2099) {
            return 1;                                       //Year error
        }
    }
    else if (memcmp(psMaskYYYYMMDDhhmmss, "YY", 2) == 0) {
        uiYear = Asc2Long(psDateTime, 2);
        uiYear += 2000;
        ucOffset += 2;
    }

//processing Month
    if (psMaskYYYYMMDDhhmmss[ucOffset] == 0)
        return 0;      //Finished and return OK
    if (uiYear != 0) {
        if (memcmp(psMaskYYYYMMDDhhmmss + ucOffset, "MM", 2) != 0)
            return -1;  //Mask error
    }
    if (memcmp(psMaskYYYYMMDDhhmmss + ucOffset, "MM", 2) == 0) {
        uiMonth = Asc2Long(psDateTime + ucOffset, 2);
        ucOffset += 2;
        if (uiMonth == 0 || uiMonth > 12) {
            return 2;                                       //Month error
        }
    }

//processing Day
    if (psMaskYYYYMMDDhhmmss[ucOffset] == 0)
        return 0;      //Finished and return OK
    if (uiMonth != 0) {
        if (memcmp(psMaskYYYYMMDDhhmmss + ucOffset, "DD", 2) != 0)
            return -1;  //Mask error
    }
    if (memcmp(psMaskYYYYMMDDhhmmss + ucOffset, "DD", 2) == 0) {
        uiDay = Asc2Long(psDateTime + ucOffset, 2);
        ucOffset += 2;
        if (uiDay == 0 || uiDay > 31) {
            return 3;                                       //Day error
        }
        if (uiMonth != 0) {
            const unsigned char ucMonthDay[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            if (uiDay > ucMonthDay[uiMonth - 1]) {
                return 3;                                       //Day error
            }
            if (uiYear != 0 && uiMonth == 2 && uiDay == 29) {
                if (IsLeapYear(uiYear) == 0) {
                    return 3;                                       //Day error
                }
            }
        }
    }

//processing Hour
    if (psMaskYYYYMMDDhhmmss[ucOffset] == 0)
        return 0;      //Finished and return OK
    if (uiDay != 0) {
        if (memcmp(psMaskYYYYMMDDhhmmss + ucOffset, "hh", 2) != 0)
            return -1;  //Mask error
    }
    if (memcmp(psMaskYYYYMMDDhhmmss + ucOffset, "hh", 2) == 0) {
        uiHour = Asc2Long(psDateTime + ucOffset, 2);
        ucOffset += 2;
        if (uiHour > 23) {
            return 4;                                       //Hour error
        }
        ++uiHour;       //for not equal 0
    }

//processing Minite
    if (psMaskYYYYMMDDhhmmss[ucOffset] == 0)
        return 0;      //Finished and return OK
    if (uiHour != 0) {
        if (memcmp(psMaskYYYYMMDDhhmmss + ucOffset, "mm", 2) != 0)
            return -1;  //Mask error
    }
    if (memcmp(psMaskYYYYMMDDhhmmss + ucOffset, "mm", 2) == 0) {
        uiMinute = Asc2Long(psDateTime + ucOffset, 2);
        ucOffset += 2;
        if (uiMinute > 59) {
            return 5;                                       //Minute error
        }
        ++uiMinute;     //for not equal 0
    }

//processing Second
    if (psMaskYYYYMMDDhhmmss[ucOffset] == 0)
        return 0;      //Finished and return OK
    if (uiMinute != 0) {
        if (memcmp(psMaskYYYYMMDDhhmmss + ucOffset, "ss", 2) != 0)
            return -1;  //Mask error
    }
    if (memcmp(psMaskYYYYMMDDhhmmss + ucOffset, "ss", 2) == 0) {
        unsigned int uiSecond = 0;
        uiSecond = Asc2Long(psDateTime + ucOffset, 2);
        ucOffset += 2;
        if (uiSecond > 59) {
            return 6;                                       //Minute error
        }
        ++uiSecond;     //for not equal 0
    }

    if (ucOffset == 0) {
        return -1;      //Mask error
    }
    return 0;
}

unsigned long Asc2Long(const unsigned char *psString, unsigned int uiLength) {
    unsigned char szBuff[15 + 1];

    sprintf((char *) szBuff, "%.*s", uiLength <= 15 ? uiLength : 15, psString);
    return (unsigned long) atol((char*) szBuff);
}

unsigned char IsLeapYear(unsigned long ulYear) {
    if ((ulYear % 400) == 0) {
        return 1;
    }
    else if ((ulYear % 100) == 0) {
        return 0;
    }
    else if ((ulYear % 4) == 0) {
        return 1;
    }
    return 0;
}

void AdjustDateTime(unsigned char *pszYYMMDDhhmmss, short nOffset, unsigned char ucMode) {
    short nMin[5] = { 0, 1, 1, 0, 0 };
    short nMax[5] = { 99, 12, -1, 23, 59 };
    unsigned char nSeek[5] = { 0, 2, 4, 6, 8 };
    const unsigned char ucMonthDayMax[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    unsigned char szTmp[4 + 1];

    long nCurrValue = 0;
    if (ucMode > 5)
        return;

    nCurrValue = Asc2Long(pszYYMMDDhhmmss + nSeek[ucMode], 2);

    switch (ucMode) { //special field
        case 0:
            if (2 == Asc2Long(pszYYMMDDhhmmss + nSeek[1], 2) && 29 == Asc2Long(pszYYMMDDhhmmss + nSeek[2], 2)) { //Is leap year
                nOffset = nOffset * 4;
            }
            break;
        case 2:
            nMax[ucMode] = ucMonthDayMax[Asc2Long(pszYYMMDDhhmmss + nSeek[1], 2) - 1];
            if (29 == nMax[ucMode] && !IsLeapYear(Asc2Long(pszYYMMDDhhmmss + nSeek[0], 2)))
                nMax[ucMode] = 28;
            break;
    }

    nCurrValue += nOffset;
    if (nCurrValue > nMax[ucMode]) {
        nCurrValue = nMin[ucMode] + (nOffset - 1);
    }
    else if (nCurrValue < nMin[ucMode]) {
        nCurrValue = nMax[ucMode] - (nOffset + 1);
    }

    sprintf(szTmp, "%0*d", 2, nCurrValue);

    memcpy(pszYYMMDDhhmmss + nSeek[ucMode], szTmp, 2);
}

// Added by Kim 2014-08-27 v0.4
int GetFontHeight(ST_FONT stFont_S, ST_FONT stFont_M) {
    int nHeight;
#ifdef _AR_MODE_
    if (CHARSET_ARABIA == stFont_M.CharSet && sg_uiArabicFileID != AR_OPENFILE_ERROR) {
        nHeight = ArFontHeight(sg_uiArabicFileID, stFont_M.Height); //cuz Height set before is just index
        if (nHeight <= 0) {
            nHeight = stFont_S.Height;
        }
    }
    else {
#endif
        nHeight = stFont_S.Height;
#ifdef _AR_MODE_
    }
#endif
    return nHeight;
}

static int SetKeybyIndex(int index, const char *text)
{
	int iRet1;
	int iFoundIndex = sg_key.num;
	int i;
	KeyMap stTemp;

	for(i = 0; i < sg_key.num; ++i){
		if(index == sg_key.key[i].KeyValue){
			iFoundIndex = i;
			break;
		}
	}

	if(i >= sizeof(sg_key.key) / sizeof(sg_key.key[0]))
		return GUI_ERR_EXCEED;

	memcpy(&stTemp, &sg_key.key[iFoundIndex], sizeof(KeyMap));

	if(iFoundIndex == sg_key.num){
		stTemp.KeyValue = index;
	}

	iRet1 = SetKeybyIndex_Text(&stTemp, text);

	if( GUI_OK == iRet1)
	{
		memcpy(&sg_key.key[iFoundIndex], &stTemp, sizeof(KeyMap));

		if(iFoundIndex == sg_key.num)
			++sg_key.num;
		return GUI_OK;
	}

	return GUI_ERR_INVALIDPARAM;

}

static int SetKeybyIndex_Text(KeyMap *key, const char *text){
	//if(!key)return GUI_ERR_INVALIDPARAM; //will not happen
	if(key->Table)free(key->Table);
	key->Table = NULL;
	if(text){
		key->Table = (char *)malloc(strlen(text) + 1);
		strcpy(key->Table, text);
	}
	return GUI_OK;
}

static const char *GetKeyValuebyIndex(int index){
	int i;
	for(i = 0; i < sg_key.num; ++i){
		if(index == sg_key.key[i].KeyValue){
			return sg_key.key[i].Table;
		}
	}

	return NULL;
}


static int SetCallbackEvent(gui_callbacktype_t type, GUI_CALLBACK func)
{
	int i;
	for(i = 0; i < sizeof(sg_event)/sizeof(sg_event[0]); ++i)
	{
		if(type == sg_event[i].type)
		{
			sg_event[i].vFunc = func;
			return GUI_OK;
		}
	}
	return GUI_ERR_INVALIDPARAM;
}
static GUI_CALLBACK GetCallbackEvent(gui_callbacktype_t type)
{
	int i;
	for(i = 0; i < sizeof(sg_event)/sizeof(sg_event[0]); ++i)
	{
		if(type == sg_event[i].type)
		{
			return sg_event[i].vFunc;
		}
	}
	return NULL;
}

#pragma pack(1)
typedef struct tagBITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned int  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int  bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	unsigned int  biSize;
	long biWidth;
	long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int  biCompression;
	unsigned int  biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	unsigned int  biClrUsed;
	unsigned int  biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
} RGBQUAD;
#pragma pack()

static int SaveSignImg(const Rect *area, const unsigned char *pszOutputFile){
	int x,y;
	int fd;
	BITMAPFILEHEADER nbmfHeader;
	BITMAPINFOHEADER   bmi;
	RGBQUAD colorTable[2] = { {0,0,0,0}, {255,255,255,0} };
	int newBitCount = 1;
	int lineSize = 0;
	long desBufSize = 0;
	int iCnt = 0;
	unsigned int color;
	unsigned char desBuf[600 * 1024];

	lineSize = (((area->right - area->left) * newBitCount + 31) & ~31) / 8L;
	desBufSize = (area->bottom - area->top) * lineSize;
	memset(desBuf, 0, sizeof(desBuf));

	remove(pszOutputFile);

	fd = open((char *)pszOutputFile, O_CREATE | O_RDWR);
	if(fd < 0){
		return -1;
	}

	nbmfHeader.bfType = 0x4D42;
	nbmfHeader.bfSize = 62 + desBufSize;
	nbmfHeader.bfReserved1 = 0;
	nbmfHeader.bfReserved2 = 0;
	nbmfHeader.bfOffBits = 62;

	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = area->right - area->left;
	bmi.biHeight = area->bottom - area->top;
	bmi.biPlanes = 1;
	bmi.biBitCount = 1;
	bmi.biCompression = 0;
	bmi.biSizeImage = desBufSize;
	bmi.biXPelsPerMeter = 0;
	bmi.biYPelsPerMeter = 0;
	bmi.biClrUsed = 0;
	bmi.biClrImportant = 0;

	write(fd, (unsigned char *)&nbmfHeader.bfType, sizeof(unsigned short));
	write(fd, (unsigned char *)&nbmfHeader.bfSize, sizeof(unsigned int));
	write(fd, (unsigned char *)&nbmfHeader.bfReserved1, sizeof(unsigned short));
	write(fd, (unsigned char *)&nbmfHeader.bfReserved2, sizeof(unsigned short));
	write(fd, (unsigned char *)&nbmfHeader.bfOffBits, sizeof(unsigned int));

	write(fd, (unsigned char *)&bmi, sizeof(BITMAPINFOHEADER));
	write(fd, (unsigned char *)colorTable, sizeof(RGBQUAD) * 2);

	for(y = area->top; y < area->bottom; ++y){
		iCnt = 0;
		for(x = area->left; x < area->right; ++x){
			int index = (y - area->top) * lineSize + (x - area->left) * newBitCount / 8;
			if ((x - area->left) != 0 && 0 == (x - area->left) % 8)
				++iCnt;
			CLcdGetPixel(x, y, &color);
			if(color == (sg_nDefBg & 0x00F8FCF8)){
				desBuf[(area->bottom - 1 - y) * lineSize + iCnt] |= 1 << (7-(x - area->left)%8);
			}
			else{
				desBuf[(area->bottom - 1 - y) * lineSize + iCnt] &= ~(1 << (7-(x - area->left)%8));
			}
		}
	}

	write(fd, desBuf, desBufSize * sizeof(unsigned char));
	close(fd);
	return 0;
}

static int SaveSignRoute(const Rect *area, const SignData *data, const unsigned char *pszOutputFile){
	unsigned int i;
	int fd;
	SignPoint tmp;
	if(fexist((char *)pszOutputFile) >= 0){
		remove((char *)pszOutputFile);
	}

	fd = open((char *)pszOutputFile, O_CREATE | O_RDWR);
	if(fd < 0) {
		return GUI_ERR_SYSTEM;
	}

	seek(fd, 0, SEEK_SET);

	write(fd, (unsigned char *)&data->point_len, sizeof(data->point_len));

	for(i = 0; i < data->point_len; ++i){
		tmp.x = data->point_array[i].x - area->left;
		tmp.y = data->point_array[i].y - area->top;
		write(fd, (unsigned char *)&tmp, sizeof(tmp));
	}

	close(fd);
	return GUI_OK;
}

static void PrepareRes(){
	int i;
	int nTopOfKeyBoard, nKeyWidth;
	//title
	sgTitleArea.top = 0;
	sgTitleArea.bottom = sg_nHeightOfALine;
	sgTitleArea.left = 0;
	sgTitleArea.right = sg_nScrWidth;

	//msgbox buttons
	sgMsgContentArea.top = sg_nHeightOfALine;
	sgMsgContentArea.bottom = sg_nScrHeight - sg_nHeightOfALine - 5;
	sgMsgContentArea.left = 0;
	sgMsgContentArea.right = sg_nScrWidth;
	sgMsgBoxButtons[0].top = sg_nScrHeight - sg_nHeightOfALine;
	sgMsgBoxButtons[0].bottom = sg_nScrHeight - 5;
	sgMsgBoxButtons[1].top = sgMsgBoxButtons[2].top = sgMsgBoxButtons[0].top;
	sgMsgBoxButtons[1].bottom = sgMsgBoxButtons[2].bottom = sgMsgBoxButtons[0].bottom;
	sgMsgBoxButtons[0].nValue = KEYCANCEL;
	sgMsgBoxButtons[1].nValue = KEYENTER;
	//cancel & ok
	sgMsgBoxButtons[0].left = 5;
	sgMsgBoxButtons[0].right = sg_nScrWidth / 3;
	sgMsgBoxButtons[1].left = sg_nScrWidth *2 / 3;
	sgMsgBoxButtons[1].right = sg_nScrWidth - 5;
	//cancel or ok
	sgMsgBoxButtons[2].left = sg_nScrWidth / 3;
	sgMsgBoxButtons[2].right = sg_nScrWidth *2 / 3;

	//inputbox
	memcpy(sgInputBoxButtons, sgMsgBoxButtons, 2 * sizeof(Rect)); // Cancel OK
	nTopOfKeyBoard = sg_nScrHeight - 4 * sg_nHeightOfALine;
	nKeyWidth = sg_nScrWidth / 4;
	for(i = 0; i < sizeof(sgPhyKeyMap)/sizeof(sgPhyKeyMap[0]); ++i) {         // 4 * 4
		sgKeypad[i].top = nTopOfKeyBoard + i  / 4 * sg_nHeightOfALine;
		sgKeypad[i].bottom = sgKeypad[i].top + sgPhyKeyMap[i].iProportion_y * sg_nHeightOfALine;
		sgKeypad[i].left = i % 4 * nKeyWidth;
		sgKeypad[i].right = sgKeypad[i].left + sgPhyKeyMap[i].iProportion_x * nKeyWidth;
		sgKeypad[i].nValue = sgPhyKeyMap[i].KeyValue;
	}

	if(sg_hasTouchScreen && sg_nScrHeight > sg_nScrWidth){
		sgInputPromptArea.top = sgTitleArea.bottom;
		sgInputPromptArea.bottom = sgInputPromptArea.top + sg_nHeightOfALine;
		sgInputPromptArea.left = 0;
		sgInputPromptArea.right = sg_nScrWidth;

		sgInputContentArea.top = sgInputPromptArea.bottom;
		sgInputContentArea.bottom = sgInputContentArea.top + sg_nHeightOfALine - 1;
		sgInputContentArea.left = 0;
		sgInputContentArea.right = sg_nScrWidth;
	}
	else{
		sgInputPromptArea.top = sgTitleArea.bottom;
		sgInputPromptArea.bottom = (sg_nScrHeight - sg_nHeightOfALine + sgTitleArea.bottom) / 2;
		sgInputPromptArea.left = 0;
		sgInputPromptArea.right = sg_nScrWidth;

		sgInputContentArea.top = sgInputPromptArea.bottom;
		sgInputContentArea.bottom = sg_nScrHeight - sg_nHeightOfALine - 1;
		sgInputContentArea.left = 0;
		sgInputContentArea.right = sg_nScrWidth;
	}

	//timebox
	memcpy(sgTimeBoxButtons, sgMsgBoxButtons, 2 * sizeof(Rect)); // Cancel OK
	sgCurrTimeArea.top = sgTitleArea.bottom;
	sgCurrTimeArea.bottom = sgCurrTimeArea.top + sg_nHeightOfALine;
	sgCurrTimeArea.left = 0;
	sgCurrTimeArea.right = sg_nScrWidth;

	nKeyWidth = sg_nScrWidth / 3;
	nTopOfKeyBoard = sgCurrTimeArea.bottom + 1;
	for (i = 0; i < 3; ++i) { // 3 for date, 3 for time too, for center alignment
		sgDateArea[i].top = nTopOfKeyBoard;
		sgDateArea[i].bottom = sgDateArea[i].top + sg_nHeightOfALine -2;
		sgDateArea[i].left = i * nKeyWidth + 5;
		sgDateArea[i].right = sgDateArea[i].left + nKeyWidth;
		sgDateArea[i].nValue = KEYALPHA+1000+i;
	}
	for (i = 0; i < 2; ++i) {
		sgTimeArea[i].top = nTopOfKeyBoard;
		sgTimeArea[i].bottom = sgTimeArea[i].top + sg_nHeightOfALine - 2;
		sgTimeArea[i].left = i * nKeyWidth + 5 + sg_nScrWidth / 6;
		sgTimeArea[i].right = sgTimeArea[i].left + nKeyWidth - 10;
		sgTimeArea[i].nValue = KEYALPHA+1000+i;
	}

	//Alternative
	memcpy(sgAlternativeButtons, sgMsgBoxButtons, 2 * sizeof(Rect)); // Cancel OK
	sgAlternativePrompt.top = sgTitleArea.bottom;
	sgAlternativePrompt.bottom = sgAlternativePrompt.top + sg_nHeightOfALine - 1;
	sgAlternativePrompt.left = 0;
	sgAlternativePrompt.right = sg_nScrWidth;
	sgAlternativeOptions[0].top = sgAlternativePrompt.bottom + (sgAlternativeButtons[0].top - sgAlternativePrompt.bottom - sg_nHeightOfALine)/2;
	sgAlternativeOptions[0].bottom = sgAlternativeOptions->top + sg_nHeightOfALine;
	if (sg_nScrWidth < sg_nScrHeight) {
		sgAlternativeOptions[1].top = sgAlternativeOptions[0].bottom+2;
		sgAlternativeOptions[1].bottom = sgAlternativeOptions[1].top + sg_nHeightOfALine;

		sgAlternativeOptions[0].left = sgAlternativeOptions[1].left = sg_nScrWidth / 4;
		sgAlternativeOptions[0].right = sgAlternativeOptions[1].right = sgAlternativeOptions[0].left * 3;
	}
	else {
		sgAlternativeOptions[1].top = sgAlternativeOptions[0].top;
		sgAlternativeOptions[1].bottom = sgAlternativeOptions[0].bottom;

		sgAlternativeOptions[0].left = 5;
		sgAlternativeOptions[0].right = sgAlternativeOptions[1].left = sg_nScrWidth / 2;
		sgAlternativeOptions[1].right = sg_nScrWidth-5;
	}

	//menu
	nKeyWidth = sg_nScrWidth / 4;
	{
		int keys[] = {KEYCANCEL, GUI_KEYPREV, GUI_KEYNEXT, KEYENTER};
		for (i = 0; i < 4; ++i) {
			sgMenuButtons[i].top = sg_nScrHeight - sg_nHeightOfALine;
			sgMenuButtons[i].bottom = sg_nScrHeight - 1;
			sgMenuButtons[i].left = i  * nKeyWidth;
			sgMenuButtons[i].right = sgKeypad[i].left + nKeyWidth;
			sgMenuButtons[i].nValue = keys[i];
		}
	}
	sgMenuListArea.top = sgTitleArea.bottom;
	sgMenuListArea.bottom = sgMenuButtons[0].top;
	sgMenuListArea.left = 0;
	sgMenuListArea.right = sg_nScrWidth;

	//info page
	memcpy(sgInfoPageButtons, sgMenuButtons, 4 * sizeof(Rect));
	sgInfoPageButtons[1].nValue = KEYUP;
	sgInfoPageButtons[2].nValue = KEYDOWN;
	sgInfoPageArea.top = sgTitleArea.bottom;
	sgInfoPageArea.bottom = sgInfoPageButtons[0].top - 1;
	sgInfoPageArea.left = 0;
	sgInfoPageArea.right = sg_nScrWidth;
	sgInfoPageArea.bottom = sgInfoPageButtons[0].top - 1;
	if(!sg_isColorScreen){
		sgInfoPageArea.bottom = sg_nScrHeight;
	}
	else{
		sgInfoPageMenu.top = 0;
		sgInfoPageMenu.bottom = sgTitleArea.bottom;
		sgInfoPageMenu.left = sg_nScrWidth - nKeyWidth;
		sgInfoPageMenu.right = sg_nScrWidth;
		sgInfoPageMenu.nValue = KEYMENU;
		sgInfoPageMenuOpt[0] = sgInfoPageMenuOpt[1] = sgInfoPageMenu;
		sgInfoPageMenuOpt[0].top = sgInfoPageMenu.bottom;
		sgInfoPageMenuOpt[0].bottom = sgInfoPageMenuOpt[0].top + sg_nHeightOfALine;
		sgInfoPageMenuOpt[0].nValue = GUI_KEYPREV;
		sgInfoPageMenuOpt[1].top = sgInfoPageMenuOpt[0].bottom;
		sgInfoPageMenuOpt[1].bottom = sgInfoPageMenuOpt[1].top + sg_nHeightOfALine;
		sgInfoPageMenuOpt[1].nValue = GUI_KEYNEXT;
	}

	memcpy(sgSignatureButtons, sgMsgBoxButtons, 3 * sizeof(Rect));
	sgSignatureButtons[2].nValue = KEYCLEAR;

	sgSignatureButtons[0].top = sgSignatureButtons[1].top = sgSignatureButtons[2].top = sgSignatureButtons[2].top + 5;

	sgSignatureArea.top = sgTitleArea .bottom + 5;
	sgSignatureArea.bottom = sgSignatureButtons[0].top - 5;
	sgSignatureArea.left = 5;
	sgSignatureArea.right = sg_nScrWidth - 5;
}

static unsigned char OpenTouchScreen()
{
	char isTouchScreen = FALSE;
	TouchScreenClose();

//TODO Kim RF & Touchscreen conflict
    if(sg_hasTouchScreen && 0 == TouchScreenOpen(0)) {
		isTouchScreen = TRUE;
    }
	UpdateCanvasSize(); // MEDC-2
    return isTouchScreen;
}

static const Rect *FindMatchedButton(const TS_POINT_T *pPt, const Rect *pButtons, int nButtonNum)
{
    int i;
    if(pPt && pButtons) {
        for(i = 0; i < nButtonNum; ++i) { // cancel & ok
            if(_IsIncluded(pPt->x,pPt->y, pButtons[i].top, pButtons[i].bottom,pButtons[i].left, pButtons[i].right)) {
                return &pButtons[i];
            }
        }
    }
    return NULL;
}
//end of file

