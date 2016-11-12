#include "uip.h"
#include "uip_timer.h"
#include "iot_api.h"
#include "udp_client.h"
#include "..\xJSON\xJSON.h"
#include "..\cJSON\cJSON.h"
#include "xlite.h"
#include "..\dev\light.h"
#include "..\utils\xutils.h"
#include "IoT_Custom.h"
#include "..\dev\dev.h"
#include "..\time\time.h"


#define DNS_RESOLV_NONE		0
#define DNS_RESOLV_QUERY	1
#define DNS_RESOLV_DONE		2

#define NACK_CNT_MAX	10
#define DNSResolvFailed_CNT_MAX		2
#define NACK_LINKDOWN_INTERVAL		(1000*60*30)
UINT8 gDNSResolvStatus;
UINT8 gDNSResolvTry;
u16_t serveripaddr[2]={0};
static UIP_UDP_CONN *udp_conn_svr;
static UIP_UDP_CONN *udp_conn_local;
static UINT8 gNAckCounter=0;
static UINT8 gDNSResolvFailedCnt=0;
static UINT32 gPreLinkDownTime=0;

static struct timer dns_resolv_timer;
static struct timer udp_register_timer;
static UINT8 gClientStart=0;
static struct timer udp_client_timer;
struct timer udp_client_reporttimer;
struct timer udp_updateFW_timer;
struct timer udp_updateFW_ack_timer; //updateFW result ack
struct timer udp_heartbeat_timer;
Timer_Cust update_timer = {0, 0};

static uint8 countdown=3;


unsigned short preOpcode = 0;
unsigned short proLocalOpcode = 0;
UINT16 preSequenceNum=0;
uip_ipaddr_t preIpAddr={0};
UINT16 preLocalSequenceNum = 0;
uip_ipaddr_t preLocalIpAddr={0};
UINT16 gHeartheat_retx = 0; //是否收到心跳回复
extern UINT8 gLightSetRetxCnt;
extern INT32 gLightReportOption;
extern IoTLightStatus gLightStatus;
extern IOT_ADAPTER   IoTpAd;
extern STA_FOTA_CONFIG IoTStaFOTACfg;
extern STA_FOTA_CONFIG StaFOTAQueryInfo;
extern UINT8 gUpdateFWDownload;
extern UINT16 TIMERCLOCK[16];
UINT32 GetMsTimer(VOID);




#if CFG_SUPPORT_DNS
void udp_client_init(){
	timer_set(&dns_resolv_timer, CLOCK_SECOND*60);
	gDNSResolvStatus=DNS_RESOLV_QUERY;
	gDNSResolvTry=2;
	gDNSResolvFailedCnt=0;
	resolv_query(IoTpAd.ComCfg.IoT_ServeDomain);
}

void udp_client_resolv(u16_t * ipaddr)
{
	if(NULL==ipaddr||isInnerIP(ipaddr)){
		FUNC_LOG("dns resolv failed\n");
		if(gDNSResolvTry){
			gDNSResolvTry--;
			resolv_query(IoTpAd.ComCfg.IoT_ServeDomain);
		}else{
			gDNSResolvStatus=DNS_RESOLV_NONE;
			gDNSResolvFailedCnt++;
		}
	}else{
		gDNSResolvFailedCnt=0;
		gDNSResolvStatus=DNS_RESOLV_DONE;
		FUNC_LOG("dns resolv done,%d.%d.%d.%d\n", uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr),
			uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
		serveripaddr[0]=ipaddr[0];
		serveripaddr[1]=ipaddr[1];
		udp_client_connect();
	}
}
#endif

void udp_local_listen()
{
	if(udp_conn_local!=NULL)
		uip_udp_remove(udp_conn_local);

	udp_conn_local = uip_udp_new(NULL, HTONS(0));
	if(udp_conn_local) {
		uip_udp_bind(udp_conn_local, HTONS(LOCAL_LISTEN_PORT));
	}	
}

void udp_client_connect()
{
#if (!CFG_SUPPORT_DNS)
	uip_ipaddr(serveripaddr, 120,25,161,131);
#endif

	//if(udp_conn_svr!=NULL)
	//	uip_udp_remove(udp_conn_svr);

	//udp_conn_svr = uip_udp_new(&serveripaddr, HTONS(SERVER_RPORT));
	//if (udp_conn_svr) {
	//	uip_udp_bind(udp_conn_svr, HTONS(SERVER_LPORT));
		timer_set(&udp_client_timer, 30*CLOCK_SECOND);
		gClientStart=1;
		timer_set(&udp_client_reporttimer,1*CLOCK_SECOND);
		timer_set(&udp_register_timer,5*CLOCK_SECOND);

		//1.timer set
		timer_set(&udp_updateFW_timer, 5*CLOCK_SECOND);
		timer_set(&udp_updateFW_ack_timer, 10*CLOCK_SECOND);
		timer_set(&udp_heartbeat_timer, 5*CLOCK_SECOND);	
		//timer_set(&update_timer, 3*CLOCK_SECOND);
	//}
}

void udp_client_process()
{
	UINT32 curTime;
	if(timer_expired(&dns_resolv_timer)&&gDNSResolvStatus==DNS_RESOLV_NONE){
		timer_restart(&dns_resolv_timer);
		gDNSResolvStatus=DNS_RESOLV_QUERY;
		resolv_query(IoTpAd.ComCfg.IoT_ServeDomain);
		gDNSResolvTry=2;
	}

	curTime=GetMsTimer();
	if((NACK_CNT_MAX<=gNAckCounter||DNSResolvFailed_CNT_MAX<=gDNSResolvFailedCnt)
		&&(curTime-gPreLinkDownTime>NACK_LINKDOWN_INTERVAL||gPreLinkDownTime==0)){
		gPreLinkDownTime=curTime;
		gNAckCounter=0;
		gDNSResolvFailedCnt=0;
		dhcpc_requestAgain();
	}
}

void udp_client_disconnect()
{
	udp_conn_svr=NULL;
	udp_conn_local=NULL;
}




void udp_client_parse(NetWorkType netType,UIP_UDP_CONN *conn)
{
#if (CONFIG_STATION==1)
	INT32 respLen=0,iRet;
	PUCHAR iot_resp=NULL;
	cJSON * cjsonobj=NULL,* objItem=NULL,*arrayItem=NULL;
	pLiteHeader recvHeader;
	_pLiteHeader respHeader;
	PCHAR jsonstring,plaintext=NULL;
	UCHAR timetemp[8] = {0};
	PUCHAR pBase64Buf;

	
 
	if(uip_newdata()){
		status_led_dataRecv();
		recvHeader=(pLiteHeader *)uip_appdata;

		
		#ifndef FOR_TEST
		if(recvHeader->encrypt==ENCRYPT_MODE_BASE64){
			pBase64Buf = (CHAR*)malloc(sizeof(CHAR)*(recvHeader->length));
			Base64Decode(uip_appdata+IOT_LITE_HEADER_LEN, pBase64Buf, recvHeader->length);
			memcpy(uip_appdata+IOT_LITE_HEADER_LEN, pBase64Buf, (recvHeader->length/4)*3);
			free(pBase64Buf);
		}
		#endif
		Printf_High("len:%d\n",recvHeader->length);
		//Printf_High("jason:%s\n",uip_appdata+IOT_XLITE_HEADER_LEN);
		Printf_High("id:%s,devid:%s\n",getDeviceID(),recvHeader->devid);
		//Printf_High("magic:%x\n",recvHeader->magic);
		FUNC_LOG("opcode:%d\n",recvHeader->opcode);
		if(recvHeader->opcode==OPCODE_TYPE_REP_REQ)
		{
			return;
		}
		#if 1
		if((recvHeader->opcode!=OPCODE_TYPE_QRY_REQ)&&((recvHeader->magic != XLITE_MAGIC)
			||(memcmp(recvHeader->devid,getDeviceID(),sizeof(recvHeader->devid))))
			)
		{
			return;
		}
		#endif

		#if 0//ndef FOR_TEST
		if(netType == NETWORK_TYPE_WAN){
			if(preSequenceNum==recvHeader->seqnum
				&&!memcmp(&preIpAddr,&conn->ripaddr,sizeof(uip_ipaddr_t))
				&&(preOpcode == recvHeader->opcode)){
				return;
			}else{
				preSequenceNum=recvHeader->seqnum;
				memcpy(&preIpAddr,&conn->ripaddr,sizeof(uip_ipaddr_t));
				preOpcode = recvHeader->opcode;
			}
		}else{
			if(preLocalSequenceNum==recvHeader->seqnum
				&&!memcmp(&preLocalIpAddr,&conn->ripaddr,sizeof(uip_ipaddr_t))
				&&(proLocalOpcode == recvHeader->opcode)){
				return;
			}else{
				preLocalSequenceNum=recvHeader->seqnum;
				memcpy(&preLocalIpAddr,&conn->ripaddr,sizeof(uip_ipaddr_t));
				proLocalOpcode = recvHeader->opcode;
			}
		}
		#endif

		preSequenceNum=recvHeader->seqnum;
		
		//Printf_High("length:%x\n",recvHeader->length);
		if(((recvHeader->length)&&(recvHeader->length<MAX_CMD_LEN))&&(recvHeader->opcode!=OPCODE_TYPE_QRY_REQ)&&(recvHeader->opcode!=OPCODE_TYPE_CLR_REQ)
			&&(recvHeader->opcode!=OPCODE_TYPE_HB_RSP)&&(recvHeader->opcode!=OPCODE_TYPE_REP_RSP))
		{
			jsonstring=(CHAR*)malloc(sizeof(CHAR)*recvHeader->length);
			#ifdef FOR_TEST
			memcpy(jsonstring,uip_appdata+IOT_XLITE_HEADER_LEN,recvHeader->length-IOT_CMD_HEADER_LEN);
			#else
			memcpy(jsonstring,uip_appdata+IOT_XLITE_HEADER_LEN,recvHeader->length-BASE64_BUFF_SIZE_GET(IOT_CMD_HEADER_LEN));
			#endif
			//Printf_High("jsonstring:%s\n",jsonstring);
			cjsonobj=cJSON_Parse(jsonstring);
			free(jsonstring);
			
			if(NULL==cjsonobj){
				Printf_High("json error\n");

				iot_resp=(CHAR*)malloc(sizeof(CHAR)*(IOT_LITE_HEADER_LEN+BASE64_BUFF_SIZE_GET(IOT_CMD_HEADER_LEN)));
				respLen=XLite_Packet_Build(iot_resp,OPCODE_TYPE_OUT_BOUND,ERRCODE_PARSE_INCOMPLETE,ENCRYPT_MODE_BASE64,0,NULL,IOT_RESPONSE);	
				uip_send(iot_resp,respLen);
				free(iot_resp);
				return;
			}
		}
		
		FUNC_LOG("%s opcode:%d\n",(NETWORK_TYPE_WAN==netType)?"Remote":"Local",recvHeader->opcode);
		
		switch(recvHeader->opcode){
			case OPCODE_TYPE_CTRL_REQ:
				FUNC_LOG("errcode:%d\n",recvHeader->errcode);
				if(ERRCODE_REQUEST==recvHeader->errcode){
					onDataReceived(recvHeader->opcode,cjsonobj);
					iot_resp=(PUCHAR)malloc(sizeof(UCHAR)*(IOT_LITE_HEADER_LEN+BASE64_BUFF_SIZE_GET(IOT_CMD_HEADER_LEN)));
					#ifdef FOR_TEST
					respLen=XLite_Packet_Build(iot_resp,OPCODE_TYPE_CTRL_RSP,ERRCODE_SUCCESS,ENCRYPT_MODE_PLAINTEXT,0,NULL,IOT_RESPONSE);	
					#else
					respLen=XLite_Packet_Build(iot_resp,OPCODE_TYPE_CTRL_RSP,ERRCODE_SUCCESS,ENCRYPT_MODE_BASE64,0,NULL,IOT_RESPONSE);	
					#endif
				}
				break;
			case OPCODE_TYPE_STAT_REQ:
				if(ERRCODE_REQUEST==recvHeader->errcode){
					onDataReceived(recvHeader->opcode,cjsonobj);
					iot_resp=(PUCHAR)malloc(sizeof(UCHAR)*(IOT_LITE_HEADER_LEN+MAX_CMD_LEN));
					memset(iot_resp, '\0', IOT_XLITE_HEADER_LEN+MAX_CMD_LEN);
					onDeviceStatusQuery(iot_resp+IOT_XLITE_HEADER_LEN,&respLen);
					if(respLen>0){
						respHeader=(pLiteHeader *)iot_resp;
						#ifdef FOR_TEST
						respLen=XLite_Packet_Build(respHeader,OPCODE_TYPE_STAT_RSP,ERRCODE_SUCCESS,ENCRYPT_MODE_PLAINTEXT,respLen,iot_resp+IOT_XLITE_HEADER_LEN,IOT_RESPONSE);
						#else
						respLen=XLite_Packet_Build(respHeader,OPCODE_TYPE_STAT_RSP,ERRCODE_SUCCESS,ENCRYPT_MODE_BASE64,respLen,iot_resp+IOT_XLITE_HEADER_LEN,IOT_RESPONSE);	
						#endif
					}
				}
				break;
			case OPCODE_TYPE_QRY_REQ:
			if(ERRCODE_REQUEST==recvHeader->errcode){
				send_deviceInfo();
			}
			break;
			case OPCODE_TYPE_HB_RSP:
				timeSYNC.syncCurTime = (GetMsTimer()/1000);
				gHeartheat_retx = 0;				
				if(NETWORK_TYPE_WAN==netType&&ERRCODE_REQUEST==recvHeader->errcode)
					gNAckCounter=0;				
				break;
			case OPCODE_TYPE_DFT_REQ:
				if(NETWORK_TYPE_LAN==netType&&ERRCODE_REQUEST==recvHeader->errcode){
					reset_cfg();
					IoT_Cmd_LinkDown(0);
					Sys_reboot();
				}
				break;				
			case OPCODE_TYPE_CLR_REQ:
				if(ERRCODE_REQUEST==recvHeader->errcode){
					if(NETWORK_TYPE_LAN==netType){
						reset_sta_cfg();
						IoT_Cmd_LinkDown(0);
					}
				}
				break;		

			case OPCODE_TYPE_CFG_REQ:
				if(ERRCODE_REQUEST==recvHeader->errcode)
				{
					onDataReceived(recvHeader->opcode,cjsonobj);
					iot_resp=(PUCHAR)malloc(sizeof(UCHAR)*(IOT_LITE_HEADER_LEN+BASE64_BUFF_SIZE_GET(IOT_CMD_HEADER_LEN)));
					respLen=XLite_Packet_Build(iot_resp,OPCODE_TYPE_CFG_RSP,ERRCODE_SUCCESS,ENCRYPT_MODE_BASE64,0,NULL,IOT_RESPONSE);	
				}
			break;

			case OPCODE_TYPE_TRIG_REQ:
				if(ERRCODE_REQUEST==recvHeader->errcode)
				{
					onDataReceived(recvHeader->opcode,cjsonobj);
					iot_resp=(PUCHAR)malloc(sizeof(UCHAR)*(IOT_LITE_HEADER_LEN+MAX_CMD_LEN));
					memset(iot_resp, '\0', (IOT_LITE_HEADER_LEN+MAX_CMD_LEN));
					onDeviceTrigger(iot_resp+IOT_XLITE_HEADER_LEN,&respLen);
					respLen=XLite_Packet_Build(iot_resp,OPCODE_TYPE_TRIG_RSP,ERRCODE_SUCCESS,ENCRYPT_MODE_BASE64,0,NULL,IOT_RESPONSE);	
				}
			break;
			
			
			default:
				//iot_resp=(PUCHAR)malloc(sizeof(UCHAR)*(IOT_LITE_HEADER_LEN+BASE64_BUFF_SIZE_GET(IOT_CMD_HEADER_LEN)));
				//respLen=XLite_Packet_Build(iot_resp,recvHeader->opcode,ERRCODE_ILLEGAL,ENCRYPT_MODE_BASE64,0,NULL,IOT_RESPONSE);	
				break;
		}
l_parseend:		
		cJSON_Delete(cjsonobj);
		if(respLen>0){		
				uip_send((void *)iot_resp,respLen);
		}
		if(iot_resp!=NULL)  free(iot_resp);		
	}
#endif	
}

extern void uip_udp_sendto(const void *data, u16_t len,uip_ipaddr_t *ripaddr, u16_t rport);
void udp_client_handle()
{
	UINT32 respLen=0;
	pLiteHeader *packetHeader;
	PUCHAR iot_resp;


	//Printf_High("handle\n");
	//if (uip_poll()) {
		if(timer_expired(&udp_heartbeat_timer) && (gHeartheat_retx > 0)){
			packetHeader=(pLiteHeader *)malloc(sizeof(CHAR)*(IOT_LITE_HEADER_LEN+BASE64_BUFF_SIZE_GET(IOT_CMD_HEADER_LEN)));
			if(NULL!=packetHeader){
				Printf_High("HB\n");
				#ifdef FOR_TEST
				respLen=XLite_Packet_Build(packetHeader,OPCODE_TYPE_HB_REQ,ERRCODE_REQUEST,ENCRYPT_MODE_PLAINTEXT,0,NULL,IOT_START);
				#else
				respLen=XLite_Packet_Build(packetHeader,OPCODE_TYPE_HB_REQ,ERRCODE_REQUEST,ENCRYPT_MODE_BASE64,0,NULL,IOT_START);
				#endif
				uip_udp_sendto(packetHeader, respLen,&serveripaddr,HTONS(SERVER_RPORT));
				gHeartheat_retx--;
				//gNAckCounter++;
				free(packetHeader);
				timer_restart(&udp_heartbeat_timer);
			}
		}else{
			if(((gLightReportOption&LIGHT_OPT_REPORT))&&timer_expired(&udp_client_reporttimer)){
				iot_resp=(PUCHAR)malloc(sizeof(UCHAR)*(IOT_XLITE_HEADER_LEN+MAX_CMD_LEN));
				onDeviceStatusReport(iot_resp+IOT_XLITE_HEADER_LEN,&respLen);
				if(respLen>0){
					packetHeader=(pLiteHeader *)iot_resp;
					Printf_High("REP\n");
					#ifdef FOR_TEST
					respLen=XLite_Packet_Build(packetHeader,OPCODE_TYPE_REP_REQ,ERRCODE_REQUEST,ENCRYPT_MODE_PLAINTEXT,respLen,iot_resp+IOT_XLITE_HEADER_LEN,IOT_START);
					#else
					respLen=XLite_Packet_Build(packetHeader,OPCODE_TYPE_REP_REQ,ERRCODE_REQUEST,ENCRYPT_MODE_BASE64,respLen,iot_resp+IOT_XLITE_HEADER_LEN,IOT_START);	
					#endif
					uip_udp_sendto(iot_resp,respLen,&serveripaddr,HTONS(SERVER_RPORT));
				}
				free(iot_resp);
				timer_restart(&udp_client_reporttimer);
			}else if(timer_expired(&udp_client_timer)||gClientStart) {
				gClientStart=0;
				FUNC_LOG("send [%u]\n",atomic);
				packetHeader=(pLiteHeader *)malloc(sizeof(CHAR)*IOT_LITE_HEADER_LEN+BASE64_BUFF_SIZE_GET(IOT_CMD_HEADER_LEN));
				if(NULL!=packetHeader){
					FUNC_LOG("HB2\n");
					#ifdef FOR_TEST
					respLen=XLite_Packet_Build(packetHeader,OPCODE_TYPE_HB_REQ,ERRCODE_REQUEST,ENCRYPT_MODE_PLAINTEXT,0,NULL,IOT_START);
					#else
					respLen=XLite_Packet_Build(packetHeader,OPCODE_TYPE_HB_REQ,ERRCODE_REQUEST,ENCRYPT_MODE_BASE64,0,NULL,IOT_START);
					#endif
					uip_udp_sendto(packetHeader, respLen,&serveripaddr,HTONS(SERVER_RPORT));
					gHeartheat_retx = 2;
					//gNAckCounter++;
					free(packetHeader);
					timer_restart(&udp_client_timer);	
					timer_restart(&udp_heartbeat_timer);
				}
			}
		}
	//}
	//udp_client_parse(NETWORK_TYPE_WAN,conn);
}
