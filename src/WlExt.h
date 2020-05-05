#ifndef __WLEXT_H__
#define __WLEXT_H__

#define WNET_GPRS_Q24  				0x01
#define WNET_GPRS_G24				0x02
#define WNET_CDMA_MG815				0x03
#define WNET_GPRS_GTM900			0x04
#define WNET_CDMA_CM300				0x05
#define WNET_WCDMA_TELIT			0x06
#define WNET_GPRS_ENFORA			0x07                      
#define WNET_CDMA_EM200				0x08

#define WNET_GPRS_Q2687				0x0a
#define WNET_CDMA_Q26EL				0x0b
#define WNET_CDMA_MC323				0x0c
#define WNET_GPRS_MG323				0x0d
#define WNET_WCDMA_MU509			0x0e
#define WNET_CDMA_MC509				0x0f
#define WNET_GPRS_G620				0x10
#define WNET_GPRS_G610				0x11 
#define WNET_WCDMA_H330				0x12
#define WNET_GPRS_G510				0x13 
#define WNET_GPRS_G535				0x14
#define WNET_WCDMA_H350             0x15
#define WNET_WCDMA_MU709			0x16
#define WNET_4G_ME909 			0X17

#define REG_LOCATION    1   // locaion network
#define REG_ROAMING     5   //roaming network
#define TA_NUM_LEN      2
#define IMSI_NUM_LEN    15  //the max len of imsi
#define PHONE_NUM_LEN   16  //the max len of phone num.
#define CCID_NUM_LEN    20  //the max len of ccid
#define ISP_NAME_LEN    31  //the max len of isp name
#define MAX_CELL_NUM    7   //the max cell number
#define FACTORY_SN_LEN  31
#define CTR_R   '\r'
#define CTR_N   '\n'


#define MCC_LEN     3
#define MNC_LEN		3
#define LAC_LEN		6
#define CELL_LEN	9
#define BSIC_LEN    5
#define CHANN_LEN	5
#define RXLEV_LEN	5

#define SIC_LEN		5
#define NIC_LEN		5


#define ATCMD_SUCCESS		0

#define ERR_ATCMD_BASE          3000
#define ERR_ATCMD_IMSI			(ERR_ATCMD_BASE+1)
#define ERR_ATCMD_CCID			(ERR_ATCMD_BASE+2)
#define ERR_ATCMD_MODULEINFO	(ERR_ATCMD_BASE+3)
#define ERR_ATCMD_RSSI			(ERR_ATCMD_BASE+4)
#define ERR_ATCMD_ISP           (ERR_ATCMD_BASE+5)
#define ERR_ATCMD_CELLINFO		(ERR_ATCMD_BASE+6)
#define ERR_ATCMD_PARA			(ERR_ATCMD_BASE+7)
#define ERR_ATCMD_NOTSUPPORT	(ERR_ATCMD_BASE+8)
#define ERR_ATCMD_SETNUMBER	    (ERR_ATCMD_BASE+9)
#define ERR_ATCMD_LOCALNUM		(ERR_ATCMD_BASE+10)
#define ERR_ATCMD_NOTREGISTER	(ERR_ATCMD_BASE+11)
#define ERR_ATCMD_TA            (ERR_ATCMD_BASE+12)
#define ERR_ATCMD_SIMUNVALID	(ERR_ATCMD_BASE+13)
#define ERR_ATCMD_NETMODE       (ERR_ATCMD_BASE+14)
#define ERR_NOT_SUPPORT_LOGIN   (ERR_ATCMD_BASE+15)//在ppplogin状态不支持，请在logout下调用

/*for NETMODE*/
#define NETMODE_2G 0X02
#define NETMODE_3G 0X03
#define NETMODE_4G 0X04

#define IMSI_KEY			0X01 //IMSI value
#define CCID_KEY			0X02 //CCID value
#define MODULEINFO_KEY	    0X04 //module information
#define RSSI_KEY			0X08 //RSSI value
#define ISP_KEY				0X10 //ISP name
#define CELLINFO_KEY		0X20 // main cell and neighbor cell information
#define GETNUMBER_KEY	    0X40
#define TA_KEY				0x80
#define NETMODE_KEY         0X100 //Returns the current network selection mode,information about the operator with which the module is registered

//only for input: WlUsePaxBaseSo(unsigned char select)
enum 
{
    //use wireless extension library in paxbase***.so
	USE_PAX_BASESO = 0,
	//complie this wlext.c into app,or use libwlext**.a(static lib)
	NONUSE_PAX_BASESO = 1, 
};

typedef struct WlGSMCellInfo
{
	char mcc[MCC_LEN+1];//mobile country code 16进制ASC码字符串
	char mnc[MNC_LEN+1];//moblie network code 16进制ASC码字符串
	char lac[LAC_LEN+1];//location code 10进制ASC码字符串
	char cell[CELL_LEN+1];//cell code 10进制ASC码字符串
	char bsic[BSIC_LEN+1];//base station identifier code 10进制ASC码字符串
	char chann[CHANN_LEN+1];//absolute Frequency channel number 10进制ASC码字符串
	char rxlev[RXLEV_LEN+1];//receive level 10进制ASC码字符串
	char reserver[32]; //resever for future use.
}WlGSMCellInfo_T;

typedef struct WlCDMACellInfo
{
	char mcc[MCC_LEN+1];//mobile country code 16进制ASC码字符串
	char mnc[MNC_LEN+1];//moblie network code 16进制ASC码字符串
	char sic[SIC_LEN+1];//system identify code 10进制ASC码字符串
	char nic[NIC_LEN+1];//network identify code 10进制ASC码字符串
	char bsic[BSIC_LEN+1];//base station identifier code 10进制ASC码字符串
	char chann[CHANN_LEN+1];//absolute Frequency channel number  10进制ASC码字符串
	char rxlev[RXLEV_LEN+1];//receive level 10进制ASC码字符串
	char reserver[32]; //resever for future use.
}WlCDMACellInfo_T;

typedef struct WlInfo
{
	char imsi[IMSI_NUM_LEN+1];//the imsi number of sim card
	char ccid[CCID_NUM_LEN+1];//the ccid of sim card		
	char isp_name[ISP_NAME_LEN + 1];//CHINA MOBLE, etc
	char phone_num[PHONE_NUM_LEN + 1];
	char module_name[64]; //MG323,etc
	char manufacture[64]; //HUAWEI,etc
	char revision[64]; //11.810.01.00.00 ,etc
	char rssi;//31, etc
	char ta[TA_NUM_LEN+1];
	int   net_status;//ppplogout状态下，返回当前模块注册状态(CGREG?).注册上网返回1；返回0，2，3，4表示未注册原因; 0表示无SIM卡

	union{
		WlGSMCellInfo_T gsm[MAX_CELL_NUM];
		WlCDMACellInfo_T cdma;
	}CellInfo;
	char f_more_CellInfo;//more CellInfo can be read,if f_more_CellInfo==1;
	char fserialnum[FACTORY_SN_LEN+1];
    char netmode;// 2G 3G 4G (NETMODE_2G NETMODE_3G NETMODE_4G)
	char reserve[222]; //reserve for future use.
}WlInfo_T;

void WlUsePaxBaseSo(unsigned char select);//select value :USE_PAX_BASESO or NONUSE_PAX_BASESO
unsigned char WlGetList(char *funcStr);
void WlGetVersion(char lib_ver[]);
int WlSetPhoneNum(char * number);

/*
Input: key, The value is one of the following values:
    IMSI_KEY,
    CCID_KEY,
    MODULEINFO_KEY,
    RSSI_KEY,
    ISP_KEY,
    CELLINFO_KEY,

Output: WlInfo_T *info 
Return: error code

notes:
1.When ppp is online, don't call this function.
2.Please call WlInit() and WlOpenPort()function before calling this function.
3.when there is any error happend, the items of WlInfo_T are set to 0, NULL, or blank.
*/
int WlGetInfo(int key, WlInfo_T *info);
#endif

