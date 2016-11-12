#include "uip.h"
#include "iot_api.h"
#include "udp_server.h"
#include "uip_timer.h"
#include "..\network\xlite.h"
#include "iot_custom.h"
#include "..\xJSON\xJSON.h"
#include "..\auth\auth.h"

#define UDP_INFO_BRCST_MAX_COUNT	20

static UINT8 broadcastCnt;
static struct timer udp_smnt_broadcast_timer;
static UIP_UDP_CONN *udp_smnt_broadcast_conn;


extern IOT_ADAPTER   	IoTpAd;

void udp_broadcast_stop()
{
	if(udp_smnt_broadcast_conn!=NULL)
		uip_udp_remove(udp_smnt_broadcast_conn);
	udp_smnt_broadcast_conn=NULL;
}

void udp_broadcast_start()
{
 
	UIP_UDP_CONN *conn=NULL;
	uip_ipaddr_t raddr={0};
	uip_ipaddr(raddr, 255,255,255,255);

	udp_broadcast_stop();

	conn = uip_udp_new(&raddr, HTONS(SMNT_SUCCESS_BROADCAST_RPORT));
	if (conn) {
		uip_udp_bind(conn, HTONS(SMNT_SUCCESS_BROADCAST_LPORT));

		udp_smnt_broadcast_conn=conn;
		broadcastCnt=0;
		timer_set(&udp_smnt_broadcast_timer, CLOCK_SECOND/2);
	}
}

#define VENDOR		"xlite_vendor"
#define NAME		"xlite_name"


/*
{
" info":{
		"vendor":"xxxx",
		"name":"xxxx",
		"prodtid":"xxxx",
		"vercode":"xx.xx.xx",
		"mac":"xx:xx:xx:xx:xx:xx",
	},
"desc":[
		{
			"ch":1,
			"ch-type":"light",
			"ch-name":"light",
			"attr-rw":[
				{"attr":"sw", "type":"Integer"}, 	
				{"attr":"brtness", "type":"Integer"},
				{"attr":"color", "type":"Integer"}
			]
		}
	]
}
*/
void send_deviceInfo() 
{
	int32 resplen;
	xJSON msg,*pmsg;
	pLiteHeader * pPacketHeader;
	PUCHAR packet;
	packet=(PUCHAR)malloc((IOT_XLITE_HEADER_LEN+MAX_CMD_LEN)*sizeof(UCHAR));
	memset(packet,0,(IOT_XLITE_HEADER_LEN+MAX_CMD_LEN)*sizeof(UCHAR));	

	pmsg=&msg;
	xJSON_InitObject(pmsg,packet+IOT_XLITE_HEADER_LEN);
		xJSON_ObjectStart(pmsg,KEY_INFO,0);
			xJSON_putString(pmsg,KEY_VENDOR,VENDOR);
			xJSON_putString(pmsg,KEY_NAME,NAME);
			xJSON_putString(pmsg,KEY_PRODTID,getDeviceID() );
			xJSON_putString(pmsg,KEY_VERCODE,getVersionCode());
			xJSON_putString(pmsg,KEY_MAC,gCurrentMAC);			
		xJSON_ObjectComplete(pmsg);
		xJSON_ArrayStart(pmsg,KEY_DESC,1);
			xJSON_ObjectStart(pmsg,NULL,0);
				xJSON_putInt(pmsg,KEY_CHANNEL,1);
				xJSON_putString(pmsg,KEY_CHANNEL_TYPE,VALUE_LIGHT);
				xJSON_putString(pmsg,KEY_CHANNEL_NAME,VALUE_LIGHT);		
				xJSON_ArrayStart(pmsg,KEY_ATTR_RW,0);
					xJSON_ObjectStart(pmsg,NULL,0);
						xJSON_putString(pmsg,KEY_ATTR,VALUE_SWITCH);
						xJSON_putString(pmsg,KEY_DATA_TYPE,VALUE_INTEGER);
					xJSON_ObjectComplete(pmsg);
					
					xJSON_ObjectStart(pmsg,NULL,1);
						xJSON_putString(pmsg,KEY_ATTR,VALUE_BRTNESS);
						xJSON_putString(pmsg,KEY_DATA_TYPE,VALUE_INTEGER);					
					xJSON_ObjectComplete(pmsg);
					
					xJSON_ObjectStart(pmsg,NULL,2);
						xJSON_putString(pmsg,KEY_ATTR,VALUE_COLOR);
						xJSON_putString(pmsg,KEY_DATA_TYPE,VALUE_INTEGER);					
					xJSON_ObjectComplete(pmsg);					
				xJSON_ArrayComplete(pmsg);	
			xJSON_ObjectComplete(pmsg);
		xJSON_ArrayComplete(pmsg);	
	xJSON_ObjectComplete(pmsg);
	pPacketHeader=(pLiteHeader *)packet;
	
	#ifdef FOR_TEST
	resplen=XLite_Packet_Build(pPacketHeader,OPCODE_TYPE_QRY_RSP,ERRCODE_SUCCESS,ENCRYPT_MODE_PLAINTEXT,msg.length,packet+IOT_XLITE_HEADER_LEN,IOT_RESPONSE);
	#else
	resplen=XLite_Packet_Build(pPacketHeader,OPCODE_TYPE_QRY_RSP,ERRCODE_SUCCESS,ENCRYPT_MODE_BASE64,msg.length,packet+IOT_XLITE_HEADER_LEN,IOT_RESPONSE);
	#endif
	Printf_High("resplen:%d,length:%d\n",resplen,msg.length);
	//Printf_High("send data:\n");
	//IoT_uart_output(packet, resplen);
	uip_send(packet, resplen);
	free(packet);	
}

void udp_broadcast_send()
{
	if (uip_poll()){
		if(timer_expired(&udp_smnt_broadcast_timer)) {
			timer_restart(&udp_smnt_broadcast_timer);
			if(broadcastCnt++>UDP_INFO_BRCST_MAX_COUNT){
				udp_broadcast_stop();
				udp_smnt_broadcast_conn=NULL;
				return;
			}
			Printf_High("broadcast\n");
			send_deviceInfo();
		}
	}
}
