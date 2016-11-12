
#ifndef __UDP_SERVER_H__
#define __UDP_SERVER_H__

void udp_broadcast_stop() XIP_ATTRIBUTE(".xipsec0");
void udp_broadcast_start() XIP_ATTRIBUTE(".xipsec0");
void udp_broadcast_send() XIP_ATTRIBUTE(".xipsec0");
void send_deviceInfo() XIP_ATTRIBUTE(".xipsec0");
void udp_server_init() XIP_ATTRIBUTE(".xipsec0");
void udp_server_handle(UIP_UDP_CONN *conn) XIP_ATTRIBUTE(".xipsec0");

#endif
