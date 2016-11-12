#ifndef _XLITE_H
#define _XLITE_H
#include "types.h"
#include "iot_api.h"

#ifdef _WIN32
#define uint32 __int32
#define unit64 __int64
#endif


#define MAX_CMD_LEN		512

//SERVER PORT DEFINE
#define		SERVER_RPORT		9999
#define		SERVER_LPORT		6070
//LOCAL PORT DEFINE
#define		LOCAL_LISTEN_PORT					6060
#define		SMNT_SUCCESS_BROADCAST_RPORT	6080
#define		SMNT_SUCCESS_BROADCAST_LPORT	6666
	

//DEVICE DEFINE
#define	IOT_DEVICE			1
#define	MOBILE_DEVICE		2
#define 	SERVER				4

#define FOURCC( ch0, ch1, ch2, ch3 )				\
			( (UINT32)(BYTE)(ch0) | ( (UINT32)(BYTE)(ch1) << 8 ) |	\
			( (UINT32)(BYTE)(ch2) << 16 ) | ( (UINT32)(BYTE)(ch3) << 24 ) )

//////////////////////////////////////////////////////////////////////////
//packet magic
#define     XLITE_MAGIC           FOURCC('L','i','t','e')
#define     XLITE_MAGIC_LEN    4
#define	  XLITE_PROTOCOL_VER	0

#define ENCRYPT_MODE_PLAINTEXT	0
#define ENCRYPT_MODE_BASE64		1
#define ENCRYPT_MODE_AES_CBC		2
#define ENCRYPT_MODE_AES_ECB		3


#define  OPCODE_TYPE_CTRL_REQ 		100
#define  OPCODE_TYPE_CTRL_RSP  		101
#define  OPCODE_TYPE_QRY_REQ   		200
#define  OPCODE_TYPE_QRY_RSP 		201
#define  OPCODE_TYPE_STAT_REQ 		300
#define  OPCODE_TYPE_STAT_RSP 		301
#define  OPCODE_TYPE_ONLINE_REQ 	302
#define  OPCODE_TYPE_ONLINE_RSP 	303
#define  OPCODE_TYPE_REP_REQ		400
#define  OPCODE_TYPE_REP_RSP		401
#define  OPCODE_TYPE_HB_REQ		402
#define  OPCODE_TYPE_HB_RSP		403
#define  OPCODE_TYPE_CFG_REQ		500
#define  OPCODE_TYPE_CFG_RSP 		501
#define  OPCODE_TYPE_CLR_REQ 		502
#define  OPCODE_TYPE_DFT_REQ		504
#define  OPCODE_TYPE_REBOOT_REQ	506
#define  OPCODE_TYPE_ASSOC_REQ	600
#define  OPCODE_TYPE_ASSOC_RSP 	601
#define  OPCODE_TYPE_DISASSOC_REQ 	602
#define  OPCODE_TYPE_DISASSOC_RSP 	603
#define  OPCODE_TYPE_DEVLIST_REQ 		604
#define  OPCODE_TYPE_DEVLIST_RSP 		605
#define  OPCODE_TYPE_TRIG_REQ 			700
#define  OPCODE_TYPE_TRIG_RSP 			701
#define  OPCODE_TYPE_TRIG_QRY_REQ 	702
#define  OPCODE_TYPE_TRIG_QRY_RSP 		703
#define  OPCODE_TYPE_TRIG_MOD_REQ 	704
#define  OPCODE_TYPE_TRIG_MOD_RSP 	705
#define  OPCODE_TYPE_TRIG_DEL_REQ 		706
#define  OPCODE_TYPE_TRIG_DEL_REQ		707
#define  OPCODE_TYPE_OUT_BOUND		10000

//ERROR CODE DEFINE
#define ERRCODE_REQUEST						0
#define ERRCODE_SUCCESS						8888
#define ERRCODE_ILLEGAL							8800
#define ERRCODE_PARSE_FAILED					8010
#define ERRCODE_PARSE_INCOMPLETE				8000
#define ERRCODE_UNKNOWN						8500							
#define ERRCODE_TIMEOUT						1
#define ERRCODE_NOT_MATCH						10
#define ERRCODE_DATABASE_EXCEPTION			19
#define ERRCODE_DEV_PWD_WRONG				20
#define ERRCODE_USER_PWS_WRONG				29
#define ERRCODE_NOT_NULL						200
#define ERRCODE_OUT_BOUND						201


//JSON KEY DEFINE
#define KEY_INFO		"info"
#define KEY_DESC		"desc"
#define KEY_VENDOR		"vendor"
#define KEY_NAME		"name"
#define KEY_PRODTID		"prodtid"
#define KEY_VERCODE		"vercode"
#define KEY_MAC			"mac"
#define KEY_CHANNEL		"ch"
#define KEY_CHANNEL_TYPE		"ch-type"
#define KEY_CHANNEL_NAME		"ch-name"
#define KEY_ATTR_RW			"attr-rw"
#define KEY_ATTR_R				"attr-r"
#define KEY_ATTR_W				"attr-w"
#define KEY_ATTR 				"attr"
#define KEY_DATA_TYPE			"type"

#define KEY_SSID					"ssid"
#define KEY_PASSPHRASE			"passphrase"
#define KEY_HOST				"host"
#define KEY_PORT				"port"
#define KEY_INTERVAL			"interval"
#define KEY_PASSWD				"passwd"

#define KEY_TRIGGER				"trigger"
#define KEY_IF					"if"
#define KEY_COND				"cond"
#define KEY_EVENT				"event"
#define KEY_CMP					"cmp"
#define KEY_VALUE				"value"
#define KEY_DO					"do"
#define KEY_REPEAT				"repeat"


#define KEY_CTRL				"ctrl"
#define KEY_STAT_QUERY		"query"
#define KEY_RESULT			"result"
#define KEY_STAT			"stat"
#define KEY_REPORT			"report"
#define KEY_CONF			"conf"
#define KEY_LABEL			"label"

#define KEY_SWITCH			"sw"
#define KEY_BRTNESS			"brtness"
#define KEY_CTEMP			"ctemp"
#define KEY_COLOR			"color"

//JSON VALUE DEFINE
#define VALUE_LIGHT				"light"
#define VALUE_OUTLETS			"outlets"

#define VALUE_INTEGER			"Integer"
#define VALUE_BYTESTREAM		"bytes"			


#define VALUE_SWITCH			"sw"
#define VALUE_BRTNESS			"brtness"
#define VALUE_CTEMP				"ctemp"
#define VALUE_COLOR				"color"

#define VALUE_MON					"mon"
#define VALUE_TUE					"tue"
#define VALUE_WEN					"wen"
#define VALUE_THU					"thu"
#define VALUE_FRI					"fri"
#define VALUE_SAT					"sat"
#define VALUE_SUN					"sun"
#define VALUE_TIME				"time"

//#define DEVICE_ID 				"devid is ssssss "

#define IOT_RESPONSE       1
#define IOT_START       	 0

typedef struct GNU_PACKED __LiteHeader{
unsigned int magic;//0x4C697464(Lite)
unsigned char version;//当前版本号0
unsigned char encrypt;//加密模式，0--明文，1--base64，2--aes_cbc，3--aes_ecb
unsigned short length;//报文体长度，不超过512
}_LiteHeader,*_pLiteHeader;


typedef struct GNU_PACKED xLiteCMDHeader{
unsigned short opcode;//操作码
unsigned short seqnum;//流水号
unsigned short errcode;//错误码
unsigned char devtype;//发送端设备类型，1--IoT设备，2--移动设备，4--Server
unsigned char reserve;//保留对齐
unsigned long long timestamp;//时间戳，GMT标准时间，IoT设备通过服务器的时间戳同步时间
unsigned char devid[16];//IoT设备ID
unsigned char meidsign[16];//移动设备的IMEI/MEID/ESN的md5签名，GSM制式为IMEI，CDMA制式为MEID或ESN
}LiteCMDHeader,*pLiteCMDHeader;

typedef struct GNU_PACKED xLiteHeader{
unsigned int magic;//0x4C697464(Lite)
unsigned char version;//当前版本号0
unsigned char encrypt;//加密模式，0--明文，1--base64，2--aes_cbc，3--aes_ecb
unsigned short length;//报文体长度，不超过512

unsigned short opcode;//操作码
unsigned short seqnum;//流水号
unsigned short errcode;//错误码
unsigned char devtype;//发送端设备类型，1--IoT设备，2--移动设备，4--Server
unsigned char reserve;//保留对齐
INT64 timestamp;//时间戳，GMT标准时间，IoT设备通过服务器的时间戳同步时间
unsigned char devid[16];//IoT设备ID
unsigned char meidsign[16];//移动设备的IMEI/MEID/ESN的md5签名，GSM制式为IMEI，CDMA制式为MEID或ESN
}LiteHeader,*pLiteHeader;

 

 #define IOT_XLITE_HEADER_LEN sizeof(LiteHeader)
 #define IOT_LITE_HEADER_LEN sizeof(_LiteHeader)
 #define IOT_CMD_HEADER_LEN sizeof(LiteCMDHeader)

#define BASE64_BUFF_SIZE_GET(x)	(((x)+2)/3*4)
 
extern uint16 atomic;
int32 XLite_Packet_Build(pLiteHeader pPacketHeader,short opCode,short errCode,UINT8 encryptMode,int length,char * jsonstring,UINT8 Direction) XIP_ATTRIBUTE(".xipsec0");

#endif
