
#ifndef _HTTP_API_H_
#define _HTTP_API_H_


#define PROTO_HTTP      1
#define PROTO_HTTPS 2

#define DEBUG_PRI_CHAR  1
#define DEBUG_PRI_HEX   2

// HTTP Return codes
#define HTTP_SUCCESS                 0 // HTTP Success status


#define ERR_HTTP_BASE               200
#define ERR_HTTP_MEM                -(ERR_HTTP_BASE+1)
#define ERR_HTTP_HANDLE         -(ERR_HTTP_BASE+2)
#define ERR_HTTP_BAD_URL            -(ERR_HTTP_BASE+3) // Could not parse curtail elements from the URL (such as the host name, HTTP prefix act')
#define ERR_HTTP_GET_IP             -(ERR_HTTP_BASE+4)
#define ERR_HTTP_SEND               -(ERR_HTTP_BASE+5)
#define ERR_HTTP_NO_EXIST           -(ERR_HTTP_BASE+6)
#define ERR_HTTP_BAD_HEAD           -(ERR_HTTP_BASE+7)
#define ERR_HTTP_NO_CONTENT     -(ERR_HTTP_BASE+8)
#define ERR_HTTP_ILLEGAL_PARA       -(ERR_HTTP_BASE+9)
#define ERR_HTTP_NO_CMD         -(ERR_HTTP_BASE+10)
#define ERR_HTTP_BAD_REDIRECT       -(ERR_HTTP_BASE+11)
#define ERR_HTTP_CONTENT            -(ERR_HTTP_BASE+12)//Content is not receiving
#define ERR_HTTP_BP_REQ_DIFF            -(ERR_HTTP_BASE+13)
#define ERR_HTTP_CONNECT_FAIL       -(ERR_HTTP_BASE+14)
#define ERR_HTTPS_CONNECT_FAIL      -(ERR_HTTP_BASE+15)


#define HTTP_RESET_CERT_REASON  0x01
#define HTTP_RESET_CERT_CRL     0x02
#define HTTP_RESET_PRI_KEY          0x04
#define HTTP_RESET_LOCAL_CERT       0x08
#define HTTP_RESET_SER_CA_CERT  0x10
#define HTTP_RESET_SSL_PARA     0xFF

// Add only when added at the end to prevent the impact of previous commands
enum
{
    HTTP_CMD_SET_DEBUG  = 1,
    HTTP_CMD_GET_PROTO,
    HTTP_CMD_GET_HOST,
    HTTP_CMD_GET_PORT,
    HTTP_CMD_GET_URL_REQ,
    HTTP_CMD_GET_CONNECT_STATUS,

    HTTP_CMD_SET_PROTO = 0x32,
    HTTP_CMD_SET_HOST,
    HTTP_CMD_SET_PORT,
    HTTP_CMD_SET_URL_REQ,

    HTTP_CMD_SET_CONTENT_TYPE,
    HTTP_CMD_SET_USER_AGENT,

    HTTP_CMD_SET_TIME_ZONE,
    HTTP_CMD_SET_CERT_REASON,
    HTTP_CMD_SET_CERT_CRL,
    HTTP_CMD_SET_PRI_KEY,
    HTTP_CMD_SET_LOCAL_CERT,
    HTTP_CMD_SET_SER_CA_CERT,

    HTTP_CMD_SET_TIMEOUT,

    HTTP_CMD_RESET_CERT_PARA,

    HTTP_CMD_SET_BREAKPOINT_TRAN_LEN,
    HTTP_CMD_SET_BREAKPOINT_START,
    HTTP_CMD_SET_ACCEPT,
    HTTP_CMD_SET_ACCEPT_ENCODING,

    HTTP_CMD_SET_ACCEPT_LANGUAGE,
    HTTP_CMD_SET_CACHE_CONTROL,

    HTTP_SYS_SET_HTTPGET_NONBLOCK = 0X64,
    HTTP_CMD_ADD_NEW_FIELDS = 0x80,

    HTTP_CMD_SET_TLSEXT_SNI,
};

typedef struct http_sys_ops
{
    int (*UserCallBackF)(void *arg);
} HTTP_SYS_OPS;



//void HTTPGetVer(char *str);
int HttpCreate(void);
int HttpParaCtl(int s, int cmd, char *arg, int len);
int HttpGet(int s, char *pUrlStr);
int HttpPost(int s, char *pUrlStr, char *pData, int DataLen);
int HttpRecvContent(int s, char *pBuf, int len);
int HttpClose(int s);
int HttpEscapeEncode(unsigned char *BufIn, int InSize, unsigned char *BufOut, int OutSize);
int HttpSetSysOps(HTTP_SYS_OPS *ops);
int HttpGetHead(char *pBuf, int len);


#endif


