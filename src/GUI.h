#ifndef _GUI_H_
#define _GUI_H_

#pragma pack(4)
#define 	GUI_OK		0
#define	GUI_ERR_UNSUPPORTED				-1
#define	GUI_ERR_NOTINIT					-2
#define	GUI_ERR_INVALIDPARAM			-3
#define	GUI_ERR_USERCANCELLED			-4
#define	GUI_ERR_TIMEOUT					-5
#define	GUI_ERR_EXCEED					-6
#define	GUI_ERR_CANTCALLFROMCALLBACK	-9
#define	GUI_RETURNFROMCALLBACK 			-10
#define	GUI_ERR_SYSTEM		 			-11

//for info page
#ifndef 	GUI_KEYPREV
#define 	GUI_KEYPREV		600
#define 	GUI_KEYNEXT		601
#endif
//for info page
#define		GUI_OK_NEXT			0
#define		GUI_OK_PREVIOUS		1

//0xFFRRGGBB
#ifndef _RGB_INT_
#define _RGB_INT_(_r, _g, _b) 	((unsigned int)(((unsigned int)(0xFF << 24)) | ((unsigned int)(_r << 16)) | ((unsigned int)(_g << 8)) | ((unsigned int)(_b))))
#endif
#define _GetRValue(_argb)		((((unsigned int)(_argb)) >> 16) & 0x000000FF)
#define _GetGValue(_argb)		((((unsigned int)(_argb)) >> 8) & 0x000000FF)
#define _GetBValue(_argb)		(((unsigned int)(_argb))  & 0x000000FF)

#define GUI_TIMER_INDEX			3
#define GUI_TIMER_INPUT_TEMP	4

enum ALIGNMENT {
    GUI_ALIGN_LEFT = 0, GUI_ALIGN_CENTER, GUI_ALIGN_RIGHT
};
enum FONTSTYLE {
    GUI_FONT_STD = 0, GUI_FONT_BOLD = 0x01, GUI_FONT_ITALIC = 0x02, GUI_FONT_REVERSAL = 0x04, GUI_FONT_OPAQUE = 0x08
};
enum FONTSIZE {
    GUI_FONT_SMALL = 0, GUI_FONT_NORMAL, GUI_FONT_LARGE
};

enum MSGBOXTYPE {
    GUI_BUTTON_NONE, GUI_BUTTON_OK, GUI_BUTTON_CANCEL, GUI_BUTTON_YandN
};
enum INPUTBOXTYPE {
    GUI_INPUT_NUM, GUI_INPUT_MIX,
};
enum MENUSTYLE {
    GUI_MENU_MANUAL_INDEX = 0x00000001, GUI_MENU_0_START = 0x00000001 << 1, //index from 0, off when GUI_MENU_ARROW is on(def if from 1)
    GUI_MENU_DIRECT_RETURN = 0x00000001 << 2, //cancel to exit menu
};

//////////////////////////////////////////////////////////////////////////

typedef struct _Gui_ResText_Attr {
    enum FONTSIZE eFontSize;
    enum ALIGNMENT eAlign;
    enum FONTSTYLE eStyle;
} GUI_TEXT_ATTR;

typedef void (*MENUFUNC)(void);
typedef struct _Gui_ResMenuItem {
    unsigned char szText[30];
    int nValue; /*id or key value*/
    unsigned char bVisible;
    MENUFUNC vFunc;
} GUI_MENUITEM;

typedef struct _Gui_ResMenu {
    GUI_TEXT_ATTR stTitleAttr;
    GUI_TEXT_ATTR stItemAttr;
    unsigned int nSize;
    GUI_MENUITEM *pstContent;
    unsigned char szTitle[30];
} GUI_MENU;

typedef struct _Gui_InputBox_ATTR {
    enum INPUTBOXTYPE eType;
    unsigned char nMinLen;
    unsigned char nMaxLen;

    /*
     available when mode num/mix
     */
    unsigned char bSensitive;

    /*
     unavailable when bSensitive
     */
    unsigned char bEchoMode;
} GUI_INPUTBOX_ATTR;

typedef struct _Gui_PageLine {
    unsigned char szLine[30];
    GUI_TEXT_ATTR stLineAttr;
} GUI_PAGELINE;

typedef struct _Gui_ResPage {
    GUI_TEXT_ATTR stTitleAttr;
    unsigned int nLine;
    unsigned char szTitle[30];
    GUI_PAGELINE *pstContent;
} GUI_PAGE;

#ifdef __cplusplus
extern "C" {
#endif

// Modified by Kim_LinHB 2014-08-25 v0.4
//************************************
// Function:    Gui_Init
// Description: clear screen with background color, and set foreground color
// Returns:     int
//				GUI_OK
//				GUI_ERR_INVALIDPARAM
// Parameter:   enum BACKGROUND eBg
// Parameter:   unsigned int nColor
//************************************
int Gui_Init(unsigned int nBgColor, unsigned int nColor, const unsigned char *pszArabicFile);

//************************************
// Function:    Gui_ClearScr
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//************************************
int Gui_ClearScr();

//************************************
// Function:    Gui_LoadFont
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
// Parameter:   enum FONTSIZE eFontSize
// Parameter:   const ST_FONT * pSingleCodeFont
// Parameter:   const ST_FONT * pMultiCodeFont
//************************************
int Gui_LoadFont(enum FONTSIZE eFontSize, const ST_FONT *pSingleCodeFont, const ST_FONT *pMultiCodeFont);

//************************************
// Function:    Gui_DrawText
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
// Parameter:   const unsigned char * pszText
// Parameter:   const GUI_TEXT_ATTR stTextAttr
// Parameter:   unsigned int x_percent
// Parameter:   unsigned int y_percent
//************************************
int Gui_DrawText(const unsigned char *pszText, const GUI_TEXT_ATTR stTextAttr, unsigned int x_percent, unsigned int y_percent);

// Modified by Kim_LinHB 2014-8-11 v0.3
//************************************
// Function:    Gui_DrawLogo
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
// Parameter:   const unsigned char * psLogo
// Parameter:   int x
// Parameter:   int y
//************************************
int Gui_DrawLogo(const unsigned char *psLogo, int x, int y);

//************************************
// Function:    Gui_DrawImage
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_UNSUPPORTED
//				GUI_ERR_INVALIDPARAM
// Parameter:   const unsigned char * pszImagePath
// Parameter:   enum IMAGETYPE type
// Parameter:   unsigned int x_percent
// Parameter:   unsigned int y_percent
//************************************
int Gui_DrawImage(const unsigned char *pszImagePath, unsigned int x_percent, unsigned int y_percent);

//************************************
// Function:    Gui_ShowMsgBox
// Description: show message box
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const unsigned char * pszTitle
// Parameter:   GUI_TEXT_ATTR stTitleAttr
// Parameter:   const unsigned char * pszContent
// Parameter:   GUI_TEXT_ATTR stContentAttr
// Parameter:   enum MSGBOXTYPE eType
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
// Parameter:   int * pucKeyValue
//************************************
int Gui_ShowMsgBox(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr, const unsigned char *pszContent, GUI_TEXT_ATTR stContentAttr, enum MSGBOXTYPE eType, int timeoutSec, int *pucKeyValue);

//************************************
// Function:    Gui_ShowInputBox
// Description: show input box
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const unsigned char * pszTitle
// Parameter:   GUI_TEXT_ATTR stTitleAttr
// Parameter:   const unsigned char * pszPrompt
// Parameter:   GUI_TEXT_ATTR stPromptAttr
// Parameter:   unsigned char * pszContent
// Parameter:   GUI_TEXT_ATTR stContentAttr
// Parameter:   const GUI_INPUTBOX_ATTR * pstAttr
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
//************************************
int Gui_ShowInputBox(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr, const unsigned char *pszPrompt, GUI_TEXT_ATTR stPromptAttr, unsigned char *pszContent, GUI_TEXT_ATTR stContentAttr, const GUI_INPUTBOX_ATTR *pstAttr, int timeoutSec);

//************************************
// Function:    Gui_ShowTimeBox
// Description: show time setting box
// Returns:		int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const unsigned char * pszTitle
// Parameter:   GUI_TEXT_ATTR stTitleAttr
// Parameter:   unsigned char * pszTime
// Parameter:   GUI_TEXT_ATTR stContentAttr
// Parameter:   unsigned char isTime
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
//************************************
int Gui_ShowTimeBox(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr, unsigned char *pszTime, GUI_TEXT_ATTR stContentAttr, unsigned char isTime, int timeoutSec);

//************************************
// Function:    GUI_BindMenu
// Description: bind menu items, menu title to a menu
// Returns:     int
//				GUI_OK
//				GUI_ERR_INVALIDPARAM
// Parameter:   const unsigned char * psztitle
// Parameter:   GUI_TEXT_ATTR stTextAttr
// Parameter:   GUI_MENU * pstMenu
// Parameter:   GUI_MENUITEM * pstMenuItem
//************************************
int Gui_BindMenu(const unsigned char *psztitle, GUI_TEXT_ATTR stTitleAttr, GUI_TEXT_ATTR stTextAttr, const GUI_MENUITEM *pstMenuItem, GUI_MENU *pstMenu);

//************************************
// Function:    Gui_ShowMenuList
// Description: show menu list
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const GUI_MENU * pstMenu
// Parameter:   enum MENUSTYLE eMode
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
// Parameter:   int *piSelValue
//************************************
int Gui_ShowMenuList(const GUI_MENU *pstMenu, enum MENUSTYLE eMode, int timeoutSec, int *piSelValue);

int Gui_ShowMenuListWithoutButtons(const GUI_MENU *pstMenu, enum MENUSTYLE eMode, int timeoutSec, int *piSelValue);


//************************************
// Function:    Gui_ShowAlternative
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_TIMEOUT
//				GUI_ERR_USERCANCELLED
// Parameter:   const unsigned char * pszTitle
// Parameter:   GUI_TEXT_ATTR stTitleAttr
// Parameter:   const unsigned char * pszPrompt
// Parameter:   GUI_TEXT_ATTR stContentAttr
// Parameter:   const unsigned char * pszOption1
// Parameter:   unsigned char ucValue1
// Parameter:   const unsigned char * pszOption2
// Parameter:   unsigned char ucValue2
// Parameter:   int timeoutSec
// Parameter:   [input & output]unsigned char * pucSelOption
//************************************
int Gui_ShowAlternative(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr, const unsigned char *pszPrompt, GUI_TEXT_ATTR stContentAttr, const unsigned char *pszOption1, int iValue1, const unsigned char *pszOption2, int iValue2, int timeoutSec, int *piSelOption);

//************************************
// Function:    Gui_CreateInfoPage
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_INVALIDPARAM
// Parameter:   const unsigned char * psztitle
// Parameter:   GUI_TEXT_ATTR stTextAttr
// Parameter:   const GUI_PAGELINE *pstContent
// Parameter:   unsigned int nLine
// Parameter:   GUI_PAGE * pstPage
//************************************
int Gui_CreateInfoPage(const unsigned char *psztitle, GUI_TEXT_ATTR stTitleAttr, const GUI_PAGELINE *pstContent, unsigned int nLine, GUI_PAGE *pstPage);

// Modified by Kim_LinHB 2014-8-8 v0.3
//************************************
// Function:    Gui_ShowInfoPage
// Description: 
// Returns:     int
//				GUI_OK_NEXT
//				GUI_OK_PREVIOUS
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const GUI_PAGE * pstPage
// Parameter:	unsigned char isMultiChapters
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
//************************************
int Gui_ShowInfoPage(const GUI_PAGE *pstPage, unsigned char isMultiChapters, int timeoutSec);

//************************************
// Function:    Gui_ShowSignatureBoard
// Description:
// Returns:     int
//				GUI_OK
//				GUI_ERR_UNSUPPORTED
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const unsigned char *pszTitle
// Parameter:	GUI_TEXT_ATTR stTitleAttr,
// Parameter:   const unsigned char *pszOutputFile,
// Parameter:   char nMode  (reserved)
//				0: png
//				1: coordinate
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
//************************************
int Gui_ShowSignatureBoard(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr,
		const unsigned char *pszOutputFile,
		char nMode, int timeoutSec);

// Added by Kim_LinHB 2014-08-13 v0.3
//************************************
// Function:    Gui_GetImageSize
// Description: 
// Returns:     int
// Parameter:   const unsigned char * File
// Parameter:   int * pWidth
// Parameter:   int * pHeight
//************************************
int Gui_GetImageSize(const unsigned char *File, unsigned int *pWidth, unsigned int *pHeight);

//************************************
// Function:    Gui_GetScrWidth
// Description: Get the content screen width
// Returns:     int Width
//************************************
int Gui_GetScrWidth(void);

//************************************
// Function:    Gui_GetScrHeight
// Description: Get the content screen height
// Returns:     int Height
//************************************
int Gui_GetScrHeight(void);

//************************************
// Function:    Gui_UpdateTitle
// Description: update the title
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
// Parameter:   const char * pszTitle
// Parameter:	GUI_TEXT_ATTR stTitleAttr
//************************************
int Gui_UpdateTitle(const char *pszTitle, GUI_TEXT_ATTR stTitleAttr);

//************************************
// Function:    Gui_UpdateKey
// Description: set text for a key value
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_EXCEED
// Parameter:   int index	[limit 100]
// Parameter:	const char* text
//************************************
int Gui_UpdateKey(int index, const char* text);

struct _Gui_Callback_Text{
	int x,y;
	int size;
	char *pStr;
};

typedef enum GUI_CALLBACKTYPE{
	GUI_CALLBACK_LISTEN_EVENT,		// just allow GUI_DrawXXX in this callback function
	GUI_CALLBACK_UPDATE_TEXT, 		//(for ex. update number to Persian number, special symbol)
	// data: struct _Gui_Callback_Text
}gui_callbacktype_t;

typedef int (*GUI_CALLBACK)(gui_callbacktype_t type, void *data, int *dataLen);
//************************************
// Function:    Gui_RegCallback
// Description: register/un-register a callback function for some special events need to be handled during displaying page
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
// Parameter:   gui_callbacktype_t type
// Parameter:	GUI_CALLBACK func
//************************************
int Gui_RegCallback(gui_callbacktype_t type, GUI_CALLBACK func);

#ifdef __cplusplus
}
#endif
#pragma pack()

#endif
