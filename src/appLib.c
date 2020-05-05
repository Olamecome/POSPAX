
#include "global.h"

/********************** Internal macros declaration ************************/
#define TIMER_TEMPORARY         4   // Temporary timer(Shared by different modules)

#ifdef _POS_DEBUG
#define LEN_DBGDATA         1024    // Maximum debug data length
#define MAX_CHARS           5       // Max characters per line in debug usage
#endif /* _POS_DEBUG */

#define LEN_GETSTRING		512	// for PubGetString() use

/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/

static uchar IsLeapYear(unsigned long ulYear);

static int UnPackElement(const FIELD_ATTR *pAttr, const uchar *pusIn, uchar *pusOut,uint *puiInLen);
static uchar ScrSend(const uchar *psData, ushort nDataLen, ushort nStartLine);
static uchar PortSendstr(uchar ucChannel, const uchar *psStr, ushort nStrLen);
static uchar UnitSend(uchar *psDataInOut, ushort nDataLen, uchar ucDevice, uchar ucMode, ushort nStartLine);

/********************** Internal variables declaration *********************/

const uint uiConstMonthday[12]=
{
	0,		
	31,  //1 
	31+28,  //2 
	31+28+31,  //3 
	31+28+31+30,  //4 
	31+28+31+30+31,  //5 
	31+28+31+30+31+30,  //6 
	31+28+31+30+31+30+31,  //7 
	31+28+31+30+31+30+31+31,  //8 
	31+28+31+30+31+30+31+31+30,  //9 
	31+28+31+30+31+30+31+31+30+31,  //10
	31+28+31+30+31+30+31+31+30+31+30,  //11
};     

// ISO8583 field attribute, for debug use only.
static FIELD_ATTR DebugDefaulDef[] =
{
	{Attr_n, Attr_fix,  4},					// message code(MTI)
	{Attr_b, Attr_fix,  8},					// 1 - bitmap
	{Attr_n, Attr_var1, 19},				// 2 - PAN
	{Attr_n, Attr_fix,  6},					// 3 - process code
	{Attr_n, Attr_fix,  12},				// 4 - txn amount
	{Attr_n, Attr_fix,  12},				// 5 - Foreign Amt(DCC)
	{Attr_UnUsed, Attr_fix, 0},				// 6 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 7 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 8 - not used
	{Attr_a, Attr_fix, 8},					// 9 - DCC rate
	{Attr_UnUsed, Attr_fix, 0},				// 10 - not used
	{Attr_n, Attr_fix, 6},					// 11 - STAN
	{Attr_n, Attr_fix, 6},					// 12 - local time
	{Attr_n, Attr_fix, 4},					// 13 - local date
	{Attr_n, Attr_fix, 4},					// 14 - expire
	{Attr_UnUsed, Attr_fix, 0},				// 15 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 16 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 17 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 18 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 19 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 20 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 21 - not used
	{Attr_n, Attr_fix, 4},					// 22 - entry modes
	{Attr_n, Attr_fix, 3},					// 23 - PAN Seq #
	{Attr_n, Attr_fix, 3},					// 24 - NII
	{Attr_n, Attr_fix, 2},					// 25 - condition code
	{Attr_UnUsed, Attr_fix, 0},				// 26 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 27 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 28 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 29 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 30 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 31 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 32 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 33 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 34 - not used
	{Attr_z, Attr_var1, 37},				// 35 - track 2
	{Attr_z, Attr_var2, 104},				// 36 - track 3
	{Attr_a, Attr_fix,  12},				// 37 - RRN
	{Attr_a, Attr_fix,  6},					// 38 - auth. code
	{Attr_a, Attr_fix,  2},					// 39 - response code
	{Attr_UnUsed, Attr_fix, 0},				// 40 - not uesed
	{Attr_a, Attr_fix, 8},					// 41 - TID
	{Attr_a, Attr_fix, 15},					// 42 - MID
	{Attr_UnUsed, Attr_fix, 0},				// 43 - not used
	{Attr_a, Attr_var1, 2},					// 44 - Add'l rsp data
	{Attr_a, Attr_var1, 76+1},				// 45 - track 1
	{Attr_UnUsed, Attr_fix, 0},				// 46 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 47 - not used
	{Attr_b, Attr_var2, 100},				// 48 - add'l req data
	{Attr_a, Attr_fix,  3},					// 49 - FRN Curcy code
	{Attr_a, Attr_fix,  3},					// 50 - Currency code
	{Attr_UnUsed, Attr_fix, 0},				// 51 - not used
	{Attr_b, Attr_fix, 8},					// 52 - PIN data
	{Attr_UnUsed, Attr_fix, 0},				// 53 - not used
	{Attr_a, Attr_var2, 12},				// 54 - Extra Amount
	{Attr_b, Attr_var2, 260},				// 55 - ICC data
	{Attr_b, Attr_var2, 110},				// 56 - ICC data 2(HK)
	{Attr_UnUsed, Attr_fix, 0},				// 57 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 58 - not used
	{Attr_UnUsed, Attr_fix, 0},				// 59 - not used
	{Attr_a, Attr_var2, 999},				// 60 - Private used
	{Attr_a, Attr_var2, 999},				// 61 - POS SN/desc code
	{Attr_b, Attr_var2, 999},				// 62 - ROC/SOC
	{Attr_b, Attr_var2, 999},				// 63 - Private used
	{Attr_b, Attr_fix,  8},					// 64 - MAC(not used)
	{Attr_Over,   Attr_fix, 0}
};

FIELD_ATTR *pstDataAttr=NULL;

// Support right-to-left language
// The below definition is basically same as in Farsi.h
#define     APPLIB_SHIFT_RIGHT             0x00
#define     APPLIB_SHIFT_CENTER            0x01
#define     APPLIB_SHIFT_LEFT              0x02
#define     APPLIB_EFT_REVERSE             0x80

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/


//Source string XOR object string, then store the result in object string.(The two string are same length)
void PubXor(const uchar *psVect1, const uchar *psVect2, uint uiLength, uchar *psOut)
{
    uint   uiCnt;

	if ((psVect1 == NULL) || (psVect2 == NULL) || (psOut == NULL))
	{
		return;
	}

	memcpy(psOut, psVect1, uiLength); 
	
    for(uiCnt = 0; uiCnt < uiLength; uiCnt++)
    {
        psOut[uiCnt] ^= psVect2[uiCnt];
    }
}

//Convert BIN string to readable HEX string, which have double length of BIN string. 0x12AB-->"12AB"
void PubBcd2Asc(const uchar *psIn, uint uiLength, uchar *psOut)
{
    static const uchar ucHexToChar[16] = {"0123456789ABCDEF"};
    uint   uiCnt;
	
	if ((psIn == NULL) || (psOut == NULL))
	{
		return;
	}

    for(uiCnt = 0; uiCnt < uiLength; uiCnt++)
    {
        psOut[2*uiCnt]     = ucHexToChar[(psIn[uiCnt] >> 4)];
        psOut[2*uiCnt + 1] = ucHexToChar[(psIn[uiCnt] & 0x0F)];
    }
}

//Similar with function PubOne2Two(), and add '\0' at the end of object string
void PubBcd2Asc0(const uchar *psIn, uint uiLength, uchar *pszOut)
{
	if ((psIn == NULL) || (pszOut == NULL))
	{
		return;
	}
	
	PubBcd2Asc(psIn, uiLength, pszOut);
    pszOut[2*uiLength] = 0;
}

//Convert readable HEX string to BIN string, which only half length of HEX string. "12AB"-->0x12AB
void PubAsc2Bcd(const uchar *psIn, uint uiLength, uchar *psOut)
{
    uchar   tmp;
    uint    i;
	
  	if ((psIn == NULL) || (psOut == NULL))
	{
		return;
	}
	
	for(i = 0; i < uiLength; i += 2)
    {
        tmp = psIn[i];
        if( tmp > '9' )
        {
            tmp = (uchar)toupper((int)tmp) - 'A' + 0x0A;
        }
        else
        {
            tmp &= 0x0F;
        }
        psOut[i / 2] = (tmp << 4);
		
        tmp = psIn[i+1];
        if( tmp > '9' )
        {
            tmp = toupper((char)tmp) - 'A' + 0x0A;
        }else
        {
            tmp &= 0x0F;
        }
        psOut[i/2] |= tmp;
    }
}

//Convert number string to long integer, similar to atol(). This function don't request end with '\0'.
ulong PubAsc2Long(const uchar *psString, uint uiLength)
{
    uchar    szBuff[15+1];
    sprintf((char *)szBuff, "%.*s", uiLength <= 15 ? uiLength : 15, psString);
    return (ulong)atol((char*)szBuff);
}

//Convert integer to string which high bit at the front. (Store it according to network byte format)
void PubLong2Char(ulong ulSource, uint uiTCnt, uchar *psTarget)
{
    uint    i;
	
    for(i = 0; i < uiTCnt; i++)
    {
        psTarget[i] = (uchar)(ulSource >> (8 * (uiTCnt - i - 1)));
    }
}

//Convert the character string with high bit in the front to integer
ulong PubChar2Long(const uchar *psSource, uint uiSCnt)
{
    uint    i;
	ulong ulTmp;
	
   	if (psSource == NULL)
	{
		return 0;
	}
	
	ulTmp = 0L;
    for(i = 0; i < uiSCnt; i++)
    {
        ulTmp |= ((ulong)psSource[i] << 8 * (uiSCnt - i - 1));
    }
	
    return ulTmp;
}

//Converted integer to BCD string,1234 --> 0x12 0x34
void PubLong2Bcd(ulong ulSource, uint uiTCnt, uchar *psTarget)
{
    uchar    szFmt[30], szBuf[30];
	
    sprintf((char*)szFmt, "%%0%dlu", uiTCnt*2);
    sprintf((char*)szBuf, (char *)szFmt, ulSource);
    if( psTarget != NULL )
    {
        PubAsc2Bcd(szBuf, uiTCnt * 2, psTarget);
    }
}

//Convert BCD string to integer
ulong PubBcd2Long(const uchar *psSource, uint uiSCnt)
{
    uchar   szBuf[30];
 	
   	if ((psSource == NULL))
	{
		return 0;
	}
	
	PubBcd2Asc0(psSource, uiSCnt, szBuf);
    return (ulong)atol((char *)szBuf);
}

//Convert the character string to capital
void PubStrUpper(uchar *pszStringInOut)
{
   	if ((pszStringInOut == NULL))
	{
		return;
	}
	
    while( *pszStringInOut )
    {
        *pszStringInOut = toupper((char)*pszStringInOut);
        pszStringInOut++;
    }
}

//Convert the character string to lowercase
void PubStrLower(uchar *pszStringInOut)
{
   	if ((pszStringInOut == NULL))
	{
		return;
	}
	
    while( *pszStringInOut )
    {
        *pszStringInOut = (uchar)tolower((int)*pszStringInOut);
        pszStringInOut++;
    }
}

//Delete the characters include blank,enter,newline,TAB in the string
void PubTrimStr(uchar *pszStringInOut)
{
#define ISSPACE(ch) ( ((ch) == ' ')  || ((ch) == '\t') || \
	((ch) == '\n') || ((ch) == '\r') )
	
    uchar *p, *q;
	
    if( !pszStringInOut || !*pszStringInOut )
    {
        return;
    }
	
    p = &pszStringInOut[strlen((char*)pszStringInOut) - 1];
    while(( p > pszStringInOut) && ISSPACE(*p) )
    {
        *p-- = 0;
    }
    if( (p == pszStringInOut) && ISSPACE(*p) )  *p = 0;
	
    for(p = pszStringInOut; *p && ISSPACE(*p); p++);
    if( p != pszStringInOut )
    {
        q = pszStringInOut;
        while( *p )   *q++ = *p++;
        *q = 0;
    }
#undef ISSPACE
}

//Delete the specified character in string
void PubTrimSpcStr(uchar *pszStringInOut, uchar ucSpcChar)
{
	uchar *p, *q;
	
	if( !pszStringInOut || !*pszStringInOut )
	{
		return;
	}
	
	p = &pszStringInOut[strlen((char*)pszStringInOut) - 1];
	while( (p > pszStringInOut) && ((*p) == ucSpcChar))
	{
		*p-- = 0;
	}
	if( p == pszStringInOut && ((*p) == ucSpcChar) )  *p = 0;
	
	for(p = pszStringInOut; *p && ((*p) == ucSpcChar); p++);
	if( p != pszStringInOut )
	{
		q = pszStringInOut;
		while( *p )   *q++ = *p++;
		*q = 0;
	}
}

//Delete the specified continuous characters on the right of the string
void PubTrimTailChars(uchar *pszStringInOut, uchar ucRemoveChar)
{
	int		i, iLen;
	
	if( !pszStringInOut || !*pszStringInOut )
    {
        return;
    }
	
	iLen = strlen((char *)pszStringInOut);
	for(i=iLen-1; i>=0; i--)
	{
		if( pszStringInOut[i]!=ucRemoveChar )
		{
			break;
		}
		pszStringInOut[i] = 0;
	}
}

// delete the specified continuous characters on the left of the string
void PubTrimHeadChars(uchar *pszStringInOut, uchar ucRemoveChar)
{
	uchar	*p;
	
	if( !pszStringInOut || !*pszStringInOut )
    {
        return;
    }
	
	for(p=pszStringInOut; *p && *p==ucRemoveChar; p++);
	if( p!=pszStringInOut )
	{
		while( (*pszStringInOut++ = *p++) );
	}
}

// Pad leading charactes till specific length.
void PubAddHeadChars( uchar *pszStringInOut, uint uiTargetLen, uchar ucAddChar )
{
	uint	uiLen;

	if (pszStringInOut == NULL)
	{
		return;
	}
	
	uiLen = (uint)strlen((char *)pszStringInOut);
	if( uiLen>=uiTargetLen )
	{
		return;
	}
	
	memmove(pszStringInOut+uiTargetLen-uiLen, pszStringInOut, uiLen+1);
	memset(pszStringInOut, ucAddChar, uiTargetLen-uiLen);
}

//Compare two strings, non case-sensitive.
int PubStrNoCaseCmp(const uchar *pszStr1, const uchar *pszStr2)
{
    if((pszStr1 == NULL) && (pszStr2 == NULL))
    {
        return 0;
    }

	if((pszStr1 == NULL) && (pszStr2 != NULL))
	{
		return -1;
	}

	if((pszStr1 != NULL) && (pszStr2 == NULL))
	{
		return 1;
	}
	
    while( *pszStr1 && *pszStr2 )
    {
        if( toupper((char)*pszStr1) != toupper((char)*pszStr2) )
        {
            return (toupper((char)*pszStr1) - toupper((char)*pszStr2));
        }
        pszStr1++;
        pszStr2++;
    }
    if( !*pszStr1 && !*pszStr2 )
    {
        return 0;
    }
    if( !*pszStr1 )
    {
        return -1;
    }
	
    return 1;
}

//reverse the string
void PubStrReverse(uchar *pszStringInOut)
{
    int     i, j, iLength;
    uchar    ucTmp;
	
	if (pszStringInOut == NULL)
	{
		return;
	}

    iLength = strlen((char*)pszStringInOut);
    for(i = 0,j = iLength - 1; i < iLength / 2; i++,j--)
    {
        ucTmp        = pszStringInOut[i];
        pszStringInOut[i] = pszStringInOut[j];
        pszStringInOut[j] = ucTmp;
    }
}

//get the high 4 bit of the byte
uchar PubHigh4Bit(uchar ucInChar)
{
	return (ucInChar / 16);
}

//get the low 4 bit of the byte
uchar PubLow4Bit(uchar ucInChar)
{
	return (ucInChar & 0x0F);
}

//multiply one ASC string by another
void PubAscMul(const uchar *pszFaciend, const uchar *pszMultiplier, uchar *pszProduct)
{
	uchar	*p, ucTemp, ucCarryBit, szBuff[100+1];
	uint	uiFaciLen, uiMulLen, uiProdPos, uiCnt;
	
	if ((pszFaciend == NULL) || (pszMultiplier == NULL) || (pszProduct == NULL))
	{
		return;
	}
	
	uiFaciLen = strlen((char *)pszFaciend);
	uiMulLen  = strlen((char *)pszMultiplier);
	
	PubASSERT( uiFaciLen+uiMulLen<=100 );
	sprintf((char *)szBuff, "%0*ld", uiFaciLen+uiMulLen, 0L);
	
	for(uiProdPos=0; uiFaciLen>0; uiFaciLen--,uiProdPos++)
	{
		ucCarryBit = 0;
		p = &szBuff[uiProdPos];
		
		for(uiCnt=uiMulLen; uiCnt>0; uiCnt--)
		{
			ucTemp = (pszFaciend[uiFaciLen-1] & 0x0F) * (pszMultiplier[uiCnt-1] & 0x0F) +
				ucCarryBit + (*p & 0x0F);
			*p++   = (ucTemp % 10) + '0';
			ucCarryBit = ucTemp / 10;
		}
		if( ucCarryBit!=0 )
		{
			*p++ = ucCarryBit + '0';
		}
	}
	PubTrimTailChars(szBuff, '0');
	PubStrReverse(szBuff);
	
	if( szBuff[0]==0 )
	{
		sprintf((char *)szBuff, "0");
	}
	if( pszProduct!=NULL )
	{
		sprintf((char *)pszProduct, "%s", szBuff);
	}
}

// Addition of two equal length ASCII string(length specified, no need '\0' ending)
void PubAscAdd(const uchar *psAddend1, const uchar *psAddend2, uint uiLen, uchar *pszSum)
{
	uchar	*pszResult, ucCarryBit, ucTemp, szBuff[100+1];
	
	if ((psAddend1 == NULL) || (psAddend2 == NULL) || (pszSum == NULL) || (uiLen > 100))
	{
		return;
	}

	ucCarryBit = 0;
	pszResult  = szBuff;
	while( uiLen>0 )
	{
		ucTemp = (psAddend1[uiLen-1] & 0x0F) + (psAddend2[uiLen-1] & 0x0F) + ucCarryBit;
		*pszResult++ = (ucTemp % 10) + '0';
		ucCarryBit   = (ucTemp>9) ? 1 : 0;
		uiLen--;
	}
	if( ucCarryBit!=0 )
	{
		*pszResult++ = '1';
	}
	*pszResult = 0;
	
	PubStrReverse(szBuff);
	if( pszSum!=NULL )
	{
		sprintf((char *)pszSum, "%s", szBuff);
	}
}

// Subtraction of two equal length ASCII string(length specified, no need '\0' ending)
void PubAscSub(const uchar *psMinuend, const uchar *psSubtrahend, uint uiLen, uchar *pszResult)
{
	uchar	*pszOut, ucCarryBit, ucTemp, szBuff[100+1];
	
	if ((psMinuend == NULL) || (psSubtrahend == NULL) || (pszResult == NULL) || (uiLen > 100))
	{
		return;
	}

	pszOut     = szBuff;
	ucCarryBit = 0;
	while( uiLen>0 )
	{
		ucTemp = (psMinuend[uiLen-1] & 0x0F) - (psSubtrahend[uiLen-1] & 0x0F) - ucCarryBit + 10;
		*pszOut++  = (ucTemp % 10) + '0';
		ucCarryBit = (psMinuend[uiLen-1]<psSubtrahend[uiLen-1]+ucCarryBit) ? 1 : 0;
		uiLen--;
	}
	*pszOut = 0;
	
	PubStrReverse(szBuff);
	if( pszResult!=NULL )
	{
		sprintf((char *)pszResult, "%s", szBuff);
	}
}

//subtract 1 from the ASC string, the result store in that ASC string
uchar PubAscDec(uchar *psAscStrInOut, uint uiStrLen)
{
	if ((psAscStrInOut == NULL) || (uiStrLen == 0))
	{
		return 1;
	}
	
	psAscStrInOut += uiStrLen - 1;
	while(--(*psAscStrInOut) < '0')
	{
		*psAscStrInOut-- = '9';
		if(--uiStrLen == 0) return 1;
	}
	return 0;
}

//Add 1 to the ASC string, the result store in that ASC string
uchar PubAscInc(uchar *psAscStrInOut, uint uiStrLen)
{
	if ((psAscStrInOut == NULL) || (uiStrLen == 0))
	{
		return 1;
	}

	psAscStrInOut += uiStrLen - 1;
	while(++(*psAscStrInOut) > '9')
	{
		*psAscStrInOut-- = '0';
		if(--uiStrLen == 0) return 1;
	}
	
	return 0;
}

//Addition of 2 equal length BCD string. length specified and no need '\0' ending. fill leading 0 if necessary.
void PubBcdAdd(const uchar *psAddend1, const uchar *psAddend2, uint uiLen, uchar *pszSum)
{
	uchar	sAdd1[100+1], sAdd2[100];
	
	if ((psAddend1 == NULL) || (psAddend2 == NULL) || (pszSum == NULL) || (uiLen > 50))
	{
		return;
	}

	PubASSERT( uiLen<=50 );
	PubBcd2Asc(psAddend1, uiLen, sAdd1);
	PubBcd2Asc(psAddend2, uiLen, sAdd2);
	PubAscAdd(sAdd1, sAdd2, uiLen*2, sAdd1);
	PubAsc2Bcd(sAdd1, uiLen*2, pszSum);
}

//subtraction of 2 equal length BCD string. length specified and no need '\0' ending. fill leading 0 if necessary.
void PubBcdSub(const uchar *psMinuend, const uchar *psSubtrahend, uint uiLen, uchar *psResult)
{
	uchar	sMinuend[100+1], sSubtrahend[100];
	
	if ((psMinuend == NULL) || (psSubtrahend == NULL) || (psResult == NULL) || (uiLen > 50))
	{
		return;
	}

	PubASSERT( uiLen<=50 );
	PubBcd2Asc(psMinuend, uiLen, sMinuend);
	PubBcd2Asc(psSubtrahend, uiLen, sSubtrahend);
	PubAscSub(sMinuend, sSubtrahend, uiLen*2, sMinuend);
	PubAsc2Bcd(sMinuend, uiLen*2, psResult);
}

//subtracted from the BCD string, the result store in that BCD string
uchar PubBcdDec(uchar *psBcdStrInOut, uint uiStrLen)
{
	if ((psBcdStrInOut == NULL) || (uiStrLen == 0))
	{
		return 1;
	}

	psBcdStrInOut += uiStrLen - 1;
	while(--(*psBcdStrInOut) == 0xff)
	{
		*psBcdStrInOut = 0x99;
		psBcdStrInOut--;
		if(--uiStrLen == 0) return 1;
	}
	
	if(((*psBcdStrInOut) & 0x0f) > 9) *psBcdStrInOut -= 0x06;
	return 0;
}

//Add 1 to the BCD string, the result store in that BCD string
uchar PubBcdInc(uchar *psBcdStrInOut, uint uiStrLen)
{
	if ((psBcdStrInOut == NULL) || (uiStrLen == 0))
	{
		return 1;
	}

	psBcdStrInOut += uiStrLen - 1;
	while(++(*psBcdStrInOut) > 0x99)
	{
		*psBcdStrInOut = 0;
		psBcdStrInOut--;
		if((--uiStrLen) == 0) return 1;
	}
	
	if(((*psBcdStrInOut) & 0x0f) > 9) *psBcdStrInOut += 0x06;
	return 0;
}


/*********************************************************************************************/
/*********************************************************************************************/

// Modified by Kim_LinHB 2014-6-17 v1.01.0000
// convert a string number with amount format
void PubConvAmount(const uchar *pszPrefix, const uchar *pszIn, uchar ucDeciPos, uchar ucIgnoreDigit, uchar *pszOut, uchar ucMisc)
{
	int		ii, iInLen, iIntegerLen;
	uchar	bNegative, szBuff[40+1], szTemp[40];
	uchar	*pRead, *pWr;
	
	PubASSERT(pszIn!=NULL && pszOut!=NULL && ucDeciPos<4);

	if ((pszIn == NULL) || (pszOut == NULL) || (strlen((char*)pszIn) > 40))
	{
		return;
	}
	
	sprintf((char *)szBuff, "%.40s", pszIn);
	PubTrimHeadChars(szBuff, '0');
	iInLen = strlen((char *)szBuff);			//get input amount length
	if (ucIgnoreDigit && iInLen>ucIgnoreDigit)	// trim ignored digits.
	{
		iInLen -= ucIgnoreDigit;
		szBuff[iInLen] = 0;
	}
	pRead  = szBuff;
	pWr = pszOut;
	
	bNegative  = ucMisc & GA_NEGATIVE;		//whether display negative amount
	
	if (pszPrefix!=NULL && *pszPrefix!=0)
	{
		pWr += sprintf((char *)pWr, "%.3s ", pszPrefix);
		//		pWr += MIN(strlen((char *)pszPrefix), 4);
	}
	if (bNegative)
	{
		*(pWr++) = '-';
	}
	
	// before decimal point
	if (iInLen>ucDeciPos)	// if value>1
	{
		iIntegerLen = iInLen - ucDeciPos;
		ii = iIntegerLen;
		while (ii--)
		{
			*(pWr++) = *(pRead++);
			if ((ii%3==0) && ii)
			{
				*(pWr++) = ',';
			}
		}
	}
	else
	{
		*(pWr++) = '0';
	}
	
	// decimal point and afterwards
	if (ucDeciPos!=0)
	{
		sprintf((char *)szTemp, "%.*s", ucDeciPos, pRead);
		PubAddHeadChars(szTemp, ucDeciPos, '0');
		sprintf((char *)pWr, ".%s", szTemp);
		//		sprintf((char *)pWr, ".%0*s", ucDeciPos, pRead);
		pWr += (ucDeciPos+1);
	}
	else
	{
		*(pWr++) = 0;
	}
	
	// Process suffix (if any)
	//...
}

// Wait key within specific timeout. no time limit if iWaitTime<0
uchar PubWaitKey(short iWaitTime)
{
	uchar   ucKey, bChkTimer;

	if (iWaitTime>=0)
	{
		bChkTimer = TRUE;
		TimerSet(TIMER_TEMPORARY, (short)(iWaitTime*10));
	}
	else
	{
		bChkTimer = FALSE;
	}

	while (1)
	{
		if (0 == kbhit())
		{
			ucKey = getkey();
			break;
		}
		else if (bChkTimer && TimerCheck(TIMER_TEMPORARY)==0)
		{
			ucKey = NOKEY;
			break;
		}
	}   // end of while (1)

	return ucKey;
}

// check the card no is using Luhn algorithm or not
uchar PubChkCardNo(const uchar *pszPanNo)
{
	uchar	bFlag, ucTemp, ucResult;
	uchar	*pszTemp;
	
	if ((pszPanNo == NULL) || (strlen((char*)pszPanNo) == 0))
	{
		return 1;
	}

	// (2121 algorithm)
	bFlag    = FALSE;
	pszTemp  = (uchar *)&pszPanNo[strlen((char *)pszPanNo)-1];
	ucResult = 0;
	while( pszTemp>=pszPanNo )
	{
		ucTemp = (*pszTemp--) & 0x0F;
		if( bFlag )    ucTemp *= 2;
		if( ucTemp>9 ) ucTemp -= 9;
		ucResult = (ucTemp + ucResult) % 10;
		bFlag = !bFlag;
	}
	
	if( ucResult!=0 )
	{
		return 1;
	}
	
	return 0;
}

////////////////////////////////////// Time calculation //////////////////////////////////////

//verify if the format of input time is based on Masks
uchar PubIsValidTime(const uchar *psDateTime, const uchar *psMaskYYYYMMDDhhmmss)
{
#define ERROR_MASK_DATATIME  9
	
	const uchar ucMonthDay[12]={31,29,31,30,31,30,31,31,30,31,30,31};
	uint  uiYear=0, uiMonth=0, uiDay=0, uiHour=0, uiMinute=0, uiSecond=0;
	uchar ucOffset=0;
	
	if (psMaskYYYYMMDDhhmmss==NULL)
	{
		return ERROR_MASK_DATATIME;							//Mask error
	}

	if (strlen(psMaskYYYYMMDDhhmmss)==0 || strlen(psMaskYYYYMMDDhhmmss)>14 || (strlen(psMaskYYYYMMDDhhmmss)%2)==1)
	{
		return ERROR_MASK_DATATIME;							//Mask error
	}
	
	//processing Year
	if (memcmp(psMaskYYYYMMDDhhmmss,"YYYY",4)==0)
	{
		uiYear = PubAsc2Long(psDateTime,4);
		ucOffset+=4; 
		if (uiYear<1900 || uiYear>2099)
		{
			return 1;										//Year error
		}
	}
	else if (memcmp(psMaskYYYYMMDDhhmmss,"YY",2)==0)
	{
		uiYear = PubAsc2Long(psDateTime,2);
		uiYear += 2000;
		ucOffset += 2; 
	}
	
	//processing Month
	if (psMaskYYYYMMDDhhmmss[ucOffset] == 0) return 0;		//Finished and return OK
	if (uiYear !=0)
	{
		if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"MM",2) !=0) return ERROR_MASK_DATATIME;	//Mask error
	}
	if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"MM",2) ==0)
	{
		uiMonth = PubAsc2Long(psDateTime+ucOffset,2);
		ucOffset += 2;
		if (uiMonth==0 || uiMonth>12)
		{
			return 2;										//Month error
		}
	}
	
	//processing Day
	if (psMaskYYYYMMDDhhmmss[ucOffset] == 0) return 0;		//Finished and return OK
	if (uiMonth !=0)
	{
		if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"DD",2) !=0) return ERROR_MASK_DATATIME;	//Mask error
	}
	if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"DD",2) ==0)
	{
		uiDay = PubAsc2Long(psDateTime+ucOffset,2);
		ucOffset += 2;
		if (uiDay==0 || uiDay>31)
		{
			return 3;										//Day error
		}
		if (uiMonth != 0)
		{
			if (uiDay > ucMonthDay[uiMonth-1])
			{
				return 3;										//Day error
			}
			if (uiYear!=0 && uiMonth==2 && uiDay==29)
			{
				if (IsLeapYear(uiYear)==0)
				{
					return 3;										//Day error
				}
			}
		}
	}
	
	//processing Hour
	if (psMaskYYYYMMDDhhmmss[ucOffset] == 0) return 0;		//Finished and return OK
	if (uiDay !=0)
	{
		if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"hh",2) !=0) return ERROR_MASK_DATATIME;	//Mask error
	}
	if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"hh",2) ==0)
	{
		uiHour = PubAsc2Long(psDateTime+ucOffset,2);
		ucOffset += 2;
		if (uiHour>23)
		{
			return 4;										//Hour error
		}
		uiHour++;		//for not equal 0
	}
	
	//processing Minite
	if (psMaskYYYYMMDDhhmmss[ucOffset] == 0) return 0;		//Finished and return OK
	if (uiHour !=0)
	{
		if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"mm",2) !=0) return ERROR_MASK_DATATIME;	//Mask error
	}
	if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"mm",2) ==0)
	{
		uiMinute = PubAsc2Long(psDateTime+ucOffset,2);
		ucOffset += 2;
		if (uiMinute>59)
		{
			return 5;										//Minute error
		}
		uiMinute++;		//for not equal 0
	}
	
	//processing Second
	if (psMaskYYYYMMDDhhmmss[ucOffset] == 0) return 0;		//Finished and return OK
	if (uiMinute !=0)
	{
		if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"ss",2) !=0) return ERROR_MASK_DATATIME;	//Mask error
	}
	if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"ss",2) ==0)
	{
		uiSecond = PubAsc2Long(psDateTime+ucOffset,2);
		ucOffset += 2;
		if (uiSecond>59)
		{
			return 6;										//Minute error
		}
		uiSecond++;		//for not equal 0
	}
 
	if (ucOffset==0)
	{
		return ERROR_MASK_DATATIME;		//Mask error
	}
	return 0;
}


ulong PubDay2Long(const uchar *psYYYYMMDD)
{
	uint uiYear,uiMonth,uiDate,uiNowYear;
	
	ulong ulRet;
	
	if (psYYYYMMDD==NULL)
	{
		return 0;
	}

	uiNowYear=PubAsc2Long(&psYYYYMMDD[0],4);
	
	uiYear=uiNowYear-STAR_YEAR;
	
	/* Leap year */
	//ulRet=uiYear*365l+(uiYear/4)+(uiYear/400)-(uiYear/100);
	//ulRet=0;
	
	ulRet=(uiNowYear/4)-(STAR_YEAR/4);
	ulRet=ulRet+(STAR_YEAR/100)-(uiNowYear/100);
	ulRet=ulRet+(uiNowYear/400)-(STAR_YEAR/400);
	
	
	ulRet=uiYear*365l+ulRet;
	uiMonth=PubAsc2Long(&psYYYYMMDD[4],2);
	
	uiDate=uiConstMonthday[uiMonth-1];
	ulRet=ulRet+uiDate;
	if(IsLeapYear(uiNowYear))
	{
		if(IsLeapYear(STAR_YEAR))
		{
			if(uiMonth>=3)
			{
				/* Leap year */
				ulRet=ulRet+1;
			}
		}
		else
		{
			if(uiMonth<3)
			{
				/* End year is leap year, But start year isn't */
				ulRet=ulRet-1;
			}
			
		}
	}
	else if(IsLeapYear(STAR_YEAR))
	{
		/* End year isn't leap year, But start year is */
		ulRet=ulRet+1;
	}
	
	uiDate=PubAsc2Long(&psYYYYMMDD[6],2);
	uiDate=uiDate-1;
	ulRet=ulRet+uiDate;
	return ulRet;
}

ulong PubTime2Long(const uchar *psYYYYMMDDhhmmss)
{
	ulong ulRet;
	ulong ulTemp;
	
	if (psYYYYMMDDhhmmss==NULL)
	{
		return 0;
	}

	ulRet=PubDay2Long(psYYYYMMDDhhmmss);
	ulRet=ulRet*24;
	
	ulTemp=PubAsc2Long(&psYYYYMMDDhhmmss[8],2);
	ulRet=ulRet+ulTemp;
	/* convert to minute */
	ulRet=ulRet*60l;
	
	ulTemp=PubAsc2Long(&psYYYYMMDDhhmmss[10],2);
	ulRet=ulRet+ulTemp;
	/* convert to seconds */
	ulRet=ulRet*60l;
	
	ulTemp=PubAsc2Long(&psYYYYMMDDhhmmss[12],2);
	ulRet=ulRet+ulTemp;
	return ulRet;	
}

/* Check if leap year or not */
static uchar IsLeapYear(ulong ulYear)
{
	if((ulYear%400)==0)
	{
		return 1;
	}
	else if((ulYear%100)==0)
	{
		return 0;
	}
	else if((ulYear%4)==0)
	{
		return 1;
	}
	return 0;		
}	

void PubLong2Day(ulong ulDay,uchar *pszYYYYMMDD)  
{
	ulong ulYear,ulMonth;
	
	if(pszYYYYMMDD==NULL)
	{
		return;
	}

	ulYear=0;
	while(ulDay>365)
	{
		if(IsLeapYear(ulYear+STAR_YEAR))
		{
			ulDay--;
		}
		ulYear++;
		ulDay-=365;
	}
	if(ulDay==365)
	{
		if(IsLeapYear(ulYear+STAR_YEAR))
		{
			ulDay=31;
			ulMonth=12;
		}
		else
		{
			ulYear++;
			ulDay=1;
			ulMonth=1;
		}
		
	}
	else
	{
		for(ulMonth=1;ulMonth<=11;ulMonth++)
		{
			if(ulMonth==3)
			{
				if(IsLeapYear(ulYear+STAR_YEAR))
				{
					if(ulDay==uiConstMonthday[2])
					{
						ulMonth=2;
						break;
					}
					ulDay--;
				}
			}
			if(ulDay<uiConstMonthday[ulMonth])
			{
				break;
			}
		}
		ulDay=ulDay-uiConstMonthday[ulMonth-1];
		ulDay++;
	}
	sprintf(&pszYYYYMMDD[0],"%04lu",ulYear+STAR_YEAR);
	sprintf(&pszYYYYMMDD[4],"%02lu",ulMonth);
	sprintf(&pszYYYYMMDD[6],"%02lu",ulDay);
	pszYYYYMMDD[8]=0;
} 

void PubLong2Time(ulong ulSecond,uchar *pszYYYYMMDDhhmmss)   
{
	ulong ulYear,ulMonth,ulDay,ulDaySecond,ulWeek;
	
	if (pszYYYYMMDDhhmmss==NULL)
	{
		return;
	}

	ulDay=ulSecond/(24*3600l);
	ulWeek=(ulDay+STAR_WEEK)%7;
	ulWeek++;
	ulDaySecond=ulSecond%(24*3600);
	ulYear=0;
	while(ulDay>365)
	{
		if(IsLeapYear(ulYear+STAR_YEAR))
		{
			ulDay--;
		}
		ulYear++;
		ulDay-=365;
	}
	if(ulDay==365)
	{
		if(IsLeapYear(ulYear+STAR_YEAR))
		{
			ulDay=31;
			ulMonth=12;
		}
		else
		{
			ulYear++;
			ulDay=1;
			ulMonth=1;
		}
		
	}
	else
	{
		for(ulMonth=1;ulMonth<=11;ulMonth++)
		{
			if(ulMonth==3)
			{
				if(IsLeapYear(ulYear+STAR_YEAR))
				{
					if(ulDay==uiConstMonthday[2])
					{
						ulMonth=2;
						break;
					}
					ulDay--;
				}
			}
			if(ulDay<uiConstMonthday[ulMonth])
			{
				break;
			}
		}
		ulDay=ulDay-uiConstMonthday[ulMonth-1];
		ulDay++;
	}
	sprintf(&pszYYYYMMDDhhmmss[0],"%04lu",ulYear+STAR_YEAR);
	sprintf(&pszYYYYMMDDhhmmss[4],"%02lu",ulMonth);
	sprintf(&pszYYYYMMDDhhmmss[6],"%02lu",ulDay);
	sprintf(&pszYYYYMMDDhhmmss[8],"%02lu",ulDaySecond/3600);
	ulDaySecond=ulDaySecond%3600;
	sprintf(&pszYYYYMMDDhhmmss[10],"%02lu",ulDaySecond/60);
	ulDaySecond=ulDaySecond%60;
	sprintf(&pszYYYYMMDDhhmmss[12],"%02lu",ulDaySecond);
	//	sprintf(&pszYYYYMMDDhhmmss[14],"%02lu",ulWeek); //no need week yet
	pszYYYYMMDDhhmmss[14]=0;
}

void PubCalDateTime(const uchar *psDateTimeIn, uchar *pszDateTimeOut, ulong ulInterval, const uchar *psCalMode)
{//YYYYMMDDhhmmss format
	ulong ulYear,ulMonth,ulTemp;
	
	if (psDateTimeIn==NULL || pszDateTimeOut==NULL || psCalMode == NULL)
	{
		return;
	}

	if (psCalMode!=NULL)
	{
		switch(psCalMode[0])
		{
		case 'Y':
			ulYear = PubAsc2Long(psDateTimeIn,4);
			sprintf(pszDateTimeOut,"%04lu",ulYear+ulInterval);
			memcpy(pszDateTimeOut+4,psDateTimeIn+4,10);   
			pszDateTimeOut[14]=0;
			return;  //not break but return
		case 'M':
			ulYear = PubAsc2Long(psDateTimeIn,4);
			ulMonth = PubAsc2Long(psDateTimeIn+4,2);
			ulYear = ulYear + (ulMonth - 1 + ulInterval) / 12;
			ulMonth = (ulMonth -1 + ulInterval) % 12 + 1;
			sprintf(pszDateTimeOut,"%04lu",ulYear);
			sprintf(pszDateTimeOut+4,"%02lu",ulMonth);
			memcpy(pszDateTimeOut+6,psDateTimeIn+6,8);   
			pszDateTimeOut[14]=0;
			return;  //not break but return
		case 'D':
			ulInterval = ulInterval*3600*24;
			break;
		case 'h':
			ulInterval = ulInterval*3600;
			break;
		case 'm':
			ulInterval = ulInterval*60;
			break;
		case 's':
		default:
			break;
		}
	}
	
	ulTemp = PubTime2Long(psDateTimeIn)+ulInterval;
	PubLong2Time(ulTemp,pszDateTimeOut);
}

ulong PubCalInterval(const uchar *psTimeBegin, const uchar *psTimeEnd, const uchar *psCalMode, uchar *pucSignal)
{
	ulong ulBegin,ulEnd,ulRet;
	uchar *psBig,*psSmall;

	if (psTimeBegin==NULL || psTimeEnd==NULL || psCalMode==NULL || pucSignal==NULL)
	{
		return 0;
	}

	*pucSignal = 0;
	ulBegin = PubTime2Long(psTimeBegin);
	ulEnd   = PubTime2Long(psTimeEnd);
	
	if (ulEnd >= ulBegin)
	{
		*pucSignal = 0;
		ulRet = ulEnd - ulBegin;
		psBig   = (uchar *)psTimeEnd;
		psSmall = (uchar *)psTimeBegin;
	}
	else
	{
		*pucSignal = 1;
		ulRet = ulBegin - ulEnd;
		psBig   = (uchar *)psTimeBegin;
		psSmall = (uchar *)psTimeEnd;
	}
	
 
	switch(psCalMode[0])
	{
	case 'Y':
		ulRet = PubAsc2Long(psBig,4) - PubAsc2Long(psSmall,4);
		if (memcmp(psBig+4,psSmall+4,10)<0)
		{
			ulRet--;
		}
		return ulRet; //not break but return
	case 'M':
		ulRet =  PubAsc2Long(psBig,4)*12 + PubAsc2Long(psBig+4,2) 
			- PubAsc2Long(psSmall,4)*12 - PubAsc2Long(psSmall+4,2);
		if (memcmp(psBig+6,psSmall+6,8)<0)
		{
			ulRet--;
		}
		return ulRet; //not break but return
	case 'D':
		ulRet = ulRet/3600/24;
		break;
	case 'h':
		ulRet = ulRet/3600;
		break;
	case 'm':
		ulRet = ulRet/60;
		break;
	case 's':
	default:
		break;
	}
	
	return ulRet;
}




//...
// Modified by Kim_LinHB 2014-6-18 v1.01.0000
void PubHalt(const uchar *pszfile, uint uiLine)
{
	int iErr;
	unsigned char szBuff[200];

	iErr = GetLastError();

	sprintf(szBuff, "File:%s,\nLine:%u,\nRet:%d\nPLS RECORD\nTHEN RESTART POS", pszfile, uiLine, iErr);

	Gui_ClearScr();
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, -1, NULL);

	while (1);
}

////////////////////////////////////// Sound //////////////////////////////////////

void PubLongBeep(void)
{
	Beef(6, 800);
}


void PubBeepOk(void)
{
	uchar	i;
	
	for (i=0; i<3; i++)
	{
		Beef(6, 60);
		DelayMs(80);
	}
}


void PubBeepErr(void)
{
	Beef(1, 200);
	DelayMs(200);
}

////////////////////////////////////// File I/O //////////////////////////////////////


int PubFReadN(int fd, void *psData, int iNeedBytes)
{
	int		iReadBytes, iLeftBytes;
	uchar	*psTmp;
	
	PubASSERT( fd>=0 && psData!=NULL && iNeedBytes>=0 );
	psTmp      = (uchar *)psData;
	iLeftBytes = iNeedBytes;
	while(iLeftBytes>0)
	{
		iReadBytes = read(fd, psTmp, iLeftBytes);
		if (iReadBytes<=0)
		{
			PubTRACE1("errno:%d", errno);
			break;
		}
		iLeftBytes -= iReadBytes;
		psTmp	   += iReadBytes;
	}
	
	return (iNeedBytes-iLeftBytes);
}


uchar PubFileRead(const uchar *pszFileName, long lOffset, void *psData, int iLen)
{
	int		iRet, fd, iReadBytes;
	
	PubASSERT( pszFileName!=NULL&& *pszFileName!=0 && iLen>0 );
	fd = open((char *)pszFileName, O_RDWR);
	if (fd<0)
	{
		PubTRACE1("open:%s", pszFileName);
		PubTRACE1("errno:%d", errno);
		return 1;
	}
	
	iRet = seek(fd, lOffset, SEEK_SET);
	if (iRet<0)
	{
		close(fd);
		PubTRACE2("seek:%s,%ld", pszFileName, lOffset);
		PubTRACE1("errno:%d", errno);
		return 2;
	}
	
	iReadBytes = PubFReadN(fd, psData, iLen);
	close(fd);
	if (iReadBytes!=iLen)
	{
		return 3;
	}
	
	return 0;
}


int PubFWriteN(int fd, const void *psData, int iNeedBytes)
{
	int		iWriteBytes, iLeftBytes;
	uchar   *psTmp;
	
	PubASSERT( fd>=0 && psData!=NULL && iNeedBytes>=0 );
	psTmp      = (uchar *)psData;
	iLeftBytes = iNeedBytes;
	
	while(iLeftBytes>0)
	{
		iWriteBytes = write(fd, psTmp, iLeftBytes);
		if (iWriteBytes<=0)
		{
			PubTRACE2("FWN:%d,%d", iWriteBytes, iLeftBytes);
			PubTRACE1("errno:%d", errno);
			break;
		}
		iLeftBytes -= iWriteBytes;
		psTmp	   += iWriteBytes;
	}
	
	return (iNeedBytes-iLeftBytes);
}


uchar PubFileWrite(const uchar *pszFileName, long lOffset, const void *psData, int iLen)
{
	int	 iRet, fd, iWriteBytes;
	
	PubASSERT( pszFileName!=NULL && *pszFileName!=0 && iLen>=0 );
	fd = open((char *)pszFileName, O_RDWR|O_CREATE);
	if (fd<0)
	{
		PubTRACE1("open:%s", pszFileName);
		PubTRACE1("errno:%d", errno);
		return 1;
	}
	
	iRet = seek(fd, lOffset, SEEK_SET);
	if (iRet<0)
	{
		close(fd);
		PubTRACE2("seek:%s,%ld", pszFileName, lOffset);
		PubTRACE1("errno:%d", errno);
		return 2;
	}
	
	iWriteBytes = PubFWriteN(fd, psData, iLen);
	close(fd);
	if (iWriteBytes!=iLen)
	{
		PubTRACE2("FW:%d,%d", iWriteBytes, iLen);
		return 3;
	}
	
	return 0;
}

////////////////////////////////////// Calculation //////////////////////////////////////


void PubDes(uchar ucMode, const uchar *psData, const uchar *psKey, uchar *psResult)
{
	uchar   sTmp[8];
	
	PubASSERT(ucMode==ONE_ENCRYPT  || ucMode==ONE_DECRYPT ||
		ucMode==TRI_ENCRYPT  || ucMode==TRI_DECRYPT ||
		ucMode==TRI_ENCRYPT3 || ucMode==TRI_DECRYPT3);
	
	if ((psData == NULL) || (psKey == NULL) || (psResult == NULL))
	{
		return;
	}

	switch(ucMode)
	{
	case ONE_ENCRYPT:
		des((uchar *)psData, psResult, (uchar *)psKey, ENCRYPT);
		break;
		
	case ONE_DECRYPT:
		des((uchar *)psData, psResult, (uchar *)psKey, DECRYPT);
		break;
		
	case TRI_ENCRYPT:
		des((uchar *)psData,   psResult, (uchar *)psKey,   ENCRYPT);
		des(psResult, sTmp,     (uchar *)psKey+8, DECRYPT);
		des(sTmp,     psResult, (uchar *)psKey,   ENCRYPT);
		break;
		
	case TRI_DECRYPT:
		des((uchar *)psData,   psResult, (uchar *)psKey,   DECRYPT);
		des(psResult, sTmp,     (uchar *)psKey+8, ENCRYPT);
		des(sTmp,     psResult, (uchar *)psKey,   DECRYPT);
		break;
		
	case TRI_ENCRYPT3:
		des((uchar *)psData,   psResult, (uchar *)psKey,    ENCRYPT);
		des(psResult, sTmp,     (uchar *)psKey+8,  DECRYPT);
		des(sTmp,     psResult, (uchar *)psKey+16, ENCRYPT);
		break;
		
	case TRI_DECRYPT3:
		des((uchar *)psData,   psResult, (uchar *)psKey+16, DECRYPT);
		des(psResult, sTmp,     (uchar *)psKey+8,  ENCRYPT);
		des(sTmp,     psResult, (uchar *)psKey,    DECRYPT);
		break;
	}
}


void PubCalcMac(uchar ucMode, const uchar *psKey, const uchar *psMsg, uint uiLen, uchar *psMac)
{
	uchar   sOutMac[8];
	uint	uiOffset, i;
	
	PubASSERT(ucMode==MAC_FAST || ucMode==MAC_ANSIX99 || ucMode==MAC_ANSIX919);
	memset(sOutMac, 0, sizeof(sOutMac));
	uiOffset = 0;

	if ((psKey == NULL) || (psMsg == NULL) || (psMac == NULL))
	{
		return;
	}
	
	while(uiLen>uiOffset)
	{
		if (uiLen-uiOffset<=8)
		{
			for (i=0; i<uiLen-uiOffset; i++)
			{
				sOutMac[i] ^= psMsg[uiOffset+i];
			}
			break;
		}
		for (i=0; i<8; i++)
		{
			sOutMac[i] ^= psMsg[uiOffset+i];
		}
		if (ucMode==MAC_ANSIX99 || ucMode==MAC_ANSIX919)
		{
			PubDes(ONE_ENCRYPT, sOutMac, psKey, sOutMac);
		}
		uiOffset += 8;
	}
	
	PubDes(ONE_ENCRYPT, sOutMac, psKey, psMac);
	if (ucMode==MAC_ANSIX919)
	{
		PubDes(ONE_DECRYPT, psMac, psKey+8, sOutMac);
		PubDes(ONE_ENCRYPT, sOutMac, psKey, psMac);
	}
}

// calcaute LRC
void PubCalcLRC(const uchar *psData, ushort uiLength, uchar *pucInit)
{
	uchar ucInit;

	if ((psData == NULL) || (pucInit == NULL))
	{
		return;
	}

	ucInit = *psData++;
	uiLength--;
	
	while( uiLength>0 )
	{
		ucInit ^= *psData++;
		uiLength--;
	}

	*pucInit = ucInit;
}



/*********************************************************************************************/

//									Debug Output

/*********************************************************************************************/

int UnPackElement(const FIELD_ATTR *pAttr, const uchar *pusIn, uchar *pusOut,
                   uint *puiInLen)
{
    uint    i, j, iInLen, iTmpLen;

    memset(pusOut, 0, pAttr->uiLength);
    *puiInLen = 0;

    switch( pAttr->eLengthAttr ){
    case Attr_fix:
        iInLen  = pAttr->uiLength;
        iTmpLen = iInLen;
        break;

    case Attr_var1:
        iTmpLen= (pusIn[0]>>4)*10 + (pusIn[0]&0x0F);
        pusIn++;
        iInLen= 1+iTmpLen;
        break;

    case Attr_var2:
        iTmpLen = (pusIn[0]&0x0F)*100 + (pusIn[1]>>4)*10 + (pusIn[1]&0x0F);
        pusIn += 2;
        iInLen = 2+iTmpLen;
        break;
    }   /*** switch(pAttr->eLengthAttr ***/

    if( iTmpLen>pAttr->uiLength ){
        return -1;
    }
    if( pAttr->eElementAttr==Attr_b ){
        pusOut[0] = (uchar)(iTmpLen>>8);
        pusOut[1] = (uchar)iTmpLen;
        pusOut   += 2;
    }

    switch( pAttr->eElementAttr ){
    case Attr_n:
        switch( pAttr->eLengthAttr ){
        case Attr_fix:
            iInLen= (pAttr->uiLength+1)/2;

            for(i=0,j=0; i<pAttr->uiLength; i+=2,j++){
                if( (pusIn[iInLen-j-1]&0x0F)<0x0A ){
                    pusOut[pAttr->uiLength-i-1] = (pusIn[iInLen-j-1]&0x0F)|0x30;
                }else{
                    pusOut[pAttr->uiLength-i-1] =
                        (pusIn[iInLen-j-1]&0x0F)-0x0A+'A';
                }

                if( i!=pAttr->uiLength-1 ){
                    if( (pusIn[iInLen-j-1]>>4)<0x0A ){
                        pusOut[pAttr->uiLength-i-2] =
                           (pusIn[iInLen-j-1]>>4)|0x30;
                    }else{
                        pusOut[pAttr->uiLength-i-2]=
                           (pusIn[iInLen-j-1]>>4)-0x0A+'A';
                    }
                }
            }
            break;

        case Attr_var1:
        case Attr_var2:
            iInLen = iInLen - iTmpLen + (iTmpLen+1)/2;

#ifndef VarAttrN_RightJustify
            for(i=0; i<(iTmpLen/2); i++){
                if( (pusIn[i]>>4)<0x0A ){
                    pusOut[2*i] = (pusIn[i]>>4) | 0x30;
                }else{
                     pusOut[2*i] = (pusIn[i]>>4) -0x0A + 'A';
                }

                if( (pusIn[i]&0x0F)<0x0A ){
                     pusOut[2*i+1] = (pusIn[i]&0x0F) | 0x30;
                }else{
                     pusOut[2*i+1] = (pusIn[i]&0x0F) - 0x0A + 'A';
                }
            }

            if( iTmpLen%2 ){
                if( (pusIn[i]>>4)<0x0A ){
                     pusOut[2*i] = (pusIn[i]>>4) | 0x30;
                }else{
                     pusOut[2*i] = (pusIn[i]>>4) - 0x0A + 'A';
                }
            }
#else
            for(i=0,j=0; i<iTmpLen; i+=2,j++){
                if( (pusIn[(iTmpLen+1)/2-j-1]&0x0F)<0x0A ){
                    pusOut[iTmpLen-i-1] = (pusIn[(iTmpLen+1)/2-j-1]&0x0F)|0x30;
                }else{
                     pusOut[iTmpLen-i-1] =
                        (pusIn[(iTmpLen+1)/2-j-1]&0x0F)-0x0A+'A';
                }

                if( i!=pAttr->uiLength-1 ){
                    if( (pusIn[(iTmpLen+1)/2-j-1]>>4)<0x0A ){
                        pusOut[iTmpLen-i-2] =
                           (pusIn[(iTmpLen+1)/2-j-1]>>4)|0x30;
                    }else{
                        pusOut[iTmpLen-i-2] =
                           (pusIn[(iTmpLen+1)/2-j-1]>>4)-0x0A+'A';
                    }
                }
            }
#endif
            break;
        }   /*** switch(pAttr->eLengthAttr) ***/
        break;

    case Attr_z:
        switch( pAttr->eLengthAttr ){
        case Attr_fix:
            iInLen = (pAttr->uiLength+1)/2;

            for(i=0,j=0; i<pAttr->uiLength; i+=2,j++){
                pusOut[pAttr->uiLength-i-1]= (pusIn[iInLen-j-1]&0x0F)|0x30;
                if( i!=pAttr->uiLength-1 ){
                     pusOut[pAttr->uiLength-i-2]= (pusIn[iInLen-j-1]>>4)|0x30;
                }
            }
            break;

        case Attr_var1:
        case Attr_var2:
            iInLen = iInLen - iTmpLen + (iTmpLen+1)/2;

            for(i=0; i<(iTmpLen/2); i++){
                pusOut[2*i]   = (pusIn[i]>>4) | 0x30;
                pusOut[2*i+1] = (pusIn[i]&0x0F) | 0x30;
            }
            if( iTmpLen%2 ){
                pusOut[2*i]= (pusIn[i]>>4) | 0x30;
            }
            break;
        }   /*** switch(pAttr->eLengthAttr) ***/
        break;

    case Attr_b:
        memcpy(pusOut, pusIn, iTmpLen);
        break;

    case Attr_a:
        memcpy(pusOut, pusIn, iTmpLen);
        break;
    }    /*** switch(pAttr->eElementAttr) ***/

    *puiInLen = iInLen;

    return 0;
}

#define END_FLAG	0x80

//Modified by Kim_LinHB 2014-6-7 v1.01.0000
uchar ScrSend(const uchar *psData, ushort nDataLen, ushort nStartLine)
{
	ushort nCurrLen;
	ushort nCurrLine;

	nCurrLine = nStartLine%6;
	nCurrLen = 0;

	while(1)
	{
		if (nCurrLine%6 ==0 && nCurrLine != 0)
		{
			getkey();
			Gui_ClearScr();
		}
		if (nDataLen - nCurrLen > 21)
		{
			unsigned char szBuff[22];
			sprintf(szBuff, "%21.21s", psData+nCurrLen);
			Gui_DrawText(szBuff, gl_stLeftAttr, 0, nCurrLine%6+25);
			nCurrLen += 21;
			nCurrLine++;
		}
		else
		{   
			unsigned char szBuff[50];
			sprintf(szBuff, "%.*s", nDataLen - nCurrLen, psData+nCurrLen);
			Gui_DrawText(szBuff, gl_stLeftAttr, 0, nCurrLine%6+25);
			nCurrLen += nDataLen - nCurrLen;
			if (nCurrLine%6+2 == 7)
			{
				getkey();
				return 1;
			}
			nCurrLine++;
			break;
		}	
	}
	return 0;
}

uchar PortSendstr(uchar ucChannel, const uchar *psStr, ushort nStrLen)
{
	uchar ucRet;

	while(nStrLen--)
	{
		ucRet = PortSend(ucChannel, *psStr++);
		if (ucRet != 0x00)
			return ucRet;
	}
	return 0;
}
	
uchar UnitSend(uchar *psDataInOut, ushort nDataLen, uchar ucDevice, uchar ucMode, ushort nStartLine)
{
	uchar ucPort;
	uchar ucRet;

	if (ucDevice&DEVICE_SCR)
	{
		ucRet = ScrSend(psDataInOut, nDataLen, nStartLine);
		if ((ucDevice&END_FLAG) && ucRet == 0)
			getkey();
	}

	if (ucDevice&DEVICE_PRN)
	{
		psDataInOut[nDataLen] = 0x00;
		PrnStr(psDataInOut);

		if ((ucMode&ISO_MODE) || (ucMode&TLV_MODE))
		{
			PrnStr("\n");
		}
		if (ucDevice&END_FLAG)
		{
			PrnStr("\n\n\n\n\n\n\n");
			PrnStart();
			PrnInit();
			PrnFontSet(0,0);
		}
		else if (nDataLen >= 3360)
		{
			PrnStart();
			PrnInit();
			PrnFontSet(0,0);
		}
	}

	if ((ucDevice&DEVICE_COM1) || (ucDevice&DEVICE_COM2))
	{
		ucPort = ucDevice&DEVICE_COM1?0:1;
		PortSendstr(ucPort, psDataInOut, nDataLen);
		DelayMs(50);// make sure sending is done
		if ((ucMode&ISO_MODE) || (ucMode&TLV_MODE))
		{
			PortSendstr(ucPort, "\x0d\x0a", 2);
		}
		if (ucDevice&END_FLAG)
		{
			PortSendstr(ucPort, "\x0d\x0a\x0d\x0a", 4);
		}
	}

	return 0;
}

void PubISODefine(const void *pvDataDef)
{
	pstDataAttr = (FIELD_ATTR *)pvDataDef;
}
//
uchar PubDebugOutput(const uchar *pszTitle, const uchar *psData, ushort nDataLen, uchar ucDevice, uchar ucMode)
{
	ushort nCurrLen;
	ushort nUnitLen;
	ushort nCurrLine;
	ushort nTmpLen;
	ushort nSendLen;
	ushort nFixLen;
	uchar sDataBuff[3360+1];
	uchar sFieldBuff[1024];
	uchar sBitmapBuf[20];
	uchar ucLengthValue;
	uchar ucOverFlag;
	uchar ucPort;
	int iCnt;
	int iFieldCnt;
	int iRet;
	uint uiFieldLen;
	uint uiBitmap;
	uchar szTitle[32+1];

	if (psData == NULL)
		return 0xff;
	if(!(ucDevice&DEVICE_SCR) && !(ucDevice&DEVICE_PRN) && !(ucDevice&DEVICE_COM1)&& !(ucDevice&DEVICE_COM2))
		return 0xff;
	if(!(ucMode&ASC_MODE) && !(ucMode&HEX_MODE) && !(ucMode&ISO_MODE) && !(ucMode&TLV_MODE))
		return 0xff;
//	if ((ucDevice&0xf0) || (ucMode&0x0f))
//	{
//		return 0xff;
//	}
	if (nDataLen == 0)
	{
		return 0;
	}

	ucDevice = ucDevice&0x7f;

	ucOverFlag = 0;
	memset(szTitle, 0, sizeof(szTitle));
	if (pszTitle != NULL)
	{
		sprintf(szTitle, "%.*s", 32, pszTitle);
	}

	if (ucDevice&DEVICE_SCR)
	{
		kbflush();
	}

	if (ucDevice&DEVICE_PRN)
	{
		PrnInit();
		PrnFontSet(0,0);
		if (szTitle[0] != 0)
		{
			PrnStr(szTitle);
			PrnStr("\n");
		}
	}

	if ((ucDevice&DEVICE_COM1) || (ucDevice&DEVICE_COM2))
	{
		ucPort = ucDevice&DEVICE_COM1?0:1;
		if (PortOpen(ucPort, "115200,8,n,1") != 0)
		{
			ucOverFlag = 1;
//			return 0xff;
		}
		
		if (szTitle[0] != 0)
		{
			PortSendstr(ucPort, szTitle, (ushort)(strlen(szTitle)));
			PortSendstr(ucPort, "\x0d\x0a", 2);
		}
	}

	// 21 characters per line for screen, 
	// 48 characters per line for thermal printer,
	// 30 characters per line for stylus printer,
	nUnitLen = 3360;

	if (ucMode&ASC_MODE)
	{
		nCurrLen = 0;
		nCurrLine = 0;
		while(1)
		{
			if (nDataLen-nCurrLen < nUnitLen)
			{
				nSendLen = nDataLen-nCurrLen;
				memcpy(sDataBuff, psData+nCurrLen, nSendLen);
				UnitSend(sDataBuff, nSendLen, (uchar)(ucDevice|END_FLAG), ucMode, nCurrLine);
//				UnitSend(psData+usCurrLen, (ushort)(usDataLen-usCurrLen), (uchar)(ucDevice|END_FLAG), ucMode, 0);	
				nCurrLen += nSendLen;
				nCurrLine += nSendLen/21+(nSendLen%21?1:0);
				break;
			}
			else
			{
				nSendLen = nUnitLen;
				memcpy(sDataBuff, psData+nCurrLen, nUnitLen);
				UnitSend(sDataBuff, nSendLen, ucDevice, ucMode, nCurrLine);
//				UnitSend(psData+usCurrLen, usUnitLen, ucDevice, ucMode, 0);
				nCurrLen += nSendLen;
				nCurrLine += nSendLen/21+(nSendLen%21?1:0);
			}
		}
	}

	if (ucMode&HEX_MODE)
	{	
		nCurrLen = 0;
		nCurrLine = 0;
		while(1)
		{
			if (nDataLen-nCurrLen < nUnitLen/3)
			{
				for (iCnt=0; iCnt < nDataLen-nCurrLen; iCnt++)
					sprintf(sDataBuff+iCnt*3, "%02x ", psData[nCurrLen+iCnt]);
				nSendLen = (ushort)((nDataLen-nCurrLen)*3);
				UnitSend(sDataBuff, nSendLen, (uchar)(ucDevice|END_FLAG), ucMode, nCurrLine);	
				nCurrLen += nDataLen - nCurrLen;
				nCurrLine += nSendLen/21+(nSendLen%21?1:0);
				break;
			}
			else
			{
				for (iCnt=0; iCnt< nUnitLen/3; iCnt++)
					sprintf(sDataBuff+iCnt*3, "%02x ", psData[nCurrLen+iCnt]);

				nSendLen = nUnitLen;
				UnitSend(sDataBuff, nUnitLen, ucDevice, ucMode, nCurrLine);
				nCurrLen += nUnitLen;
				nCurrLine += nSendLen/21+(nSendLen%21?1:0);
			}
		}
	}

	if (ucMode&ISO_MODE)
	{
		nCurrLen = 0;
		nCurrLine = 0;
		if (pstDataAttr == NULL)
		{
			pstDataAttr = DebugDefaulDef;
		}

		//tpdu
		sprintf(sDataBuff, "tpdu=");
		nTmpLen = LEN_MSG_HEADER;
		memcpy(sFieldBuff, psData+nCurrLen, nTmpLen);
		nCurrLen += nTmpLen;
		nFixLen = strlen(sDataBuff);
		for (iCnt=0; iCnt< nTmpLen; iCnt++)
			sprintf(sDataBuff+nFixLen+iCnt*3, "%02x ", sFieldBuff[iCnt]);
		nSendLen = nFixLen+nTmpLen*3;
		UnitSend(sDataBuff, nSendLen, ucDevice, ucMode, nCurrLine);
		nCurrLine += nSendLen/21+(nSendLen%21?1:0);

		//message
		uiFieldLen = 0;
		iRet = UnPackElement(pstDataAttr, psData+nCurrLen, sFieldBuff, &uiFieldLen);
		nCurrLen += uiFieldLen;
		sprintf(sDataBuff, "[0]=");
		nFixLen = strlen(sDataBuff);
		memcpy(sDataBuff+nFixLen, sFieldBuff, pstDataAttr->uiLength);
		nSendLen = nFixLen+pstDataAttr->uiLength;
		UnitSend(sDataBuff, nSendLen, ucDevice, ucMode, nCurrLine);				
		nCurrLine += nSendLen/21+(nSendLen%21?1:0);

		//bitmap
		if( psData[nCurrLen] & 0x80 )
		{
			uiFieldLen = (pstDataAttr+1)->uiLength*2;
		}
		else
		{
			uiFieldLen = (pstDataAttr+1)->uiLength;
		}
		uiBitmap = uiFieldLen;		
		memcpy(sBitmapBuf, psData+nCurrLen, uiFieldLen);
		nCurrLen += uiFieldLen;
		sprintf(sDataBuff, "[1]=");
		nFixLen = strlen(sDataBuff);
		for (iCnt = 0; iCnt < (int)uiFieldLen; iCnt++)
			sprintf(sDataBuff+nFixLen+iCnt*3, "%02x ", sBitmapBuf[iCnt]);	
		nSendLen = nFixLen+uiFieldLen*3;
		UnitSend(sDataBuff, nSendLen, ucDevice, ucMode, nCurrLine);
		nCurrLine += (nSendLen)/21+(nSendLen%21?1:0);

		//field2~64/128
		for (iFieldCnt=1; iFieldCnt<(int)(uiBitmap*8); iFieldCnt++)
		{
			if( sBitmapBuf[iFieldCnt/8] & (0x80>>iFieldCnt%8)) 
			{
				memset(sFieldBuff, 0, sizeof(sFieldBuff));
				uiFieldLen = 0;
				iRet = UnPackElement(pstDataAttr+1+iFieldCnt, psData+nCurrLen, sFieldBuff, &uiFieldLen);
				if( iRet<0 || uiFieldLen > (pstDataAttr+1+iFieldCnt)->uiLength || uiFieldLen+nCurrLen > nDataLen||
					uiFieldLen <= 0)
				{
					sprintf(sDataBuff, "field %d unpack error", iFieldCnt+1);
					UnitSend(sDataBuff, (ushort)(strlen(sDataBuff)), (uchar)(ucDevice|END_FLAG), ucMode, nCurrLine);
					ucOverFlag = 1;
					break;
//					return 0xff;
				}
				nCurrLen+= uiFieldLen;

				if ((pstDataAttr+1+iFieldCnt)->eLengthAttr != Attr_fix)
				{
					
					if ((pstDataAttr+1+iFieldCnt)->eElementAttr == Attr_n ||
						(pstDataAttr+1+iFieldCnt)->eElementAttr == Attr_z ||
						(pstDataAttr+1+iFieldCnt)->eElementAttr == Attr_a)
					{
						nTmpLen = strlen(sFieldBuff);
						ucLengthValue =0;
					}
					else 
					{
						ucLengthValue = (pstDataAttr+1+iFieldCnt)->eLengthAttr+1;
						nTmpLen = uiFieldLen-ucLengthValue;
					}					
					sprintf(sDataBuff, "[%d]=(%u)", iFieldCnt+1, nTmpLen);
				}
				else
				{
					nTmpLen = (pstDataAttr+1+iFieldCnt)->uiLength;
					sprintf(sDataBuff, "[%d]=", iFieldCnt+1);
					ucLengthValue = 0;
				}
				nFixLen = strlen(sDataBuff);
				if ((pstDataAttr+1+iFieldCnt)->eElementAttr== Attr_b)
				{											
					for (iCnt=0; iCnt< nTmpLen; iCnt++)
						sprintf(sDataBuff+nFixLen+iCnt*3, "%02x ", sFieldBuff[iCnt+ucLengthValue]);
					nSendLen = nFixLen+nTmpLen*3;
				}
				else
				{					
					memcpy(sDataBuff+nFixLen, sFieldBuff+ucLengthValue, nTmpLen);	
					nSendLen = nFixLen + nTmpLen;
				}
				
				UnitSend(sDataBuff, nSendLen, ucDevice, ucMode, nCurrLine);
				nCurrLine += nSendLen/21+(nSendLen%21?1:0);
			}	
		}
		
		if (ucOverFlag == 0)
		{		
			sprintf(sDataBuff, "pack ok!");
			UnitSend(sDataBuff, (ushort)(strlen(sDataBuff)), (uchar)(ucDevice|END_FLAG), ucMode, nCurrLine);
		}
		pstDataAttr = NULL;
	}

	if (ucMode&TLV_MODE)
	{
		nCurrLen = 0;
		nCurrLine = 0;
		while (nCurrLen < nDataLen)
		{
			//tag
			if ((~(psData[nCurrLen] &0x1f))&0x1f)//1 byte tag
			{
				sprintf(sDataBuff, "Tag:%02x", psData[nCurrLen]);
				nCurrLen ++;
				nSendLen = strlen(sDataBuff);
				UnitSend(sDataBuff, nSendLen, ucDevice, ucMode, nCurrLine);
				nCurrLine += (nSendLen)/21+(nSendLen%21?1:0);
			}
			else					//2 bytes tag
			{
				sprintf(sDataBuff, "Tag:%02x ", psData[nCurrLen]);
				nCurrLen ++;
// 				do
// 				{	
// 					usFixLen = strlen(sDataBuff);
// 					sprintf(sDataBuff+usFixLen, "%02x ", psData[usCurrLen]);
// 					usCurrLen ++;
// 				}while((psData[usCurrLen-1]&0x80));
				nFixLen = strlen(sDataBuff);
				sprintf(sDataBuff+nFixLen, "%02x ", psData[nCurrLen]);
				nCurrLen++;
				nSendLen = strlen(sDataBuff);
				UnitSend(sDataBuff, nSendLen, ucDevice, ucMode, nCurrLine);
				nCurrLine += (nSendLen)/21+(nSendLen%21?1:0);
			}

			//length
			if (psData[nCurrLen]&0x80)//n bytes length
			{
				nTmpLen = 0;
				uiFieldLen = psData[nCurrLen]&0x7f;
				nCurrLen ++;
				sprintf(sDataBuff, "  L:");
				nFixLen = strlen(sDataBuff);
				for (iCnt = 0; iCnt < (int)uiFieldLen; ++iCnt)
				{
					nTmpLen += psData[nCurrLen+iCnt]<<(8*(iCnt-1));
					sprintf(sDataBuff+nFixLen+iCnt*3, "%02x ", psData[nCurrLen+iCnt]);
				}
				nCurrLen += uiFieldLen;
				nSendLen = strlen(sDataBuff);
				UnitSend(sDataBuff, nSendLen, ucDevice, ucMode, nCurrLine);
				nCurrLine += (nSendLen)/21+(nSendLen%21?1:0);
			}
			else						//1 byte length
			{
				sprintf(sDataBuff, "  L:%02x", psData[nCurrLen]);
				nTmpLen = psData[nCurrLen];
				nCurrLen ++;				
				nSendLen = strlen(sDataBuff);
				UnitSend(sDataBuff, nSendLen, ucDevice, ucMode, nCurrLine);
				nCurrLine += (nSendLen)/21+(nSendLen%21?1:0);
			}

			//value
			if (nTmpLen+nCurrLen > nDataLen)
			{
				sprintf(sDataBuff, "TLV fail!");
				UnitSend(sDataBuff, (ushort)(strlen(sDataBuff)), (uchar)(ucDevice|END_FLAG), ucMode, nCurrLine);
				ucOverFlag = 1;
				break;
			}
			sprintf(sDataBuff, "  V:");
			nFixLen = strlen(sDataBuff);
			for (iCnt = 0; iCnt < nTmpLen; iCnt++)
				sprintf(sDataBuff+nFixLen+iCnt*3, "%02x ", psData[nCurrLen+iCnt]);
			nCurrLen += nTmpLen;
			nSendLen = nFixLen+nTmpLen*3-1;	 // -1 is for removing the last space character
			UnitSend(sDataBuff, nSendLen, ucDevice, ucMode, nCurrLine);
			nCurrLine += nSendLen/21+(nSendLen%21?1:0);
		}
		if (ucOverFlag == 0)
		{
			sprintf(sDataBuff, "TLV ok!");
			UnitSend(sDataBuff, (ushort)(strlen(sDataBuff)), (uchar)(ucDevice|END_FLAG), ucMode, nCurrLine);		
		}
	}
	if (ucOverFlag)
		return 0xff;
	else
		return 0;
}




