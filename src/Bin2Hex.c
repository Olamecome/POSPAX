#include "Bin2Hex.h"
#include "String.h"

int Bin2Hex(char *theData, int theDataLen, char *hexEncodedStr, int theHexLen) {
	int k;
	int index = 0;
	char hexLookUp[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
			'B', 'C', 'D', 'E', 'F' };
	if (theHexLen <= 0) {
		return 0;
	}
	memset(hexEncodedStr, '\0', theHexLen);

	for (k = 0, index = 0;
			(k < theDataLen) && ((index = (k * 2) + 1) < theHexLen); k++) {
		hexEncodedStr[index - 1] = hexLookUp[theData[k] / 16];
		hexEncodedStr[index] = hexLookUp[theData[k] % 16];
	}
	return 1;
}

int binTohex(unsigned char *pcInBuffer, char *pcOutBuffer, int iLen) {

	int iCount;
	char *pcBuffer;
	unsigned char *pcTemp;
	unsigned char ucCh;

	memset(pcOutBuffer, 0, sizeof(pcOutBuffer));
	pcTemp = pcInBuffer;
	pcBuffer = pcOutBuffer;
	for (iCount = 0; iCount < iLen; iCount++) {
		ucCh = *pcTemp;
		pcBuffer += sprintf(pcBuffer, "%02X", (int) ucCh);
		pcTemp++;
	} //while

	return 0;
}

//
//int hex2bin(const char *pcInBuffer, char *pcOutBuffer, int iLen)
//{
//	int 	c;
//	char	*p;
//	int 	iBytes=0;
//	const char *hextable = "0123456789ABCDEF";
//	int nibble = 0;
//	int nibble_val;
//
//	while(iBytes < iLen)	{
//		c = *pcInBuffer++;
//		if (c == 0)
//			break;
//
//		c = toupper(c);
//
//		p = strchr(hextable, c);
//		if (p)
//		{
//			if (nibble & 1)
//			{
//				iBytes++;
//				*pcOutBuffer = (nibble_val << 4 | (p - hextable));
//				pcOutBuffer++;
//			}//if
//			else
//			{
//				nibble_val = (p - hextable);
//			}//else
//			nibble++;
//		}//if
//		else
//		{
//			nibble = 0;
//		}//else
//	}//while
//	
//	return iBytes;
//}//hex2bin

