#ifndef DEV_H
#define DEV_H
#include "..\cJSON\cJSON.h"

//#define DEV_CURTAIN_ENABLE
//#define DEV_FISHTANK_ENABLE

#define LIGHT_CHANNEL_ID		1


void onDeviceInit() XIP_ATTRIBUTE(".xipsec0");
void onDeviceProcessTask();
void onDeviceConnected() XIP_ATTRIBUTE(".xipsec0");
void onDeviceDisconnected() XIP_ATTRIBUTE(".xipsec0");

INT32 IoT_AppFunction_Ctrl(INT32 chid,INT32 power,INT32 brtness,INT32 color)	XIP_ATTRIBUTE(".xipsec0");
INT32 IoT_AppFunction_Query(INT32 pID,INT32 pt,INT32 pn)	XIP_ATTRIBUTE(".xipsec0");
void onDataReceived(UINT32 opCode,cJSON *json) XIP_ATTRIBUTE(".xipsec0");
void onDeviceStatusReport(UCHAR * respBuff,UINT32 * respLen) XIP_ATTRIBUTE(".xipsec0");
void onDeviceStatusQuery(UCHAR * respBuff,UINT32 * respLen) XIP_ATTRIBUTE(".xipsec0");
void statusReportPacketBuild(UCHAR * respBuff,UINT32 * respLen) XIP_ATTRIBUTE(".xipsec0");
void statusQueryPacketBuild(UCHAR * respBuff,UINT32 * respLen) XIP_ATTRIBUTE(".xipsec0");
void sharpCommandSend(int type, int subtype,INT8 * cmdbody,INT8 cmdlen) XIP_ATTRIBUTE(".xipsec0");

#endif
