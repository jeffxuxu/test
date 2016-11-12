#ifndef __AUTH_H__
#define __AUTH_H__

char * getDeviceID()  XIP_ATTRIBUTE(".xipsec0");
char * getVersionCode() XIP_ATTRIBUTE(".xipsec0");
uint16 IoT_exec_AT_cmd_Serial(PUCHAR pCmdBuf, INT16 at_cmd_len) XIP_ATTRIBUTE(".xipsec0");

#endif
