
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
enum{
	PRN_6x8=0,
	PRN_8x16,
	PRN_16x16,
	PRN_12x24,
	PRN_24x24,
	PRN_6x12,
	PRN_12x12,
	PRN_NULL=0xFF
};





/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// For thermal, small={8x16,16x16}
// For sprocket, small=normal={6x8,16x16}
// Modified by Kim_LinHB 2014-6-8
void PrnSetSmall(void)
{
	if (ChkIfThermalPrinter())
	{
#ifdef AREA_Arabia
		if(0 == strcmp(LANGCONFIG, "Arabic"))
		{
			if (ArPrnFontSelect(gl_AR_FONT_ID, 4)!=AR_SUCCESS)
			{
				while(1)
				{
					ScrCls();
					ScrPrint(0,0,0,"Error:PAX_ARFA.FONT error\nPls download ParamFile");
					getkey();
				}
			}
		}
		else
#endif
		{
#if defined(_Sxx_) || defined(_SP30_) || defined(_Sxxx_)
			PrnFontSetNew(PRN_8x16, PRN_16x16);
			PrnSpaceSet(1, 2);
#else
			PrnFontSet(0, 0);
			PrnSpaceSet(1, 2);
#endif
		}
	}
	else
	{
		PrnSetNormal();
	}
}

// For thermal, normal={12x24,24x24}
// For sprocket, normal={6x8,16x16}
// Modified by Kim_LinHB 2014-6-8
void PrnSetNormal(void)
{
	if (ChkIfThermalPrinter())
	{
#ifdef AREA_Arabia
		if(0 == strcmp(LANGCONFIG, "Arabic"))
		{
			if (ArPrnFontSelect(gl_AR_FONT_ID, 6)!=AR_SUCCESS)
			{
				while(1)
				{
					ScrCls();
					ScrPrint(0,0,0,"Error:PAX_ARFA.FONT error\nPls download ParamFile");
					getkey();
				}
			}
		}
		else
#endif
		{
#if defined(_Sxx_) || defined(_SP30_) || defined(_Sxxx_)
		PrnFontSetNew(PRN_12x24, PRN_24x24);
#else
		PrnFontSet(1, 1);
#endif
		PrnSpaceSet(1, 3);
		}
	}
	else
	{
#if defined(_Sxx_) || defined(_SP30_) || defined(_Sxxx_)
		PrnFontSetNew(PRN_6x8, PRN_16x16);
#else
		PrnFontSet(0, 0);
#endif
		PrnSpaceSet(0, 2);
	}
}

// For thermal, normal=big={12x24,24x24}
// For sprocket, big={8x16,16x16}
// Modified by Kim_LinHB 2014-6-8
void PrnSetBig(void)
{
	if (ChkIfThermalPrinter())
	{
#ifdef AREA_Arabia
		if(0 == strcmp(LANGCONFIG, "Arabic"))
		{
			if (ArPrnFontSelect(gl_AR_FONT_ID, 8)!=AR_SUCCESS)
			{
				while(1)
				{
					ScrCls();
					ScrPrint(0,0,0,"Error:PAX_ARFA.FONT error\nPls download ParamFile");
					getkey();
				}
			}
		}
		else
#endif
		{
			PrnSetNormal();
		}
	}
	else
	{
#if defined(_Sxx_) || defined(_SP30_) || defined(_Sxxx_)
		PrnFontSetNew(PRN_8x16, PRN_16x16);
#else
		PrnFontSet(1, 1);
#endif
		PrnSpaceSet(0, 2);
	}
}


// Modified by Kim_LinHB 2014-6-8
int DispPrnError(int iErrCode)
{
	unsigned char szBuff[100];
	Gui_ClearScr();
	PubBeepErr();
	switch( iErrCode )
	{
	case ERR_PRN_BUSY:
		strcpy(szBuff, _T("PRINTER BUSY"));
		break;

	case ERR_PRN_PAPEROUT:
		strcpy(szBuff, _T("OUT OF PAPER"));
		return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL);
		break;

	case ERR_PRN_WRONG_PACKAGE:
		strcpy(szBuff, _T("PRINTER DATA ERROR"));
		break;

	case ERR_PRN_OVERHEAT:
		strcpy(szBuff, _T("PRINTER OVERHEAT"));
		break;

	case ERR_PRN_OUTOFMEMORY:
		strcpy(szBuff, _T("PRINTER OVERFLOW"));
		break;

	case PRN_NO_FONT:
		strcpy(szBuff, _T("PLEASE LOAD FONT"));
		break;

	default:
		strcpy(szBuff, _T("PRINTER ERROR"));
		break;
	}
	return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
}


// end of file

