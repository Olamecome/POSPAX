////////////////////////////////////////////////////////////////////////////
//              PAX CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with PAX Corporation and may not be copied
//  or disclosed except in accordance with the terms in that agreement.
//      Copyright(c) 2011 PAX Corporation. All rights reserved.
//
//  Author:     Harry Gao
//  Created:    2011/03/01
//
/// @file       arabic_farsi_api.h
/// @brief      Define the PAX Arabic and Farsi system interfaces.
/// @version    1.0
////////////////////////////////////////////////////////////////////////////


#ifndef _ARABIC_FARSI_API_H_
#define _ARABIC_FARSI_API_H_


#ifndef uchar
	#define uchar unsigned char
#endif
#ifndef uint
	#define uint unsigned int
#endif
#ifndef ulong
	#define ulong unsigned long
#endif
#ifndef ushort
	#define ushort unsigned short
#endif


// ======================== Common Macros Begin,  AR_*** ====================


#define     AR_SUCCESS              0x00	/// @brief sucess
#define     AR_OPENFILE_ERROR       0xFF	/// @brief open file error
#define     AR_READFILE_ERROR       0xFE	/// @brief read file error
#define     AR_PARA_ERROR           0xFD	/// @brief parameter error
#define     AR_FONT_NOT_SELECT		0xFC	/// @brief font not select error
#define     AR_OPEN_TOO_MANY		0xFB	/// @brief open too many font files
#define		AR_ERROR_CODE_MIN		0xF0	/// @brief all the error code must larger than this

#define     AR_ALIGN_RIGHT          0x00	/// @brief align right
#define     AR_ALIGN_CENTER         0x01	/// @brief align center
#define     AR_ALIGN_LEFT           0x02	/// @brief align left

#define		AR_FILE_DEFAULT			"PAXARFA.FONT"	/// @brief default font file name

#define		AR_ERR_STR_TOO_LONG		-1		/// @brief string is too long
#define		AR_ERR_FONT_NOT_SELECT	-2		/// @brief font not select error
#define		AR_ERR_STR_IS_NULL		-3		/// @brief string is NULL


// ======================== Common Macros End ====================


// ======================== Common Methods Begin, Ar*** ====================


///////////////////////////////////////
///  @brief			Open specified font file.
///  @param	[in]	fileName		Name of font file
///  @return		AR_PARA_ERROR/AR_OPENFILE_ERROR/AR_OPER_TOO_MANY/AR_READFILE_ERROR
///  @note			At least one font file should be opened before display or print;
///					No more than ten font files can be opened at same time;
///					Caller of this function need maintain the returned file handler;
///					The suffix identify the type of font , only ".FONT" is supported now;
///					After call ArFontOpen , it is necessary to call ArPrnFontSelect or ArScrFontSelect before printing or displaying some content.
///  @see			ArFontClose
///////////////////////////////////////
uint ArFontOpen(uchar *fileName);


///////////////////////////////////////
///  @brief			Close specified font file.
///  @param [in]	fileID			The file handler
///  @return		NONE
///  @note			Call this function when the specified font file is not used any more.
///  @see			ArFontOpen
///////////////////////////////////////
void ArFontClose(uint fileID);


///////////////////////////////////////
///  @brief			Fetch the amount of font in the specified font file.
///  @param [in]	fileID			The file handler
///  @return		the real amount/AR_PARA_ERROR/AR_READFILE_ERROR
///  @note			Can be used with ArFontHeight , ArFontIsBold and ArFontIsItalic to choose a font if the font file content is not confirmed.
///  @see			ArFontHeight , ArFontIsBold and ArFontIsItalic
///////////////////////////////////////
uchar ArFontAmount(uint fileID);


///////////////////////////////////////
///  @brief			Get the height of specified font in the specified file.
///  @param [in]	fileID			The file handler
///  @param [in]	fontID			font index in the font file
///  @return		the font height/AR_PARA_ERROR/AR_READFILE_ERROR
///  @note			There can be several fonts with same height in one font file, each font will assigned a auto-increased ID;
///					This function can be used with ArFontAmount to enumerate and choose the suitable font.
///  @see			ArFontAmount
///////////////////////////////////////
uchar ArFontHeight(uint fileID, uchar fontID);


///////////////////////////////////////
///  @brief			Get if this font is bold or not.
///  @param [in]	fileID			The file handler
///  @param [in]	fontID			font index in the font file
///  @return		0 (not bold) / 1 (is bold)/AR_PARA_ERROR/AR_READFILE_ERROR
///  @note			This function can be used with ArFontAmount to enumerate and choose the suitable font.
///  @see			ArFontAmount
///////////////////////////////////////
uchar ArFontIsBold(uint fileID, uchar fontID);


///////////////////////////////////////
///  @brief			Get if this font is italic or not.
///  @param [in]	fileID			The file handler
///  @param [in]	fontID			font index in the font file
///  @return		0 (not italic) / 1 (is italic)/AR_PARA_ERROR/AR_READFILE_ERROR
///  @note			This function can be used with ArFontAmount to enumerate and choose the suitable font.
///  @see			ArFontAmount
///////////////////////////////////////
uchar ArFontIsItalic(uint fileID, uchar fontID);


///////////////////////////////////////
///  @brief			Choose the number type for screen and printer.		
///  @param [in]	type			0 : Arabic type ; non-0 : Indian type
///  @return		NONE
///  @note			Arabic number is the default type.
///  @see
///////////////////////////////////////
void ArNumberType(uchar type);


// ======================== Common Methods End ====================


// ======================== Printer Methods Begin, ArPrn*** ====================


///////////////////////////////////////
///  @brief			Printing content at current position
///  @param [in]	str			The content to be printed
///  @return		AR_SUCCESS/AR_READFILE_ERROR/AR_PARA_ERROR
///  @note			Support '\n'; No more than 1024 bytes.
///					Properties setting functions defined in this module can be used to set properties before printing, the properties will be effective until the related function is called again;
///					This function only stores content into printing buffer, call PrnStrat() for stating printing.
///  @see
///////////////////////////////////////
uchar ArPrnPrint(uchar *str);


///////////////////////////////////////
///  @brief			Choose a specified font in specified font file for printing.
///  @param [in]	fileID			The file handler
///  @param [in]	prnFontID		font index in the font file
///  @return		AR_SUCCESS/AR_NOT_INIT_ERROR/AR_READFILE_ERROR/AR_PARA_ERROR
///  @note			Must select one font before print, and font can be changed at any time;
///					Only change the printing font, not affect the displaying font, also not affect the previous printing properties.
///					The font ID is auto-increment and start from 1, the maximal depends on the specified font file.
///					The font ID and font type in default font file(PAXARFA.FONT) are:
///					1  : 8 dots height font
///					2  : 12 dots height font
///					3  : 16 dots height font
///					4  : 24 dots height font
///					5  : 24 dots height font, bold
///					6  : 32 dots height font
///					7  : 32 dots height font, bold
///					8  : 40 dots height font
///					9  : 40 dots height font, bold		
///  @see
///////////////////////////////////////
uchar ArPrnFontSelect(uint fileID, uchar prnFontID);


///////////////////////////////////////
///  @brief			Set the printing left indent.
///  @param [in]	indent			Value in pixels
///  @return		Real indent in [0, 300]
///  @note			Default left indent is 0;
///					The sum of left indent and right indent is no larger than 300, 
///					or else (300 - right indent) is returned as left indent;
///					It should be divisible by 8, or else (indent - indent % 8) is returned.
///  @see			ArPrnRightIndent
///////////////////////////////////////
ushort ArPrnLeftIndent(ushort indent);


///////////////////////////////////////
///  @brief			Set the printing right indent.
///  @param [in]	indent			Value in pixels
///  @return		Real indent in [0, 300]
///  @note			Default right indent is 0; 
///					The sum of left indent and right indent is no larger than 300,
///					or else (300 - left indent) is returned as right indent;
///					It should be divisible by 8, or else (indent - indent % 8) is returned.
///  @see			ArPrnLeftIndent
///////////////////////////////////////
ushort ArPrnRightIndent(ushort indent);


///////////////////////////////////////
///  @brief			Set the printing line spacing.
///  @param [in]	value			The line spacing
///  @return		Actual effect line spacing, value in [0, 80]
///  @note			Default line spacing is 0, larger than 80 will use and return 80;
///  @see
///////////////////////////////////////
ushort ArPrnLineSpacing(ushort value);


///////////////////////////////////////
///  @brief			Set using double width to print or not.
///  @param [in]	flag			0 : single width ; non-0 : double width
///  @return		NONE
///  @note			Default double width flag is 0;
///					Double each character's width, can be used with PrnDoubleHeight.
///  @see			ArPrnDoubleHeight
///////////////////////////////////////
void ArPrnDoubleWidth(uchar flag);


///////////////////////////////////////
///  @brief			Set using double height to print or not.
///  @param [in]	flag			0 : single height ; non-0 : double height
///  @return		NONE
///  @note			Default double height flag is 0;
///					Double each character's height, can be used with ArPrnDoubleWidth .
///  @see			ArPrnDoubleWidth
///////////////////////////////////////
void ArPrnDoubleHeight(uchar flag);


///////////////////////////////////////
///  @brief			Optimize one line's height from the top and bottom.
///  @param [in]	flag			0 : not optimize; non-0 : optimize the height
///  @param [in]	maxBlank		the max blank dot in height dimension for one line's top and bottom.
///  @return		NONE
///  @note			Implement variable height according to the contents of a line.
///					e.g.
///					Use 8 dots heights font, call ArPrnLineHeightOptimize(1, 1), then:
///					(1)If content is "--------------", then line height will cut from 8 to 3;
///					(2)If content is "========", then line height will cut from 8 to 6.
///  @see
///////////////////////////////////////
void ArPrnLineHeightOptimize(uchar flag, uchar maxBlank);


///////////////////////////////////////
///  @brief			Set using reverse mode to print or not.
///  @param [in]	flag			0 : normal mode ; non-0 : reverse mode
///  @param [in]	fillLine		Used in reverse mode; 0 : reverse characters only ; non-0 : reverse the whole line
///  @return		NONE
///  @note			Set fillLine to non-0 when the whole line need to be reverse printed.
///  @see
///////////////////////////////////////
void ArPrnReverse(uchar flag, uchar fillLine);


///////////////////////////////////////
///  @brief			Set the printing content's align mode.
///  @param [in]	align			AR_ALIGN_RIGHT/AR_ALIGN_CENTER/AR_ALIGN_LEFT
///  @return		NONE
///  @note			Default align mode is AR_ALIGN_RIGHT
///  @see
///////////////////////////////////////
void ArPrnAlign(uchar align);


///////////////////////////////////////
///  @brief			Get string width according to current printer font
///  @param [in]	str				The string to be calculated
///  @return		The string width/AR_ERR_STR_TOO_LONG/AR_ERR_FONT_NOT_SELECT
///  @note			No more than 1024 bytes
///  @see			ArScrGetStrWidth
///////////////////////////////////////
int ArPrnGetStrWidth(uchar *str);


// ======================== Printer Methods End ====================


// ======================== Screen Methods Begin, ArScr*** ====================


///////////////////////////////////////
///  @brief			Displaying content at screen position (x, y)
///  @param [in]	x				x coordinate
///  @param [in]	y				y coordinate
///  @param [in]	str				The content to be displayed
///  @return		AR_SUCCESS/AR_READFILE_ERROR/AR_PARA_ERROR
///  @note			No more than 1024 bytes.
///					Properties setting functions defined in this module can be used to set properties before displaying, 
///					the properties will be effective until the related function is called again.
///  @see			ArScrPrint, ArScrPrintAtX, ArScrPrintAtY
///////////////////////////////////////
uchar ArScrPrintAtXY(ushort x, ushort y, uchar *str);


///////////////////////////////////////
///  @brief			Displaying content at current screen position
///  @param [in]	str				The content to be displayed
///  @return		AR_SUCCESS/AR_READFILE_ERROR/AR_PARA_ERROR
///  @note			No more than 1024 bytes
///  @see			ArScrPrintAtXY, ArScrPrintAtX, ArScrPrintAtY
///////////////////////////////////////
uchar ArScrPrint(uchar *str);


///////////////////////////////////////
///  @brief			Displaying content at screen position (x, current y)
///  @param [in]	x				x coordinate
///  @param [in]	str				The content to be displayed
///  @return		AR_SUCCESS/AR_READFILE_ERROR/AR_PARA_ERROR
///  @note			No more than 1024 bytes
///  @see			ArScrPrint, ArScrPrintAtXY, ArScrPrintAtY
///////////////////////////////////////
uchar ArScrPrintAtX(ushort x, uchar *str);


///////////////////////////////////////
///  @brief
///  @param [in]	y				y coordinate
///  @param [in]	str				The content to be displayed
///  @return		AR_SUCCESS/AR_READFILE_ERROR/AR_PARA_ERROR
///  @note			No more than 1024 bytes
///  @see			ArScrPrint, ArScrPrintAtXY, ArScrPrintAtX
///////////////////////////////////////
uchar ArScrPrintAtY(ushort y, uchar *str);


///////////////////////////////////////
///  @brief			Choose a specified font in specified font file for screen displaying.
///  @param [in]	fileID			The file handler
///  @param [in]	scrFontID		font index in the font file
///  @return		AR_SUCCESS/AR_NOT_INIT_ERROR/AR_READFILE_ERROR/AR_PARA_ERROR
///  @note			Must select one font before display, and it can be changed at any time;
///					Only change the screen displaying font, not affect the printing font, also not affect the previous screen displaying properties.
///					The font ID is auto-increment and start from 1, the maximal depends on the specified font file.
///					The font ID and font type in default font file(PAXARFA.FONT) are:
///					1  : 8 dots height font
///					2  : 12 dots height font
///					3  : 16 dots height font
///					4  : 24 dots height font
///					5  : 24 dots height font, bold
///					6  : 32 dots height font
///					7  : 32 dots height font, bold
///					8  : 40 dots height font
///					9  : 40 dots height font, bold
///  @see			ArPrnFontSelect
///////////////////////////////////////
uchar ArScrFontSelect(uint fileID, uchar scrFontID);


///////////////////////////////////////
///  @brief			Set the screen left indent.
///  @param [in]	indent			Value in pixels
///  @return		Real indent in [0, 96]
///  @note			Default left indent is 0;
///					The sum of left indent and right indent is no larger than 96, or else (96 - right indent) is returned as left indent;
///					It should be divisible by 8, or else (indent - indent % 8) is returned.
///  @see			ArScrRightIndent
///////////////////////////////////////
ushort ArScrLeftIndent(ushort indent);


///////////////////////////////////////
///  @brief			Set the printing right indent.
///  @param [in]	indent			Value in pixels
///  @return		Real indent in [0, 96]
///  @note			Default right indent is 0;
///					The sum of left indent and right indent is no larger than 300, or else (300 - left indent) is returned as right indent;
///					It should be divisible by 8, or else (indent - indent % 8) is returned.
///  @see			ArScrLeftIndent
///////////////////////////////////////
ushort ArScrRightIndent(ushort indent);


///////////////////////////////////////
///  @brief			Set the displaying line spacing
///  @param [in]	value			The line spacing
///  @return		Actual effect line spacing, value in [0, 16]
///  @note			Default line spacing is 0, larger than 16 will use and return 16
///  @see
///////////////////////////////////////
ushort ArScrLineSpacing(ushort value);


///////////////////////////////////////
///  @brief			Set using double width to display or not.
///  @param [in]	flag			0 : single width ; non-0 : double width
///  @return		NONE
///  @note			Default double width flag is 0
///					Double each character's width, can be used with ArScrDoubleHeight .
///  @see			ArScrDoubleHeight
///////////////////////////////////////
void ArScrDoubleWidth(uchar flag);


///////////////////////////////////////
///  @brief			Set using double height to display or not.
///  @param [in]	flag			0 : single height ; non-0 : double height
///  @return		NONE
///  @note			Default double height flag is 0
///					Double each character's height, can be used with ArScrDoubleWidth.
///  @see			ArScrDoubleWidth
///////////////////////////////////////
void ArScrDoubleHeight(uchar flag);


///////////////////////////////////////
///  @brief			Optimize one line's height from the top and bottom
///  @param [in]	flag			0 : not optimize; non-0 : optimize the height
///  @param [in]	maxBlank		The max blank dot in height dimension for one line's top and bottom
///  @return		NONE
///  @note			Implement variable height according to the contents of a line
///					e.g.
///					Use 8 dots heights font, call ArScrLineHeightOptimize(1, 1), then:
///					(1)If content is "--------------", then line height will be cutted from 8 to 3;
///					(2)If content is "========", then line height will be cutted from 8 to 6.
///  @see			
///////////////////////////////////////
void ArScrLineHeightOptimize(uchar flag, uchar maxBlank);


///////////////////////////////////////
///  @brief			Set using reverse mode to display or not
///  @param [in]	flag			0 : normal mode ; non-0 : reverse mode
///  @param [in]	fillLine		Used in reverse mode
///									0 : reverse characters only
///									non-0 : reverse the whole line
///  @return		NONE
///  @note			Set fillLine to non-0 when the whole line need to be reverse displayed.
///  @see
///////////////////////////////////////
void ArScrReverse(uchar flag, uchar fillLine);


///////////////////////////////////////
///  @brief			Set the displaying content's align mode.
///  @param [in]	align			AR_ALIGN_RIGHT/AR_ALIGN_CENTER/AR_ALIGN_LEFT
///  @return		NONE
///  @note			Default align mode is AR_ALIGN_RIGHT
///  @see
///////////////////////////////////////
void ArScrAlign(uchar align);


///////////////////////////////////////
///  @brief			Get string width according to current screen font
///  @param [in]	str				The string to be calculated
///  @return		The string width/AR_ERR_STR_TOO_LONG/AR_ERR_FONT_NOT_SELECT
///  @note			No more than 1024 bytes
///  @see			ArPrnGetStrWidth
///////////////////////////////////////
int ArScrGetStrWidth(uchar *str);


// ======================== Screen Methods End ====================


#endif

