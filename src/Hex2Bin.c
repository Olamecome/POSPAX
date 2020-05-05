//------------------------------------------------------------[ Includes ]---
#include <string.h>
#include "Hex2Bin.h"

//-------------------------------------------------------------[ Defines ]---

//----------------------------------------------------[ Global Variables ]---

//-----------------------------------------------------[ Local Variables ]---

//-------------------------------------------------------------[ hex2bin ]---
// DESC: Convert a hex string to binary
// ARGS:
//		pInBuffer - Null terminated hex string
//		pOutBuffer - Output buffer
//
// RETURN:
//		The number of bytes inserted into the pOutBuffer
//---------------------------------------------------------------------------
int hex2bin(const char *pcInBuffer, char *pcOutBuffer, int iLen)
{
	int 	c;
	char	*p;
	int 	iBytes=0;
	const char *hextable = "0123456789ABCDEF";
	int nibble = 0;
	int nibble_val;

	while(iBytes < iLen)	{
		c = *pcInBuffer++;
		if (c == 0)
			break;

		c = toupper(c);

		p = strchr(hextable, c);
		if (p)
		{
			if (nibble & 1)
			{
				iBytes++;
				*pcOutBuffer = (nibble_val << 4 | (p - hextable));
				pcOutBuffer++;
			}//if
			else
			{
				nibble_val = (p - hextable);
			}//else
			nibble++;
		}//if
		else
		{
			nibble = 0;
		}//else
	}//while
	
	return iBytes;
}//hex2bin


//---------------------------------------------------------------------------
//
//                               T H E   E N D
//
//---------------------------------------------------------------------------
