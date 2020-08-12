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
// 判断当前应用的运行模式：独立运行/主子应用/从子应用
// 返回：0－－不需要保存状态（状态无变化）；1－－需要保存（状态有变）
int InitMultiAppInfo(void)
{
	return 0;
}

// 加载管理器传给子应用的参数同步文件(当应用管理器存在，子应用运行在非独立模式时)
// 不适用于主要子应用
// load config files received from manager application in sub-application 
//int LoadSyncParaFile(void)
//{
//	return 0;
//}

// 处理从管理器传递进来的信息（或解析管理器传给子应用的参数同步文件）
// 如果ucCheckBuffSize!=0, 则检查pszRetStr必须带有“BUFFER=xxxx;”的标记
// process message received from manager
int ProcManagerAdminMsg(void)
{
	return 0;
}

// 从应用间通信字串中提取"TAG1=VALUE1;TAG2=VALUE2"的元素
// 语法：第一个"="之前为tag，"="之后第一个";"之前为value
// 无"="代表只有tag没有value
// 空格及任何非语法指定符号均被当作tag或value本身的一部分
// 建议tag和value应该只由可见的ASCII字符组成，但不应含有关键符号"="或者";"
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

// 处理从管理器收到的、已解包的tag和value
// 不在此函数内进行保存文件动作
// precess the unpacked tag-value data
static int ProcMsgElement(char *pszTag, char *pszValue)
{
	// 这里处理管理器传给本应用的消息
	// 一般来说，大多数参数都是在主要子应用里设置，然后回传给管理器，再由管理器传给其它子应用
	// 本应用是主要子应用，设置参数并同步到其它子应用，而不被动接收其它子应用的同步
	return 1;
}

// 把"tag=value"插入到已有的buffer里面。如果buffer本身已有相同tag的字符串，则替换
// 本函数会检查待插入tag和value长度，以及保证插入后不引致缓冲溢出。
// 本函数不校验原有字串的正确性
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

	// 检查输入tag和value
	// check tag and value 
	if ((pszTag==NULL)|| (strlen(pszTag)>LEN_APPMSG_TAG))
	{
		return ERR_APPMSG_TAGLEN;
	}
	if ((pszValue!=NULL) && strlen(pszValue)>LEN_APPMSG_VALUE)
	{
		return ERR_APPMSG_VALUE;
	}
	// 构造待插入的字串
	// prepare the string format 
	strcpy(szNewStr, pszTag);
	if (pszValue!=NULL)
	{
		strcat(szNewStr, "=");
		strcat(szNewStr, pszValue);
	}
	strcat(szNewStr, ";");
	iInsLen = strlen(szNewStr);

	// 搜索，若存在则替换，否则连接到尾部
	// do search, if replace the element if it is in the string, or concatenate to the tail
	pszTemp = pszDest;
	while (1)
	{
		iRet = GetMsgElement(pszTemp, szTempTag, szTempVal, &iLen);
		if (iRet==0)
		{
			if (strcmp(szTempTag, pszTag)==0)	// 如果存在 if existed
			{
				if (strlen(pszDest)-iLen+iInsLen>SIZE_APPMSG_BUFF)
				{
					// 替换后不能超出buffer size
					// size limit
					return ERR_APPMSG_BUFF;
				}
			
				// 则替换
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
		// 连接新字符串后不能超出buffer size
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

// 向manager报告本应用中已修改、并需要通知其它子应用或者通知管理器的变量。
// 必须保证pvBuff的空间足够
// report to manager, update other application or manager
// buffer for pvBuff should >= SIZE_APPMSG_BUFF
int ProcSendAppMsg(void *pvBuff, int iBuffSize, uchar ucUpdateAll)
{
	
	return 0;
}

// end of file

