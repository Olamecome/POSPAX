
/****************************************************************************
应用标准库
Applib

百富计算机技术(深圳)有限公司
PAX

修改历史：修改内容见<<百富应用标准库使用指南>>
1.0.0 2008年9月12日    标准库维护组
1.0.1 2008年10月14日   标准库维护组
1.0.2 2009年3月11日   修改者:孙云 测试Sabrina

1.2.0 2014.7.21			Modified by Kim.L

****************************************************************************/

#ifndef _APPLIB_H
#define _APPLIB_H

#ifndef uchar
	typedef unsigned char	uchar;
#endif

#ifndef ushort
	typedef unsigned short	ushort;
#endif

#ifndef uint
	typedef unsigned int	uint;
#endif

#ifndef ulong
	typedef unsigned long	ulong;
#endif

#ifndef TRUE
	#define TRUE	1
	#define FALSE	0
#endif

#define TIMER_TEMPORARY			4       // Index of Timer which is for a short time, available for other modules

// macros for debug output
#define DEVICE_SCR   0x01
#define DEVICE_PRN   0x02
#define DEVICE_COM1  0x04
#define DEVICE_COM2  0x08

#define ASC_MODE	0x10
#define HEX_MODE	0x20
#define ISO_MODE	0x40
#define TLV_MODE	0x80

// macros for vDes()
#define ONE_DECRYPT     0           // DES decryption
#define ONE_ENCRYPT     1           // DES encryption
#define TRI_DECRYPT     2           // T-DES decryption (16 bytes key)
#define TRI_ENCRYPT     3           // T-DES encryption (16 bytes key)
#define TRI_DECRYPT3    4           // T-DES decryption (24 bytes key)
#define TRI_ENCRYPT3    5           // T-DES encryption (24 bytes key)

// macros for vCalcMac()
#define MAC_ANSIX99     0           // ANSI9.9 standard MAC algorithm
#define MAC_FAST        1           // HyperCom fast MAC algorithm
#define MAC_ANSIX919	2			// ANSI9.19

// macros for ScrSetIcon()
#ifndef ICON_PHONE
#define ICON_PHONE      1       // phone
#define ICON_SIGNAL     2       // wireless signal
#define ICON_PRINTER    3       // printer
#define ICON_ICCARD     4       // smart card IC
#define ICON_LOCK       5       // lock
#define ICON_SPEAKER    6       // speaker
#define ICON_UP         7       // up
#define ICON_DOWN       8       // down
#define CLOSEICON		0       // hide icon[for all icons]
#define OPENICON        1       // show icon[just for printer/IC card/locker/speaker/UP/DOWN]
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************
 Function:      Calculate the offset of the struct member
 Param In:
    type        struct type
    member      struct member name
 Param Out:
    none
 Return Code:   the offset of the struct member
****************************************************************************/
#define OFFSET(type, member)    ( (ulong)(&((type *)0)->member) )

/****************************************************************************
 Function:      Calculate the maximum(minimum) of two int number/expression
 Param In:
    a,b         number/expression need to be compare
 Param Out:
    none
 Return Code:   maximum(minimum)
****************************************************************************/
#define MAX(a, b)       ( (a)>=(b) ? (a) : (b) )
#define MIN(a, b)       ( (a)<=(b) ? (a) : (b) )

/****************************************************************************
 Function:      Calculate the absolute value of int number/expression
 Param In:
    a           int number/expression
 Param Out:
    none
 Return Code:   absolute value
****************************************************************************/
#define ABS(a)      ( (a)>=0 ? (a) : (-1)*(a) )

/****************************************************************************


                                      1.Base
 
   
	 
*****************************************************************************/

/****************************************************************************
 Function:		XOR
 Param In:
				psVect1, psVect2, uiLength
 Param Out:
				psOut
 Return Code:   none
 Description:
 if 3 input parameters are NULL, then this function will return directly
****************************************************************************/
void PubXor(const uchar *psVect1, const uchar *psVect2, uint uiLength, uchar *psOut);

/****************************************************************************
 Function:		BCD to ASCII
 Param In:
				psIn, uiLength
 Param Out:
				psOut
 Return Code:   none
 Description:
****************************************************************************/
void PubBcd2Asc(const uchar *psIn, uint uiLength, uchar *psOut);

/****************************************************************************
 Function:		BCD to ASCII with '\0'
 Param In:
				psIn, uiLength
 Param Out:
				psOut
 Return Code:   none
 Description:
****************************************************************************/
void PubBcd2Asc0(const uchar *psIn, uint uiLength, uchar *pszOut);

/****************************************************************************
 Function:		ASCII to BCD
 Param In:
				psIn, uiLength
 Param Out:
				psOut
 Return Code:   none
 Description:
 e.g. "12AB"(0x31 0x32 0x3A 0x3B)-->0x12AB(0x12 0xAB)
****************************************************************************/
void PubAsc2Bcd(const uchar *psIn, uint uiLength, uchar *psOut);

/****************************************************************************
 Function:		ASCII to long
 Param In:
				psString, uiLength
 Param Out:
				none
 Return Code:   long value of psString 
 Description:
 Convert number string to long integer, similar to atol(). This function don't request end with '\0'.
 e.g. "1200"(0x31 0x32 0x30 0x30)-->1200
****************************************************************************/
ulong PubAsc2Long(const uchar *psString, uint uiLength);

/****************************************************************************
 Function:		Long to Char(NBO)
 Param In:
				ulSource, uiTCnt
 Param Out:
				psTarget
 Return Code:   none
 Description:
 it is based on NBO(Network Byte Order)
 e.g. 500000-->0x07 0xA1 0x20
****************************************************************************/
void PubLong2Char(ulong ulSource, uint uiTCnt, uchar *psTarget);

/****************************************************************************
 Function:		Char(NBO) to long
 Param In:
				psSource(max:4 bytes), uiSCnt[0-4]
 Param Out:
				none
 Return Code:   long value of psSource(NBO)
 Description:
 e.g. 0x07 0xA1 0x20-->500000
****************************************************************************/
ulong PubChar2Long(const uchar *psSource, uint uiSCnt);

/****************************************************************************
 Function:		long to BCD
 Param In:
				ulSource, uiTCnt
 Param Out:
				psTarget
 Return Code:   none
 Description:
 e.g. 500000-->0x50 0x00 0x00
****************************************************************************/
void PubLong2Bcd(ulong ulSource, uint uiTCnt, uchar *psTarget);

/****************************************************************************
 Function:		BCD to long
 Param In:
				psSource, uiSCnt
 Param Out:
				none
 Return Code:   long value of psSource
 Description:
 e.g. 0x50 0x00 0x00-->500000
****************************************************************************/
ulong PubBcd2Long(const uchar *psSource, uint uiSCnt);

/****************************************************************************
 Function:	    convert to uppercase string
 Param In:		pszStringInOut   with '\0'
 Param Out:		pszStringInOut   with '\0'
 Return Code:  	
				none
 Description:
****************************************************************************/
void PubStrUpper(uchar *pszStringInOut);

/****************************************************************************
 Function:	    convert to lowercase string
 Param In:		pszStringInOut   with '\0'
 Param Out:		pszStringInOut   with '\0'
 Return Code:  	
				none
 Description:
****************************************************************************/
void PubStrLower(uchar *pszStringInOut);

/****************************************************************************
 Function:	    trim the characters on the left or right side, including space(0x20)/CR(0x0D)/new line(\n)/tab(0x09)
 Param In:		pszStringInOut   with '\0'
 Param Out:		pszStringInOut   with '\0'
 Return Code:  	
				none
 Description:
 e.g.	"   123  "-->"123"
****************************************************************************/
void PubTrimStr(uchar *pszStringInOut);

/****************************************************************************
 Function:	    trim the specific character on the left or right side
 Param In:		pszStringInOut   with '\0', ucSpcChar
 Param Out:		pszStringInOut   with '\0'
 Return Code:  	
				none
 Description:
 e.g.	PubTrimSpcStr("   123  ", 0x20)-->"123"
****************************************************************************/
void PubTrimSpcStr(uchar *pszStringInOut, uchar ucSpcChar);

/****************************************************************************
 Function:	    trim the continuous specific characters on the left side
 Param In:		pszStringInOut   with '\0', ucRemoveChar
 Param Out:		pszStringInOut   with '\0'
 Return Code:  	
				none
 Description:
 e.g.	PubTrimHeadChars("000000123  ", 0x30)-->"123  "
****************************************************************************/
void PubTrimHeadChars(uchar *pszStringInOut, uchar ucRemoveChar);

/****************************************************************************
 Function:	    trim the continuous specific characters on the right side
 Param In:		pszStringInOut   with '\0', ucRemoveChar
 Param Out:		pszStringInOut   with '\0'
 Return Code:  	
				none
 Description:
 e.g.	PubTrimTailChars("000123      ", 0x20)-->"000123"
****************************************************************************/
void PubTrimTailChars(uchar *pszStringInOut, uchar ucRemoveChar);

/****************************************************************************
 Function:	    compare two strings without case sensitive (strcasecmp())
 Param In:		pszStr1, pszStr2
 Param Out:		
				none
 Return Code:  	
				0		equivalent
				<0      pszStr1 < pszStr2
				>0      pszStr1 > pszStr2
 Description:
****************************************************************************/
int PubStrNoCaseCmp(const uchar *pszStr1, const uchar *pszStr2);

/****************************************************************************
 Function:	    fill a specific character on the left side to a fix length
 Param In:		pszStringInOut, uiTargetLen, ucAddChar
 Param Out:		pszStringInOut
 Return Code:  	
				none
 Description:
 e.g.	PubTrimTailChars("123", 6, 0x30)-->"000123"
****************************************************************************/
void PubAddHeadChars(uchar *pszStringInOut, uint uiTargetLen, uchar ucAddChar);

/****************************************************************************
 Function:	    reverse the string
 Param In:		pszStringInOut   with '\0', ucRemoveChar
 Param Out:		pszStringInOut   with '\0'
 Return Code:  	
				none
 Description:
 e.g.	"000123"-->"321000"
****************************************************************************/
void PubStrReverse(uchar *pszStringInOut);

/****************************************************************************
 Function:	    get the 4 bits of a character in the high order
 Param In:		ucInChar
 Param Out:		none
 Return Code:  	
				result
 Description:
 e.g.	"3"--> 3
****************************************************************************/
uchar PubHigh4Bit(uchar ucInChar);

/****************************************************************************
 Function:	    get the 4 bits of a character in the low order
 Param In:		ucInChar
 Param Out:		none
 Return Code:  	
				result
 Description:
 e.g.	"3"--> 0
****************************************************************************/
uchar PubLow4Bit(uchar ucInChar);

/****************************************************************************
 Function:	    multiply 2 string numbers
 Param In:		pszFaciend, pszMultiplier
 Param Out:		pszProduct
 Return Code:  	
				none
 Description:
 e.g.	"6" * "2"--> "12"
****************************************************************************/
void PubAscMul(const uchar *pszFaciend, const uchar *pszMultiplier, uchar *pszProduct);

/****************************************************************************
 Function:	    add up 2 string numbers with same length, left-padding:0
 Param In:		psAddend1, psAddend2, uiLen
 Param Out:		pszSum
 Return Code:  	
				none
 Description:
 e.g.	"6" + "2"--> "8"
****************************************************************************/
void PubAscAdd(const uchar *psAddend1, const uchar *psAddend2, uint uiLen, uchar *pszSum);
#define SafeAscAdd(a,b,c)	PubAscAdd(a,b,c,a)

/****************************************************************************
 Function:	    subtract one string number from another string number with same length, 
				left-padding:0
 Param In:		psMinuend, psSubtrahend, uiLen
 Param Out:		pszResult
 Return Code:  	
				none
 Description:
 e.g.	"6" - "2"--> "4"
****************************************************************************/
void PubAscSub(const uchar *psMinuend, const uchar *psSubtrahend, uint uiLen, uchar *pszResult);
#define SafeAscSub(a,b,c)	PubAscSub(a,b,c,a)

/****************************************************************************
 Function:	    decrease 1 from a string number
 Param In:		psAscStrInOut, uiStrLen
 Param Out:		psAscStrInOut
 Return Code:  	
				0 successful
				1 failed
 Description:
 e.g.	PubAscDec("12", 2)-->"11"
****************************************************************************/
uchar PubAscDec(uchar *psAscStrInOut, uint uiStrLen);

/****************************************************************************
 Function:	    count 1 from a string number
 Param In:		psAscStrInOut, uiStrLen
 Param Out:		psAscStrInOut
 Return Code:  	
				0 successful
				1 failed
 Description:
 e.g.	PubAscInc("12", 2)-->"13"
****************************************************************************/
uchar PubAscInc(uchar *psAscStrInOut, uint uiStrLen);

/****************************************************************************
 Function:	    add up 2 BCD numbers with same length, left-padding:0
 Param In:		psAddend1, psAddend2, uiLen
 Param Out:		psResult
 Return Code:  	
				none
 Description:
 e.g.	0x1234 + 0x1234 = 0x2468
****************************************************************************/
void PubBcdAdd(const uchar *psAddend1, const uchar *psAddend2, uint uiLen, uchar *pszSum);
#define SafeBcdAdd(a,b,c)		PubBcdAdd(a, b, c, a)

/****************************************************************************
 Function:	    subtract one BCD number from another BCD number with same length, 
				left-padding:0
 Param In:		psMinuend, psSubtrahend, uiLen
 Param Out:		pszResult
 Return Code:  	
				none
 Description:
 e.g.	0x1234 + 0x1211 = 0x0023
****************************************************************************/
void PubBcdSub(const uchar *psMinuend, const uchar *psSubtrahend, uint uiLen, uchar *psResult);
#define SafeBcdSub(a,b,c)		PubBcdSub(a, b, c, a)

/****************************************************************************
 Function:	    decrease 1 from a string number
 Param In:		psBcdStrInOut, uiStrLen
 Param Out:		psBcdStrInOut
 Return Code:  	
				0 successful
				1 failed
 Description:
 e.g.	PubBcdDec(0x12, 2)-->0x11
****************************************************************************/
uchar PubBcdDec(uchar *psBcdStrInOut, uint uiStrLen);

/****************************************************************************
 Function:	    count 1 from a string number
 Param In:		psBcdStrInOut, uiStrLen
 Param Out:		psBcdStrInOut
 Return Code:  	
				0 successful
				1 failed
 Description:
 e.g.	PubBcdInc(0x12, 2)-->0x13
****************************************************************************/
uchar PubBcdInc(uchar *psBcdStrInOut, uint uiStrLen);

/****************************************************************************
 Function:	    convert a string number with amount format
 Param In:		pszPrefix	:  "HKD" in "HKD 200.00", max:4 characters, enable NULL
				pszIn		:	string number, max: 40 characters
				ucDeciPos	:	position of a decimal dot[0-3]
				ucIgnoreDigit:	should less than string length of pszIn
				ucMisc		:	GA_NEGATIVE:	fill a minus signs on the left of string
 Param Out:		pszOut
 Return Code:  	
				none
 Description:
 e.g.	PubConvAmount("HKD", "12345", 2, 0, pszOut, 0)-->"HKD123.45"
****************************************************************************/
#define GA_NEGATIVE		0x40
void PubConvAmount(const uchar *pszPrefix, const uchar *pszIn, uchar ucDeciPos, uchar ucIgnoreDigit, uchar *pszOut, uchar ucMisc);

/****************************************************************************
 Function:	    interrupt the application if an exception existed
 Param In:		pszfile		:	file name
				uiLine		:	line of sources which catch an exception
 Param Out:		
				none
 Return Code:  	
				none
 Description:
	application will block at this calling.

 promotion:
	File:file name, Line:line numbers
	Ret:system return code
	PLS RECORD THEN RESTART POS
****************************************************************************/
void PubHalt(const uchar *pszfile, uint uiLine);

/****************************************************************************
 Function:      Beep 800ms at frequency 6.
 Param In:      None.
 Param Out:     None
 Return Code:   None
 Description:
****************************************************************************/
void PubLongBeep(void);	

/****************************************************************************
 Function:      Beep 60ms at frequency 6 for 3 times, with 80ms interval.
 Param In:      None.
 Param Out:     None
 Return Code:   None
 Description:
****************************************************************************/
void PubBeepOk(void);		

/****************************************************************************
 Function:      Beep 200ms at frequency 6,then delay 200ms.
 Param In:      None.
 Param Out:     None
 Return Code:   None
 Description:
****************************************************************************/
void PubBeepErr(void);	

/****************************************************************************
 Function:      DES encryption/decryption
 Param In:		ucMode:     
					ONE_ENCRYPT --> DES encryption
					ONE_DECRYPT --> DES decryption
					TRI_ENCRYPT --> T-DES encryption
					TRI_DECRYPT --> T-DES decryption
				psData	:	data(8 bytes)
				psKey	:	key(8 or 16, it is based on ucMode)
 Param Out:
				psResult    result(8 bytes)
 Return Code:   
				none
 Description:
****************************************************************************/
void PubDes(uchar ucMode, const uchar *psData, const uchar *psKey, uchar *psResult);	//

/****************************************************************************
 Function:      calculate MAC
 Param In:		ucMode	:	algorithm
					MAC_FAST    --> fast MAC(HyperCom)
					MAC_ANSIX99 --> ANSI x9.9
					MAC_ANSIX919--> ANSI x9.19
				psKey	:	key(8 bytes)
				psMsg	:	data
				uiLength:   data length
 Param Out:		psMAC   :	MAC(8 bytes)
 Return Code:   
				none
 Description:
****************************************************************************/
void PubCalcMac(uchar ucMode, const uchar *psKey, const uchar *psMsg, uint uiLen, uchar *psMAC);
	
/****************************************************************************
 Function:      calculate MRC
 Param In:		psData, uiLength
 Param Out:		pucInit
 Return Code:   
				none
 Description:
****************************************************************************/
void PubCalcLRC(const uchar *psData, ushort uiLength, uchar *pucInit);

/****************************************************************************
 Function:      check the card no is using Luhn algorithm or not
 Param In:		pszPanNo
 Param Out:		
				none
 Return Code:   
				0	Yes
				1	No
 Description:
****************************************************************************/
uchar PubChkCardNo(const uchar *pszPanNo);


/****************************************************************************


                                      2	Time
 
   
	 
*****************************************************************************/
#define STAR_YEAR 1980
#define STAR_WEEK 6

/****************************************************************************
 Function:      count ulSecond seconds from Jan 1st, 1980, max: Jan 1st, 2116
 Param In:		ulSecond
 Param Out:		
				pszYYYYMMDDhhmmss  (format: YYYYMMDDhhmmss\0)
 Return Code:   
				none
 Description:
****************************************************************************/
void PubLong2Time(ulong ulSecond,uchar *pszYYYYMMDDhhmmss);
 
/****************************************************************************
 Function:      count ulDay days from Jan 1st, 1980, 
 Param In:		ulDay
 Param Out:		
				pszYYYYMMDD  (format: YYYYMMDD\0)
 Return Code:   
				none
 Description:
****************************************************************************/
void PubLong2Day(ulong ulDay,uchar *pszYYYYMMDD);  

/****************************************************************************
 Function:      count seconds from Jan 1st, 1980 to psYYYYMMDDhhmmss
 Param In:		psYYYYMMDDhhmmss[Jan 1st,1980 - Jan 1st, 2116]
 Param Out:		
				none
 Return Code:   
				result
 Description:
****************************************************************************/
ulong PubTime2Long(const uchar *psYYYYMMDDhhmmss);
 
/****************************************************************************
 Function:      count days from Jan 1st, 1980 to psYYYYMMDD
 Param In:		psYYYYMMDD
 Param Out:		
				none
 Return Code:   
				result
 Description:
****************************************************************************/
ulong PubDay2Long(const uchar *psYYYYMMDD);

/****************************************************************************
 Function:      verify if the format of input time is based on Masks
 Param In:		psDateTime, 
				psMaskYYYYMMDDhhmmss:
				"YYYYMMDDhhmmss"	: it must start with "19" or "20"
				"YYMMDDhhmmss"		: it is considered as "20YYMMDDhhmmss"
 Param Out:		
				none
 Return Code:   
				0	:	valid
				1,2,3,4,5,6		:	format of year, month, day, hour, minute, second is invalid
				9	:	format of mask is invalid
 Description:
****************************************************************************/
uchar PubIsValidTime(const uchar *psDateTime, const uchar *psMaskYYYYMMDDhhmmss);

/****************************************************************************
 Function:      count from a specific time using different mode(supports leap year)
 Param In:		psDateTimeIn, 
				ulInterval
				psCalMode	:	"YY","MM","DD","hh","mm","ss" mean unit of counting,
								if psCalMode is invalid or NULL, it will be set as "ss"
 Param Out:		
				pszDateTimeOut
 Return Code:   
				none
 Description:
****************************************************************************/
void PubCalDateTime(const uchar *psDateTimeIn, uchar *pszDateTimeOut, ulong ulInterval, const uchar *psCalMode);

/****************************************************************************
 Function:      calculate an interval between 2 Points in time

 Param In:		psTimeBegin (format: YYYYMMDDhhmmss)
				psTimeEnd	(format: YYYYMMDDhhmmss)
				psCalMode	:	"YY","MM","DD","hh","mm","ss"
								if psCalMode is invalid or NULL, it will be set as "ss"
 Param Out:		
				pucSignal	:	1:negative	0:positive
 Return Code:   
				interval
 Description:
****************************************************************************/ 
ulong PubCalInterval(const uchar *psTimeBegin, const uchar *psTimeEnd, const uchar *psCalMode, uchar *pucSignal);

/****************************************************************************


                                      3	input
 
  
	 
*****************************************************************************/


/****************************************************************************
 Function:      wait for pressing any key

 Param In:		iWaitTime	:	>=0	activate a timer (it will return NOKEY)
								< 0 keep waiting for pressing (it will not return NOKEY)
 Param Out:		
				none
 Return Code:   
				key value
				NOKEY(time out)
 Description:
****************************************************************************/ 
uchar PubWaitKey(short iWaitTime) ;


/****************************************************************************


                                      4	File
 
   
	 
*****************************************************************************/

/****************************************************************************
 Function:      Read data from file. Should use Seek() to set write pointer first.
 Param In:		fd          file handle to be read from.
				psData      data pointer to place data read
				iNeedByte   number of bytes to be read out.
 Param Out:
				none
 Return Code:
				total read out bytes.
 Description:
****************************************************************************/
int PubFReadN(int fd, void *psData, int iNeedBytes);

/****************************************************************************
 Function:      Read data from file
 Param In:		pszFileName	:	file name
				lOffset		:	offset of file
				iLen		:	number of bytes to be read out.
 Param Out:
				psData
 Return Code:
				0	successful
				1	failed to open file
				2	failed to seek to the offset address
				3	failed to read data
 Description:
****************************************************************************/
uchar PubFileRead(const uchar *pszFileName, long lOffset, void *psData, int iLen);

/****************************************************************************
 Function:      Write data into file. Should use Seek() to set write pointer first.
 Param In:		fd          file handle to be written into
				psData      pointer of data source to write into file
				iNeedBytes  number of bytes to write
 Param Out:
				none
 Return Code:
				total write bytes.
****************************************************************************/
int PubFWriteN(int fd, const void *psData, int iNeedBytes);

/****************************************************************************
 Function:      Write data from file
 Param In:		pszFileName	:	file name
				lOffset		:	offset of file
				iLen		:	number of bytes to be write in
				psData		:	data
 Param Out:
				none
 Return Code:
				0	successful
				1	failed to open file
				2	failed to seek to the offset address
				3	failed to write data
 Description:
****************************************************************************/
uchar PubFileWrite(const uchar *pszFileName, long lOffset, const void *psData, int iLen);




/****************************************************************************


                                      5	Debug Output
 
   
	 
*****************************************************************************/

/****************************************************************************
 Function:		debug output
 Param In:		pszTitle	:	title, limit in 32 characters, can be NULL.
				psData		:	data
				usDataLen	:	data length
				ucDevice	:	
						output device:
							screen, DEVICE_SCR(0x01)
							printer,DEVICE_PRN(0x02)
							COM1,	DEVICE_COM1(0x04)
							COM2,	DEVICE_COM2(0x08)
						ucMode		data format:
							ASCII,	ASC_MODE(0x10) 
							HEX,	HEX_MODE(0x20) 
							ISO8583,ISO_MODE(0x40) 
							TLV,	TLV_MODE(0x80) 
 Param Out:
				none
 Return Code:
				0		successful
				0xFF	failed
 Description:
****************************************************************************/
uchar PubDebugOutput(const uchar *pszTitle, const uchar *psData, ushort nDataLen, uchar ucDevice, uchar ucMode);

/****************************************************************************
 Function:		setting for debug output, mode ISO8583, definition of the package structure
 Param In:		pvDataDef	:	package structure
 Param Out:
				none
 Return Code:
				none
 Description:
****************************************************************************/
void PubISODefine(const void *pvDataDef);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* _UTIL_H */
