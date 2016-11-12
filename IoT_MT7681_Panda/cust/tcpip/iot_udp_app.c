#include <stdio.h>
#include <string.h>

#include "iot_udp_app.h"
#include "dhcpc.h"
#include "uip.h"
#include "uiplib.h"
#include "iot_api.h"
//#include "sample_resolv_usage.h"
//#include "sample_udp_client.h"
#include "..\network\udp_server.h"
#include "..\utils\xutils.h"
#include "..\network\udp_client.h"
#if CFG_SUPPORT_DNS
#include "resolv.h"
#endif

#ifdef CONFIG_SOFTAP
#include "dhcpd.h"
#endif

extern UCHAR gCurrentAddress[];
extern IOT_ADAPTER   	IoTpAd;
struct iot_udp_app_state udp_app_state;


/***  Example for UDP APP 1 ****/
/*this APP1 will receive UDP packet from local port5555 ,  
    and determined the remote port
    then send UDP test packet to remote port */
#ifdef CONFIG_SOFTAP
#define UDP_APP1_ENABLE       0
extern struct uip_dhcpd_conn uip_dhpcd_conns[UIP_DHCPD_CONNS];
#endif
#if UDP_APP1_ENABLE
#define UDP_APP1_LOCAL_PORT   5555
#define UDP_APP1_REMOTE_PORT  0

extern void handle_udp_app1(void);

void udp_app1_server_init(uint16 LocalPort, uint16 RemotePort)
{
	//UIP_CONN *udp_conn=NULL;
	UIP_UDP_CONN *udp_conn=NULL;

	/* We don't specify a remote address and a remote port,
	   that means we can receive data from any address. */
	udp_conn = uip_udp_new(NULL, HTONS(RemotePort));
	if(udp_conn) {
		/* Bind to local port. */
		uip_udp_bind(udp_conn, HTONS(LocalPort));
	}
}
#endif


void
iot_udp_app_init(void)
{
#ifdef CONFIG_SOFTAP
	dhcpd_init();
#else
	dhcpc_init(gCurrentAddress, 6);
#endif

#if CFG_SUPPORT_DNS
	/* DNS client. */
	resolv_init();
#endif
	/* Customer APP start. */

#if UDP_APP1_ENABLE
	udp_app1_server_init(UDP_APP1_LOCAL_PORT, UDP_APP1_REMOTE_PORT);
#endif
	
	/********* Customer APP end. **********/
	
	return;
}

void
iot_udp_appcall(void)
{
	UIP_UDP_CONN *udp_conn = uip_udp_conn;
	u16_t lport, rport;

	lport=HTONS(udp_conn->lport);
	rport=HTONS(udp_conn->rport);

	if(lport == DHCPC_CLIENT_PORT) {
		handle_dhcp();
#if CFG_SUPPORT_DNS
	} else if (rport == DNS_SERVER_PORT) {
		handle_resolv();
#endif
	/* Customer APP start. */

	} else if (lport == SMNT_SUCCESS_BROADCAST_LPORT) {
		udp_broadcast_send();
	} else if(lport == SERVER_LPORT){
		//udp_client_handle(udp_conn);
	} else if (lport == LOCAL_LISTEN_PORT) {
		udp_client_parse(NETWORK_TYPE_LAN,udp_conn); 
		if(uip_poll()){
			udp_client_handle();
		}
	/* } else if (lport == 8888) {
		resolv_usage_sample(); */

	/* Customer APP end. */
	}
#ifdef CONFIG_SOFTAP
	else if (DHCPC_SERVER_PORT == lport)
	{
		handle_dhcpd();
	}
#endif
	else
	{
#if UDP_APP1_ENABLE	
		handle_udp_app1();
#endif
	}
	
	return;
}

#if UDP_APP1_ENABLE
void 
handle_udp_app1(void)
{
	UIP_UDP_CONN *udp_conn = uip_udp_conn;
	u16_t lport, rport;
	u8_t addr[16] = {0};
	uip_ipaddr_t ip;

	static u16_t rporttest=0;
	u16_t i=0;
	static u16_t j=0;
	u8_t testbuf[24] = {0};
	uip_ipaddr_t testip;
	
	lport=HTONS(udp_conn->lport);
	rport=HTONS(udp_conn->rport);
	uip_gethostaddr(&ip);

	Printf_High("%s,%d, lport[%d], rport[%d] \n",__FUNCTION__,__LINE__, lport,rport);

	if (lport == UDP_APP1_LOCAL_PORT)
	{
		if(uip_newdata()) {
			rporttest = rport;
			
			/* Here shows how to get the peer info of the received data. */
			sprintf((char *)addr, "%d.%d.%d.%d", 
					uip_ipaddr1(udp_conn->ripaddr), uip_ipaddr2(udp_conn->ripaddr),
					uip_ipaddr3(udp_conn->ripaddr), uip_ipaddr4(udp_conn->ripaddr));
			
			Printf_High("UDP Sample server RX:  LocalPort[%d],RemoteMACAddress[%s], RemotePort[%d], Data[%s]\n",
				HTONS(udp_conn->lport), addr, HTONS(udp_conn->rport), uip_appdata);

			Printf_High("HostIP: %d.%d.%d.%d \n", 
					uip_ipaddr1(ip), uip_ipaddr2(ip), uip_ipaddr3(ip), uip_ipaddr4(ip));
		}
	}
	
	{
		sprintf((char *)testbuf, "[ID=%d] UDP Test!!! \n", j++);

		/*send UDP packet to all UDP connection,  
		    Notice: the rporttest is got from last rx packet from 1 udp connection, it maybe improved for all connection*/
		for(i=0; i<UIP_DHCPD_CONNS; i++)
		{
			if(uip_dhpcd_conns[i].flag == 1)
			{
				uip_ipaddr(testip, 
						   (uip_dhpcd_conns[i].yiaddr & 0x000000ff), 
						   (uip_dhpcd_conns[i].yiaddr & 0x0000ff00)>>8,  
						   (uip_dhpcd_conns[i].yiaddr & 0x00ff0000)>>16,
						   (uip_dhpcd_conns[i].yiaddr & 0xff000000)>>24);
				
				IoT_send_udp_directly(testip, 
					                  uip_dhpcd_conns[i].chaddr,
					                  UDP_APP1_LOCAL_PORT,
					                  rporttest,
					                  testbuf,
					                  sizeof(testbuf));
				msecDelay(10);
			}
		}
	}
}
#endif
