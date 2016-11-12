#ifndef CUST_WLD_H
#define CUST_WLD_H

#include "cust_timer.h"



typedef struct t_WatchDog_Feed{
	uint16 pulseflag;
	uint16 dutyvalue;
	uint16 freevalue;
	Timer_Cust timer;	
}WatchDog_Feed;

typedef struct t_GPIO_KeyDetInfo{
	uint8  gpio;
	uint8  pressed;
	uint8  released;
	uint8  preStatus;
	Timer_Cust timer;
}GPIO_KeyDetInfo;

#define STATUS_SET						0
#define STATUS_CLR						1
#define	DEV_STATUS_CONN					0x01
#define	DEV_STATUS_NOSERIALCODE			0x80



typedef struct t_StatusLedDesc{
	uint8  gpio:4;
	uint8  flag:4;
	UINT8  status;
	uint16  interval;
	Timer_Cust timer;
}StatusLedDesc;




#define GPIO_WATCH_DOG		1
#define GPIO_OUTPUT		3
#define GPIO_INPUT		2
#define GPIO_KEY		4
#define GPIO_STATUS_LED		0

UINT16 flash_crc16_ccitt(INT32 addrstart, INT32 len) XIP_ATTRIBUTE(".xipsec0");
void status_led_set(UINT8 op,UINT8 status) XIP_ATTRIBUTE(".xipsec0");
void status_led_dataRecv()  XIP_ATTRIBUTE(".xipsec0");
void devRegCheck() XIP_ATTRIBUTE(".xipsec0");
void macStrConvert() XIP_ATTRIBUTE(".xipsec0");
void parseMacString(char *cp,uint8 * mac) XIP_ATTRIBUTE(".xipsec0");
uint8 _hexToByte(char *cp,uint8 len) XIP_ATTRIBUTE(".xipsec0");
void _convertToHex(PCHAR dest,INT8 * data,int length)  XIP_ATTRIBUTE(".xipsec0");



#endif
