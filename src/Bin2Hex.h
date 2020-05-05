#ifndef _BIN2HEX_H
#define _BIN2HEX_H
int Bin2Hex(char *theData, int theDataLen, char *hexEncodedStr, int theHexLen);
int binTohex(unsigned char *pcInBuffer, char *pcOutBuffer, int iLen);
int hex2bin(const char *pcInBuffer, char *pcOutBuffer, int iLen);
#endif
