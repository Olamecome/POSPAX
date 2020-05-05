
#include "global.h"


#define MAX_CMD_LEN  300
#define MAX_RSP_LEN  100

#define COMM_ERR     0xff
#define NO_PIN       0x0a
#define NO_AMT       0x0b
#define USER_CANCEL  0x06

#ifndef uchar
#define uchar unsigned char
#endif
#ifndef uint
#define uint unsigned short
#endif
#ifndef ulong
#define ulong unsigned long
#endif

static uchar ucRet,ucLen, CmdStr[MAX_CMD_LEN], RespStr[MAX_RSP_LEN];
static uint  uiLen;
static uchar MatrixDot[245];

static uchar SendRecv(uchar *CmdStrInOut,uint SendLen, uchar *RespStrOut, uint *RecvLen,uint TimeOut);

extern uchar s_GetMatrixDot(uchar *str,uchar *MatrixDot,unsigned int *len, uchar flag);
//extern void ExPortClose(void);

unsigned char ExPortOpen(int channel, long BaudRate, char Parity, char DataBit)
{
	return(PortOpen(3, "9600,8,e,1"));
}
unsigned char ExPortRecv(unsigned char *ch, unsigned short ms)
{
	return(PortRecv(3, ch, ms));
}
void ExPortSend(unsigned char ch)
{
	PortSend(3, ch);
	return;
}

static uchar RecvResp(uchar Head, uchar *Resp, uint *RecvLen,uint TimeOut)
{
	uint i;
	uchar key;

	Resp[0]=0;
	while(1)
	{
		if(TimeOut)
		{
            if(!ExPortRecv(Resp,(ushort)TimeOut)) break;
            return COMM_ERR;
        }
		else
		{
			ExPortRecv(Resp,50);
			if(!kbhit())
			{
				key=getkey();
				if(key==KEYCANCEL) return USER_CANCEL;
				//if(key==KEYENTER) return NO_PIN;
			}
        }
		if(Resp[0]==Head || Resp[0] == Head -0x80) break;
	}
	if(ExPortRecv(Resp+1,100)) return COMM_ERR;
	for(i=0;i<(uint)Resp[1]+1;i++)
	{
		if(ExPortRecv(Resp+i+2,100)) return COMM_ERR;
	}
	*RecvLen = Resp[1]+3;
	return 0;
}

static void GetEDC(uchar *strInOut, uint len)
{
	uint i;

	strInOut[len]=strInOut[0];
	for(i=1;i<len;i++)
		strInOut[len] ^= strInOut[i];
}

uchar SendRecv(uchar *CmdStrInOut,uint SendLen, uchar *RespStrOut, uint *RecvLen,uint TimeOut)
{
    uchar EDC;
    uint i,j;

    if(ExPortOpen(0, 9600L, 'E', 8)) return COMM_ERR;
	GetEDC(CmdStrInOut,SendLen);
	for(j=0;j<3;j++)
	{
    	for(i=0;i<SendLen+1;i++) ExPortSend(CmdStrInOut[i]);
		ucRet=RecvResp(CmdStrInOut[0],RespStrOut,RecvLen,TimeOut);
		if (ucRet)
		{
			if(ucRet ==COMM_ERR)	continue;
			return ucRet;
		}
		EDC=RespStrOut[*RecvLen-1];
		GetEDC(RespStrOut,(uint)(*RecvLen-1));
		if(RespStrOut[*RecvLen-1] ==EDC)	break;
//		if(RespStr[0]==CmdStrInOut[0] || RespStrOut[2]!=0x01) break;
		DelayMs(10);
	}
	if(j >=3)return COMM_ERR;
    //ExPortClose();
	return 0;
}

uchar PPCancel(void)
{
	uchar ucRet;

	memcpy(CmdStr,"\x8e\x00",2);
	ucRet=SendRecv(CmdStr, 2, RespStr, &uiLen,800);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x8e)	return 0;
	return RespStr[2];
}
/*
uchar PPSetTimeout(uchar timeout)
{
	memcpy(CmdStr,"\x93\x02\x33\x00",4);
	CmdStr[3] =timeout;
	ucRet=SendRecv(CmdStr, 4, RespStr, &uiLen,300);
	if(ucRet) return ucRet;
	if(RespStr[0] == 0x93){
		return 0;
	}
	return RespStr[2];
}*/

uchar PPSetTimeout(unsigned short timeout)
{
	if(timeout==0)
		return 0xff;
	memcpy(CmdStr,"\x96\x04",2);
	CmdStr[2] =0;
	CmdStr[3] =0;
	CmdStr[5]	=(uchar)(timeout&0xff);
	CmdStr[4] =(uchar)(timeout>>8);
	ucRet=SendRecv(CmdStr, 6, RespStr, &uiLen,300);
	if(ucRet) return ucRet;
	if(RespStr[0] == 0x96){
		return 0;
	}
	return RespStr[2];
}
/*
uchar PPBaudRate(long Baudrate)
{
	memcpy(CmdStr,"\x93\x02\x34\x40",4);
	if(Baudrate==19200) CmdStr[3]=0x01;
	else if(Baudrate==28800) CmdStr[3]=0x02;
	else if(Baudrate==57600) CmdStr[3]=0x03;
	else return COMM_ERR;
	ucRet=SendRecv(CmdStr, 4, RespStr, &uiLen,300);
	if(ucRet) return ucRet;
	if(RespStr[0] == 0x93){
		BaudSet=Baudrate;
		return 0;
	}
	return RespStr[2];
}
*/

uchar PPBeep(void)
{
	memcpy(CmdStr,"\x8D\x02\x40\x40",4);
	ucRet=SendRecv(CmdStr, 4, RespStr, &uiLen,1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x8d)	return 0;
	return RespStr[2];
}

uchar PPLight(uchar OperMode, uchar Index)
{
	memcpy(CmdStr,"\x8D\x02\x00\x00",4);
	if((Index & PP_RED) == PP_RED)
		CmdStr[2] |= 0x08;
	if((Index & PP_GREEN) == PP_GREEN)
		CmdStr[2] |= 0x20;
	if((Index & PP_YELLOW) == PP_YELLOW)
		CmdStr[2] |= 0x10;
	if(OperMode == PP_OPEN)
	{
		if((Index & PP_RED) == PP_RED)
			CmdStr[3] |= 0x08;
		if((Index & PP_GREEN) == PP_GREEN)
			CmdStr[3] |= 0x20;
		if((Index & PP_YELLOW) == PP_YELLOW)
			CmdStr[3] |= 0x10;
	}
	ucRet=SendRecv(CmdStr, 4, RespStr, &uiLen,1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x8d)	return 0;
	return RespStr[2];
}

uchar PPInvoice(uchar OperMode)
{
	memcpy(CmdStr,"\x8D\x02\x02\x00",4);
	if(OperMode == PP_OPEN)
		CmdStr[3] |= 0x02;
	ucRet=SendRecv(CmdStr, 4, RespStr, &uiLen,1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x8d)	return 0;
	return RespStr[2];
}


uchar PPKbmute(uchar OperMode)
{
	memcpy(CmdStr,"\x8D\x02\x01\x00",4);
	if(OperMode == PP_OPEN)
		CmdStr[3] |= 0x01;
	ucRet=SendRecv(CmdStr, 4, RespStr, &uiLen,1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x8d)	return 0;
	return RespStr[2];
}

uchar PPBackLight(uchar OperMode)
{
	memcpy(CmdStr,"\x8D\x02\x80\x00",4);
	if(OperMode == PP_OPEN)
		CmdStr[3] |= 0x80;
	ucRet=SendRecv(CmdStr, 4, RespStr, &uiLen,1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x8d)	return 0;
	return RespStr[2];
}


uchar PPInput(uchar *strInOut, uchar min, uchar max, uchar mode)
{
	uchar ucRet;

	if(PPScrClrLine(20)==COMM_ERR) return COMM_ERR;
	CmdStr[0]=0x8C;
	CmdStr[1]=0x03;
	CmdStr[2]=min;
	CmdStr[3]=max;
	CmdStr[4]=mode;
	ucRet=SendRecv(CmdStr, 5, RespStr, &uiLen, 0);
	if(ucRet)
	{
		//if(ucRet==USER_CANCEL || ucRet==NO_PIN) PPCancel();
		if(ucRet==USER_CANCEL) PPCancel();
		return ucRet;
	}
	if(RespStr[0] == 0x8C)
	{
		memcpy(strInOut,RespStr+2,RespStr[1]);
		strInOut[RespStr[1]]=0;
		return 0;
	}
	return RespStr[2];
}

uchar PPScrPrint(uchar line, uchar col, const uchar *str)
{
	unsigned int j;
	uint  i, len,mode;
	uchar flag, bDotLen;
	uchar TmpBuf[33], sDotBuf[130];


	if(line>=2 || col>121) return 0x04;
	len=strlen((char *)str);
	if(len*8>122-(uint)col) len=(122-col)/8;
	mode=0;
	flag=0;
	for(i=0;i<len;i++)
	{
		if(str[i]>0x80)
		{
			 mode=1;
			 flag ^= 1;
		}
	}
	if(flag)	len--;
	if(!len)	return 0;
	if(mode)
	{
		bDotLen =0;
		memset(sDotBuf, 0, sizeof(sDotBuf));
		for(i=0;i<len;i++)
		{
			s_GetMatrixDot((uchar *)str+i,TmpBuf,&j,1);

			if (j<6||j>32)
			{
				if(str[i]>0x80)
				{
					memcpy(MatrixDot+i*8,TmpBuf,16);
					memcpy(MatrixDot+len*8+i*8,TmpBuf+16,16);
					i ++;
					bDotLen +=16;
				}
				else
				{
			 		memcpy(MatrixDot+i*8,TmpBuf,8);
			 		memcpy(MatrixDot+len*8+i*8,TmpBuf+8,8);
					bDotLen +=8;
				}
			}
			else
			{
				memcpy(MatrixDot+bDotLen, TmpBuf, j/2);
				memcpy(sDotBuf+bDotLen, TmpBuf+j/2, j/2);
				bDotLen +=j/2;

				if(str[i]>0x80)			i++;
				if (i==(len-1))
					memcpy(MatrixDot+bDotLen, sDotBuf, bDotLen);
			}
		}
		return PPScrWrData(line,col, MatrixDot, (uchar)(bDotLen*2));
	}
	else
	{
		ucLen=strlen((char *)str);
		CmdStr[0]=0x8A;
		CmdStr[1]=ucLen+2;
        CmdStr[2]=col;
        CmdStr[3]=line;
		memcpy(CmdStr+4,str,ucLen);
		ucRet=SendRecv(CmdStr, (uint)(ucLen+4), RespStr, &uiLen,(uint)1000);
		if(ucRet)				return ucRet;
		if(RespStr[0] == 0x8A)	return 0;
		return RespStr[2];
	}
}

uchar PPScrCls(void)
{
	memcpy(CmdStr,"\x88\x00",2);
	ucRet=SendRecv(CmdStr, 2, RespStr, &uiLen,1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x88)	return 0;
	return RespStr[2];
}

uchar PPScrClrLine(uchar Line)
{
	memcpy(CmdStr,"\x89\x01\0x00",3);
	CmdStr[2]=Line;
	ucRet=SendRecv(CmdStr, 3, RespStr, &uiLen,1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x89)	return 0;
	return RespStr[2];
}

uchar PPScrWrData(uchar line, uchar col, const uchar *Data, uchar len)
{
	if (len >=252)	return 1;
	CmdStr[0]=0x8B;
	CmdStr[1]=len+2;
    CmdStr[2]=col;
    CmdStr[3]=line;
    memcpy(CmdStr+4,Data,len);
	ucRet=SendRecv(CmdStr, (uint)(len+4), RespStr, &uiLen,(uint)1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x8b)	return 0;
	return RespStr[2];
}


static uchar WriteMDKey(uchar KeyID, uchar mode, const uchar *Key)
{
	CmdStr[0]=0x80;
	if(mode==0x01)					CmdStr[1]=10;
	else if(mode==0x03)				CmdStr[1]=18;
	else 							CmdStr[1]=26;
	CmdStr[2]=KeyID;
	CmdStr[3]=mode;
	memcpy(CmdStr+4,Key,CmdStr[1]-2);
	ucRet=SendRecv(CmdStr, (uint)(CmdStr[1]+2), RespStr, &uiLen,(uint)1000);
	if(ucRet) return ucRet;
	if(RespStr[0] ==0x80)	return 0;
	return RespStr[2];
}

uchar PPWriteMKey(uchar KeyID, uchar mode, const uchar *Key)
{
	if(KeyID>100||KeyID==0)
		return 0x02;
	else
		return WriteMDKey(KeyID,mode,Key);
}

uchar PPWriteDKey(uchar KeyID, uchar mode, const uchar *Key)
{
	if(KeyID>100||KeyID==0)
		return 0x02;
	else
		return WriteMDKey((uchar)(KeyID | 0x80),mode,Key);
}

uchar PPWriteWKey(uchar MKeyID, uchar WKeyID, uchar mode, const uchar *Key)
{
	uchar TmpLen;
	CmdStr[0]=0x81;
	switch (mode)
	{
	case 0x01:
	case 0x81:
	case 0x03:
	case 0x83:
	case 0x07:
	case 0x87:
	case 0x31:
	case 0xb1:	
	case 0x71:
	case 0xf1:	CmdStr[1]=11;	TmpLen=8;	break;
	case 0x33:
	case 0xb3:	
	case 0x73:
	case 0xf3:	CmdStr[1]=19;	TmpLen=16;	break;
	default:	CmdStr[1]=27;	TmpLen=24;	break;
	}
	CmdStr[2]=MKeyID;
	CmdStr[3]=WKeyID;
	CmdStr[4]=mode;
	memcpy(CmdStr+5,Key,TmpLen);
	ucRet=SendRecv(CmdStr, (uint)(CmdStr[1]+2), RespStr, &uiLen,1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x81)	return 0;
	return RespStr[2];
}

uchar PPDeriveKey(uchar MKeyID, uchar WKeyID1, uchar WKeyID2, uchar mode)
{
	CmdStr[0]=0x82;
	CmdStr[1]=4;
	CmdStr[2]=MKeyID;
	CmdStr[3]=WKeyID1;
	CmdStr[4]=WKeyID2;
	CmdStr[5]=mode;
	ucRet=SendRecv(CmdStr, 6, RespStr, &uiLen,1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x82)	return 0;
	return RespStr[2];
}

uchar PPGetPwd(uchar PinKeyID, uchar min, uchar max, uchar *cardno, uchar *pin, uchar TwiceInput)
{
	uchar ucRet;

	if(PPScrClrLine(20)==COMM_ERR) return COMM_ERR;
	if(!TwiceInput) CmdStr[0] = 0x83;
	else			CmdStr[0] = 0x84;
	CmdStr[1] = 19;
	CmdStr[2] = PinKeyID;
	CmdStr[3] = min;
	CmdStr[4] = max;
	memcpy(CmdStr+5,cardno,16);
	ucRet=SendRecv(CmdStr, 21, RespStr, &uiLen, 0);
	if(ucRet)
	{
		//if(ucRet==USER_CANCEL || ucRet==NO_PIN) PPCancel();
		if(ucRet==USER_CANCEL) PPCancel();
		return ucRet;
	}
	if( (!TwiceInput && RespStr[0]==0x83) || (TwiceInput && RespStr[0]==0x84) )
	{
		if(RespStr[1] == 0x00) return NO_PIN;
		memcpy(pin,RespStr+2,8);
		return 0;
	}
	return RespStr[2];
}

uchar Ex_PPGetPwd(uchar PinKeyID, uchar min, uchar max, uchar *cardno, uchar *pin, uchar TwiceInput, uchar mode)
{
	uchar ucRet;
	uchar i;
	if(PPScrClrLine(20)==COMM_ERR) return COMM_ERR;
	if(mode==0x31||mode==0x71||mode==0x02)
		return 0x03;
	if(!TwiceInput) CmdStr[0] = 0x83;
	else			CmdStr[0] = 0x84;
	//if(max>0x06)
	//	return 0x05;
	//if(strlen(cardno)!=16)
	//	return 0x05;
	for(i=0;i<16;i++)
		{
			if(cardno[i]>0x39||cardno[i]<0x30)
			 cardno[i]=0x0;
		}
	CmdStr[1] = 22;
	CmdStr[2]	= 0xff;
	CmdStr[3]	=0xff;
	
	if(mode==0x02)
		return 0x03;
	if(mode==0x01)	CmdStr[4]=0x01;
	else if(mode==0x03)	CmdStr[4]=0x31;
	else if(mode==0x07)	CmdStr[4]=0x71;
	else CmdStr[4]=mode;
	CmdStr[5] = PinKeyID;
	CmdStr[6] = min;
	CmdStr[7] = max;
	memcpy(CmdStr+8,cardno,16);
	ucRet=SendRecv(CmdStr, 24, RespStr, &uiLen, 0);
	if(ucRet)
	{
		//if(ucRet==USER_CANCEL || ucRet==NO_PIN) PPCancel();
		if(ucRet==USER_CANCEL) PPCancel();
		return ucRet;
	}
	if( (!TwiceInput && RespStr[0]==0x83) || (TwiceInput && RespStr[0]==0x84) )
	{
		if(RespStr[1] == 0x00) return NO_PIN;
		memcpy(pin,RespStr+2,8);
		return 0;
	}
	return RespStr[2];
}


uchar PPDes(uchar DESKeyID, uchar mode, const uchar *datain, uchar *dataout)
{
	CmdStr[0] = 0x87;
	CmdStr[1] = 10;
	CmdStr[2] = DESKeyID;
	CmdStr[3] = mode;
	memcpy(CmdStr+4,datain,8);
	ucRet=SendRecv(CmdStr, 12, RespStr, &uiLen, 1000);
	if(ucRet) return ucRet;
	if(RespStr[0] == 0x87)
	{
		memcpy(dataout,RespStr+2,8);
		return 0;
	}
	return RespStr[2];
}

uchar PPMac(uchar WKeyID,uchar mode, const uchar *datain, ushort inLen, uchar *macout,uchar flag)
{
	unsigned char i,j,temp, bBlockNum, bSendLen, bLastLen, bBlockSize;
	unsigned char temp1[8];
	unsigned short nCount;
	unsigned long TimeOut;
	unsigned int k,l;

	if (WKeyID >100)	return 2;
	if(PPScrClrLine(20)==COMM_ERR) return COMM_ERR;
	if(flag==0x00)	CmdStr[0] = 0x85;
	else		CmdStr[0] = 0x86;
	if(flag==0x00)
	{
			if (inLen >=252)
			{
				bBlockNum =inLen/240;
				bLastLen =inLen%240;
				if (bLastLen >0)	bBlockNum ++;
				else				bLastLen =240;
				bBlockSize =240;
			}
			else
			{
				bBlockNum =1;
				bBlockSize = (uchar)inLen;
			}
			TimeOut=1000;
			if(flag==0)
			{
				TimeOut +=bBlockSize*3;
				if(mode==3)		TimeOut +=bBlockSize*5;
				if(mode==7)		TimeOut +=bBlockSize*7;
			}
			CmdStr[1] = bBlockSize+2;
			CmdStr[2] = WKeyID;
			CmdStr[3] = mode;
			memcpy(CmdStr+4,datain,bBlockSize);
			bSendLen =bBlockSize;
			nCount =0;
			for(i=0; i<bBlockNum; i++)
			{
				ucRet=SendRecv(CmdStr, (uint)(bSendLen+4), RespStr, &uiLen, (uint)TimeOut);
				if(ucRet)	return ucRet;
				if(RespStr[0] !=CmdStr[0])		return RespStr[2];

				if (i ==(bBlockNum-1))			break;
				if (i ==(bBlockNum-2))			bSendLen =bLastLen;
				else							bSendLen =bBlockSize;
				memset(temp1,0x0,8);
				nCount +=bBlockSize;				
				if((0<bSendLen)&&(bSendLen<8))
					{
						temp=bSendLen%240;
						for(j=0;j<temp;j++)
						temp1[j]=datain[nCount+j]^RespStr[2+j];
						for(j=0;j<8-temp;j++)
						temp1[j+temp]=RespStr[2+j+temp];
						bSendLen+=(8-temp);
						memcpy(CmdStr+4, temp1, 8);	
					}
				else
					{
						for(j=0;j<8;j++)
						temp1[j]=datain[nCount+j]^RespStr[2+j];
						memcpy(CmdStr+4, temp1, 8);	
						memcpy(CmdStr+12, &datain[nCount+8], bSendLen-8);
					}
				CmdStr[1] = bSendLen+2;
				CmdStr[2] = WKeyID;
			}
	}
	else
	{
		// Added by Kim_LinHB 2014-8-5 v1.1.0001
		uchar sTempBuff[LEN_MAX_COMM_DATA+10];
		memset(sTempBuff, 0, sizeof(sTempBuff));
		memcpy(sTempBuff, datain, inLen);
		// Add End

		memset(temp1,0x0,8);
		k=inLen/8;
		temp=inLen%8;
		if(temp)
			{
				memset(&sTempBuff[inLen],0x00,8-temp);
				inLen+=temp;
				k++;
			}
		TimeOut=1000;
		for(l=0;l<k;l++)
		{
				for(j=0;j<8;j++)
				temp1[j] ^= sTempBuff[j+8*l];
		}
		memcpy(CmdStr+4, temp1, 8);
		CmdStr[1] = 10;
		CmdStr[2] = WKeyID;
		CmdStr[3] = mode;
		ucRet=SendRecv(CmdStr, 12, RespStr, &uiLen, (uint)TimeOut);
				if(ucRet)	return ucRet;
	}
	if(RespStr[0] ==CmdStr[0])
	{
		memcpy(macout,RespStr+2,8);
		return 0;
	}
	return RespStr[2];
}

uchar PPVerInfo(uchar *ver)
{
	memcpy(CmdStr,"\x90\x00",2);
	ucRet=SendRecv(CmdStr, 2, RespStr, &uiLen,800);
	if(ucRet) return ucRet;
	if(RespStr[0] == 0x90)
	{
		memcpy(ver,RespStr+2,8);
		return 0;
	}
	return RespStr[2];
}

uchar PPUpdLogo(uchar line, const uchar *Data)
{
	if(PPScrClrLine(20)==COMM_ERR) return COMM_ERR;
	memcpy(CmdStr,"\x8f\xf5",2);
	CmdStr[2]=line;
	memcpy(CmdStr+3,Data,0xf4);
	ucRet=SendRecv(CmdStr,0xf7,RespStr,&uiLen,1000);
	if(ucRet)				return ucRet;
	if(RespStr[0] == 0x8f)	return 0;
	return RespStr[2];
}

uchar PPEmvGetPwd(uchar min, uchar max,uchar *pin_block)
{
	uchar ucRet;
	uchar i,tempWkey[8];
	uchar cardno[16];
	uchar En_pinblock[8];
	uchar Clear_EmvPin[8];
	
	if(min<4|| max>12) 
			return 0x05;
	memset(cardno,0x0,16);
	for(i=0;i<8;i++)
		tempWkey[i]=rand()&0xff;
	PPWriteWKey(0,100,0x01,tempWkey);	
	ucRet=PPGetPwd(100, min, max,cardno,En_pinblock, 0);
	des(En_pinblock, Clear_EmvPin,tempWkey, 0);
	Clear_EmvPin[0]|=0x20;
	memcpy(pin_block,Clear_EmvPin,8);
	return ucRet;
	
}

uchar PPGetPwd_3Des(uchar PinKeyID, uchar mode,uchar min, uchar max, uchar *cardno, uchar *pin, uchar TwiceInput)
{
	uchar ucRet;
	
	if(PPScrClrLine(20)==COMM_ERR) 
		return COMM_ERR;
	if(mode==0x03||mode==0x71||mode==0x02)
		return 0x03;
	if(!TwiceInput) CmdStr[0] = 0x83;
	else			CmdStr[0] = 0x84;
	CmdStr[1] = 22;
	CmdStr[2] = 0xff;
	CmdStr[3] = 0xff;
	CmdStr[4] = mode;	
	CmdStr[5] = PinKeyID;
	CmdStr[6] = min;
	CmdStr[7] = max;
	memcpy(CmdStr+8,cardno,16);
	ucRet=SendRecv(CmdStr, 24, RespStr, &uiLen, 0);
	if(ucRet)
	{
		//if(ucRet==USER_CANCEL || ucRet==NO_PIN) PPCancel();
		if(ucRet==USER_CANCEL) PPCancel();
		return ucRet;
	}
	if( (!TwiceInput && RespStr[0]==0x83) || (TwiceInput && RespStr[0]==0x84) )
	{
		if(RespStr[1] == 0x00) 
			return NO_PIN;
		memcpy(pin,RespStr+2,8);
		return 0;
	}
	return RespStr[2];
}

uchar PPQuickBeepTest(void)
{

	//memcpy(CmdStr,"\x8D\x02\x40\x40",4);
	//ucRet=SendRecv(CmdStr, 4, RespStr, &uiLen,300); //ddq
	//if(ucRet)				return ucRet;
	//if(RespStr[0] == 0x8d)	return 0;
	//return RespStr[2];

	return 0;	
}


