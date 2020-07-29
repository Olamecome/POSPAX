#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static int GetMsgElement(const char *pszSource, char *pszTag, char *pszValue, int *piLen);
static int SetMsgElement(char *pszDest, const char *pszTag, const char *pszValue);
static int ProcMsgElement(char *pszTag, char *pszValue);
static int PackManagerMsg(const char *pszTag, const void *pszValue, int iValueType);

/********************** Internal variables declaration *********************/
char	sg_szMsgBuff[SIZE_APPMSG_BUFF+2];

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// To determine the running mode of current application.
// return: 0--No need save; 1--Need save
// �жϵ�ǰӦ�õ�����ģʽ����������/����Ӧ��/����Ӧ��
// ���أ�0��������Ҫ����״̬��״̬�ޱ仯����1������Ҫ���棨״̬�б䣩
int InitMultiAppInfo(void)
{
	return 0;
}

// ���ع�����������Ӧ�õĲ���ͬ���ļ�(��Ӧ�ù��������ڣ���Ӧ�������ڷǶ���ģʽʱ)
// ����������Ҫ��Ӧ��
// load config files received from manager application in sub-application 
//int LoadSyncParaFile(void)
//{
//	return 0;
//}

// ����ӹ��������ݽ�������Ϣ�������������������Ӧ�õĲ���ͬ���ļ���
// ���ucCheckBuffSize!=0, ����pszRetStr������С�BUFFER=xxxx;���ı��
// process message received from manager
int ProcManagerAdminMsg(void)
{
	return 0;
}

// ��Ӧ�ü�ͨ���ִ�����ȡ"TAG1=VALUE1;TAG2=VALUE2"��Ԫ��
// �﷨����һ��"="֮ǰΪtag��"="֮���һ��";"֮ǰΪvalue
// ��"="����ֻ��tagû��value
// �ո��κη��﷨ָ�����ž�������tag��value�����һ����
// ����tag��valueӦ��ֻ�ɿɼ���ASCII�ַ���ɣ�����Ӧ���йؼ�����"="����";"
// got Tag-Value data from a string,
// rule: "TAG=VALUE", split by ";", if there is just a Tag with "=VALUE", then it means this tag is reserved,
// upper limit of tag length is 16
// upper limit of value length is 128
// except "=" and ";", all characters will be read as a part of tag or value.
// e.g. "Width=20;Height=32;Depth;"
static int GetMsgElement(const char *pszSource, char *pszTag, char *pszValue, int *piLen)
{
	char *pStart, *pEqual, *pEnd;

	if (strlen(pszSource)==0)
	{
		return ERR_APPMSG_END;
	}

	pStart = (uchar *)pszSource;

	pEnd = strstr(pStart, ";");
	if (pEnd==NULL)// no ";" in the string
	{
		pEnd = pStart+strlen(pStart);
		*piLen = pEnd-pStart;
	}
	else
	{
		*piLen = pEnd-pStart+1;
	}

	pEqual = strstr(pStart, "=");
	if ((pEqual==NULL) || (pEqual>pEnd))// case: tag without value
	{
		if (pEnd-pStart>LEN_APPMSG_TAG)
		{
			return ERR_APPMSG_TAGLEN;
		}
		sprintf(pszTag, "%.*s", pEnd-pStart, pStart);
		pszValue[0] = 0;
	}
	else
	{
		if (pEqual-pStart>LEN_APPMSG_TAG)
		{
			return ERR_APPMSG_TAGLEN;
		}
		if (pEnd-pEqual>LEN_APPMSG_VALUE)
		{
			return ERR_APPMSG_VALUE;
		}
		sprintf(pszTag,   "%.*s", pEqual-pStart, pStart);
		sprintf(pszValue, "%.*s", pEnd-pEqual-1, pEqual+1);
	}

	return 0;
}

// ����ӹ������յ��ġ��ѽ����tag��value
// ���ڴ˺����ڽ��б����ļ�����
// precess the unpacked tag-value data
static int ProcMsgElement(char *pszTag, char *pszValue)
{
	// ���ﴦ�������������Ӧ�õ���Ϣ
	// һ����˵�������������������Ҫ��Ӧ�������ã�Ȼ��ش��������������ɹ���������������Ӧ��
	// ��Ӧ������Ҫ��Ӧ�ã����ò�����ͬ����������Ӧ�ã�������������������Ӧ�õ�ͬ��
	return 1;
}

// ��"tag=value"���뵽���е�buffer���档���buffer����������ͬtag���ַ��������滻
// ���������������tag��value���ȣ��Լ���֤��������»��������
// ��������У��ԭ���ִ�����ȷ��
// insert a tag-value data into a buffer, will replace the existed tag
// will check the length of input data to avoid memory overflow.
// will not verify the target
static int SetMsgElement(char *pszDest, const char *pszTag, const char *pszValue)
{
	int		iLen, iInsLen, iRet;
	char	*pszTemp;
	char	szTempTag[LEN_APPMSG_TAG+1], szTempVal[LEN_APPMSG_VALUE+1];
	char	szNewStr[LEN_APPMSG_TAG+1+LEN_APPMSG_VALUE+1+1];

	if (pszDest==NULL)
	{
		return 0;
	}

	// �������tag��value
	// check tag and value 
	if ((pszTag==NULL)|| (strlen(pszTag)>LEN_APPMSG_TAG))
	{
		return ERR_APPMSG_TAGLEN;
	}
	if ((pszValue!=NULL) && strlen(pszValue)>LEN_APPMSG_VALUE)
	{
		return ERR_APPMSG_VALUE;
	}
	// �����������ִ�
	// prepare the string format 
	strcpy(szNewStr, pszTag);
	if (pszValue!=NULL)
	{
		strcat(szNewStr, "=");
		strcat(szNewStr, pszValue);
	}
	strcat(szNewStr, ";");
	iInsLen = strlen(szNewStr);

	// ���������������滻���������ӵ�β��
	// do search, if replace the element if it is in the string, or concatenate to the tail
	pszTemp = pszDest;
	while (1)
	{
		iRet = GetMsgElement(pszTemp, szTempTag, szTempVal, &iLen);
		if (iRet==0)
		{
			if (strcmp(szTempTag, pszTag)==0)	// ������� if existed
			{
				if (strlen(pszDest)-iLen+iInsLen>SIZE_APPMSG_BUFF)
				{
					// �滻���ܳ���buffer size
					// size limit
					return ERR_APPMSG_BUFF;
				}
			
				// ���滻
				// replacement case
				memmove(pszTemp+iInsLen, pszTemp+iLen, strlen(pszTemp+iLen)+1);
				memcpy(pszTemp, szNewStr, iInsLen);
				return 0;
			}
			else
			{
				pszTemp += iLen;
				continue;
			}
		}
		else if (iRet==ERR_APPMSG_END)
		{
			break;
		}
		else
		{
			return iRet;
		}
	}

	if (strlen(pszDest)+1+iInsLen+1>SIZE_APPMSG_BUFF)
	{
		// �������ַ������ܳ���buffer size
		// size limit
		return ERR_APPMSG_BUFF;
	}

	if ((strlen(pszDest)!=0) && (pszDest[strlen(pszDest)-1]!=';'))
	{
		strcat(pszDest, ";");
	}
	strcat(pszDest, szNewStr);
	return 0;
}

static int PackManagerMsg(const char *pszTag, const void *pszValue, int iValueType)
{
	char	szBuff[LEN_APPMSG_VALUE+2];

	switch(iValueType)
	{
	case MMSG_TYPE_STRING:
		sprintf(szBuff, "%.*s", LEN_APPMSG_VALUE, (char *)pszValue);
		break;
	case MMSG_TYPE_UCHAR:
		sprintf(szBuff, "%d", *(uchar *)pszValue);
		break;
	case MMSG_TYPE_INT:
		sprintf(szBuff, "%d", *(int *)pszValue);
		break;
	case MMSG_TYPE_ULONG:
		sprintf(szBuff, "%lu", *(ulong *)pszValue);
	    break;
	default:
	    return ERR_APPMSG_VALUE;
	}

	return SetMsgElement(sg_szMsgBuff, pszTag, szBuff);
}

// ��manager���汾Ӧ�������޸ġ�����Ҫ֪ͨ������Ӧ�û���֪ͨ�������ı�����
// ���뱣֤pvBuff�Ŀռ��㹻
// report to manager, update other application or manager
// buffer for pvBuff should >= SIZE_APPMSG_BUFF
int ProcSendAppMsg(void *pvBuff, int iBuffSize, uchar ucUpdateAll)
{
	
	return 0;
}

// end of file

