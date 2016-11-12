#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H
#include "xlite.h"

typedef enum eNetWorkType{
	NETWORK_TYPE_WAN,
	NETWORK_TYPE_LAN
}NetWorkType;

typedef struct _updateFW
{
	CHAR FOTAURL[32];
	CHAR FOTAfilename[64];
	INT16 FOTAPort;
}UpdateFW;

typedef enum eFOTA_STEP{
	FOTA_STEP_NONE,
	FOTA_STEP_QUERY,
	FOTA_STEP_DLING,
	FOTA_STEP_DL_COMPLETE,
	FOTA_STEP_CHECK_SUCCESS,
	FOTA_STEP_UPDATE_COMPLETE,
}FOTA_STEP;


extern UpdateFW updateFW;

void updateFWRequest() XIP_ATTRIBUTE(".xipsec0");


#if CFG_SUPPORT_DNS
void udp_client_resolv(u16_t * ipaddr) XIP_ATTRIBUTE(".xipsec0");
void udp_client_init() XIP_ATTRIBUTE(".xipsec0");
void udp_client_process() XIP_ATTRIBUTE(".xipsec0");

#endif
void udp_retx_send(PUCHAR data, u16_t len) XIP_ATTRIBUTE(".xipsec0");
void devRegister(PUCHAR respBuff, INT32 opCode)  XIP_ATTRIBUTE(".xipsec0");
void udp_client_connect() XIP_ATTRIBUTE(".xipsec0");
void udp_client_handle() XIP_ATTRIBUTE(".xipsec0");
void udp_client_parse(NetWorkType netType,UIP_UDP_CONN *conn) XIP_ATTRIBUTE(".xipsec0");
#endif
