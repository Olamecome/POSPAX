#include "posapi.h"
#include "print.h"

#define DATA_ERROR1  1
#define DATA_ERROR2  2
#define DATA_ERROR3  3
#define DATA_ERROR4  4

struct PAX_PRN
{
	unsigned char  count[2];
	unsigned char  content[384];
} Pax_prn;

struct PAX_LOGO
{
	unsigned char  line;
	struct PAX_PRN  prn_txt[255];
} Pax_logo;

void conv_bmp(unsigned char *dest, unsigned char *src, int len, int jplen, unsigned char off)
{
	int i, tlen;
	//the last pix need reversed
	if (off != 0)
	{
		tlen = len - jplen-1;
	}
	else
	{
		tlen = len - jplen;
	}
	for(i=0; i<tlen; i++)
	{
		dest[i] = src[i] ^ 0xff;
	}

	if(off!= 0)
	{
		dest[tlen] = src[tlen] ^ (0xff<<(8-off));
	}
}

int PrnBmp(unsigned char *filename,int mode,char alignment, unsigned char *gMallocBuffer)
{
	int fp;
	unsigned char *bmp = NULL;
	long LineByte;
	uint i=0;
	int iFileLen;
	int iBmpPixWidth, iBmpWidth, iBmpFillLen ,iBmpPixBitCount,iBmpByteSize;
	unsigned char ucOff;
	unsigned short usLeftIndent = 0;
	int iMaxLineInOneBuf = 20000 / 74;
	int iCnt = 0;
	LineByte=filesize(filename);
	if(LineByte < 0)
		return -2;

	fp=open(filename,O_RDWR);
	if(fp < 0) {
		return -1;
	}

	bmp = (unsigned char *)malloc(LineByte);
	memset(bmp,0,sizeof(bmp));
	read(fp,bmp,LineByte);
	close(fp);

	if(!(bmp[0x00]=='B' && bmp[0x01]=='M')){
		free(bmp);
		return  DATA_ERROR1;
	}

	iFileLen = ((int)bmp [0x05]) * 16777216 +
		((int)bmp [0x04]) * 65536 +
		((int)bmp [0x03]) * 256  +
		(int)bmp [0x02];

	iBmpPixWidth = ((int)bmp [0x15]) * 16777216 +
		((int)bmp [0x14]) * 65536 +
		((int)bmp [0x13]) * 256  +
		(int)bmp [0x12];
	if(iBmpPixWidth>384) {
		free(bmp);
		return  DATA_ERROR4;
	}

	iBmpPixBitCount = ((int)bmp[0x1d]) * 256 + ((int)bmp[0x1c]);
	if(iBmpPixBitCount != 1) {
		free(bmp);
		return DATA_ERROR2;
	}

	iBmpByteSize = ((int)bmp[0x25]) * 16777216 +
	    ((int)bmp [0x24]) * 65536 +
		((int)bmp [0x23]) * 256  +
		(int)bmp [0x22];
	if(iBmpByteSize > 36864) {
		free(bmp);
		return DATA_ERROR3;
	}

	ucOff = iBmpPixWidth%8;
	if(ucOff !=0) {
		iBmpPixWidth +=8;
	}

	if(((iBmpPixWidth/8)%4) != 0) {
		iBmpFillLen = 4-(iBmpPixWidth/8)%4;
	}
	else {
		iBmpFillLen = 0;
	}
	iBmpWidth = iBmpPixWidth/8+iBmpFillLen;

	switch(alignment) {
	case 1: usLeftIndent = (384/8 - iBmpWidth)/2;break;
	case 2: usLeftIndent = 384/8 - iBmpWidth;break;
	case 0:
	default:
		break;
	}

	Pax_logo.line = bmp[0x16];
	for(i=0; i<Pax_logo.line; i++) {
		memset(Pax_logo.prn_txt[i].content, 0, 72);
		Pax_logo.prn_txt[i].count[0] = 0;
		Pax_logo.prn_txt[i].count[1] = 72;
		if(mode)
			memcpy(Pax_logo.prn_txt[i].content + usLeftIndent, &bmp[iFileLen-(i+1)*iBmpWidth], iBmpWidth);
		else
			conv_bmp(Pax_logo.prn_txt[i].content + usLeftIndent, &bmp[iFileLen-(i+1)*iBmpWidth], iBmpWidth, iBmpFillLen, ucOff);
	}

	gMallocBuffer[0] = Pax_logo.line;

	for(i= 0;i<Pax_logo.line; i++) {
		memcpy(&gMallocBuffer[iCnt*74+1], Pax_logo.prn_txt[i].count, 2);
		memcpy(&gMallocBuffer[iCnt*74+2+1], Pax_logo.prn_txt[i].content, 72);
		++iCnt;
		if(iCnt >= iMaxLineInOneBuf)
		{
		    PrnLogo(gMallocBuffer);
		    iCnt = 0;
		}
	}
	if(iMaxLineInOneBuf > Pax_logo.line){
	    PrnLogo(gMallocBuffer);
	}

	free(bmp);
	return 0;
}
