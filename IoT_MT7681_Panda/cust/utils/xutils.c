#include "uip.h"
#include "xutils.h"
#include "cust_timer.h"
#include "iot_api.h"


//static WatchDog_Feed watchdogfeed;
//static GPIO_KeyDetInfo gpioKey;
static StatusLedDesc statusled;
static Timer_Cust dataRecvStatusTimer;
UINT8 ledFlashCnt=0;
extern IOT_ADAPTER   IoTpAd;
extern const unsigned int crc_ta_8[256];

/*static const unsigned short crc16tab[256]= {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};*/
UINT16 flash_crc16_ccitt(INT32 addrstart, INT32 len)
{
	INT32 i,counter,buflen,addr;
	UINT16 crc = 0;
	PUINT8 buf;
	
	counter=0;
	addr=addrstart;
	buf=(PUINT8)malloc(sizeof(UINT8)*256);
	while(counter<len){
		buflen=min(256,len-counter);
		spi_flash_read((UINT32)addr, buf, buflen);
		for( i = 0; i < buflen; i++)
			crc = (crc<<8) ^ (UINT16)(crc_ta_8[((crc>>8) ^ buf[i])&0x00FF]&0xffff);
		counter+=buflen;
		addr+=buflen;
	}
	free(buf);
	return crc; 
}

BOOLEAN isHexString(char * str){
	while(*(str++)){
		if(!((*str>='A'&&*str<='F')||(*str>='a'&&*str<='f')||(*str>='0'&&*str<='9')))
			return FALSE;
	}

	return TRUE;
}

uint8 _hexToByte(char *cp,uint8 len)
{
	uint8 i,n,temp=0;

	if(strlen(cp)<len){
		return 0;
	}

	for(i=0;i<len;i++){
		if(cp[i]>='A'&&cp[i]<='F')
			n=cp[i]-'A'+10;
		else if(cp[i]>='a'&&cp[i]<='f')
			n=cp[i]-'a'+10;
		else
			n=cp[i]-'0';

		temp=temp*16+n;
	}

	return temp;
}


void _convertToHex(PCHAR dest,INT8 * data,int length)
{
	INT8 i;
	INT8 halfbyte;
	INT8 two_halfs;

	if(NULL==data){
		return;
	}
	
	for (i = 0; i < length; i++) {
		halfbyte = (data[i] >> 4) & 0x0F;
		 two_halfs= 0;
		do {
			if ((0 <= halfbyte) && (halfbyte <= 9))
				dest[i*2+two_halfs]= ('0' + halfbyte);
			else
				dest[i*2+two_halfs]= ('A' + (halfbyte - 10));
			halfbyte = data[i] & 0x0F;
		} while (two_halfs++ < 1);
	}
}



void status_led_set(UINT8 op,UINT8 status)
{
	statusled.gpio=GPIO_STATUS_LED;

	if(op){
		statusled.status&=(~status);
	}else{
		statusled.status|=status;
	}


	if(statusled.status&DEV_STATUS_NOSERIALCODE){
		statusled.flag=0;
		statusled.interval=CLOCK_SECOND/5;	
		cust_timer_set(&statusled.timer, statusled.interval);			
	}else if(statusled.status&DEV_STATUS_CONN){
		statusled.flag=1;
	}else{
		statusled.flag=0;
		statusled.interval=CLOCK_SECOND;	
		cust_timer_set(&statusled.timer, statusled.interval);	
	}
	IoT_gpio_output(statusled.gpio,statusled.flag);	
}

void status_led_process()
{
	if(statusled.status!=DEV_STATUS_CONN&&cust_timer_expired(&statusled.timer)){

		cust_timer_reset(&statusled.timer);
		statusled.flag=statusled.flag?0:1;
		IoT_gpio_output(statusled.gpio,statusled.flag);
	}else if(cust_timer_expired(&dataRecvStatusTimer)&&ledFlashCnt){
		ledFlashCnt--;	
		if(ledFlashCnt){
			cust_timer_reset(&dataRecvStatusTimer);
			statusled.flag=statusled.flag?0:1;
		}else{
			statusled.flag=1;
		}
		IoT_gpio_output(statusled.gpio,statusled.flag);	
	}
}

void status_led_dataRecv()
{
	cust_timer_set(&dataRecvStatusTimer, 50);
	ledFlashCnt=6;
}


void macStrConvert()
{
	sprintf((char *)gCurrentMAC,"%02X%02X%02X%02X%02X%02X",
		gCurrentAddress[0],gCurrentAddress[1],
		gCurrentAddress[2],gCurrentAddress[3],
		gCurrentAddress[4],gCurrentAddress[5]);
}

void parseMacString(char *cp,uint8 * mac){
	uint8 temp=0;
	int i,j,m;
	
	m=strlen(cp);
	m=min(m,MAC_ADDR_LEN*2);
	j=0;
	for(i=0;i<m;i+=2){
		mac[j++]=_hexToByte(cp+i,min(m-i,2));		
	}
}

#if 0
void externalPWMSet(uint16 brightness)
{
	uint8 value;
	if(brightness>10||brightness==0){
		value=10;
	}else{
		value=brightness;
	}

	watchdogfeed.dutyvalue=(CLOCK_SECOND/2)/value;
	watchdogfeed.freevalue=(CLOCK_SECOND/2)-(CLOCK_SECOND/2)/value;
}


void watchDogInit()
{
	watchdogfeed.dutyvalue=CLOCK_SECOND/2;
	watchdogfeed.freevalue=CLOCK_SECOND/2;
	watchdogfeed.pulseflag=1;
	cust_timer_set(&watchdogfeed.timer, watchdogfeed.dutyvalue);
}


void feedWatchDog(){
	if(cust_timer_expired(&watchdogfeed.timer)){	
		if(watchdogfeed.pulseflag){
			cust_timer_set(&watchdogfeed.timer, watchdogfeed.freevalue);
			watchdogfeed.pulseflag=0;
		}else{
			cust_timer_set(&watchdogfeed.timer, watchdogfeed.dutyvalue);
			watchdogfeed.pulseflag=1;
		}
		IoT_gpio_output(GPIO_WATCH_DOG,watchdogfeed.pulseflag);
	}
}


void key_detect_init()
{
	gpioKey.gpio=GPIO_KEY;
	gpioKey.pressed=0;
	gpioKey.released=0;
	gpioKey.preStatus=0;
	cust_timer_set(&gpioKey.timer, CLOCK_SECOND/50);
}

void key_detect()
{
	UINT32 inputStatus;
	IoT_gpio_input(gpioKey.gpio,&inputStatus);


	if(inputStatus!=gpioKey.preStatus){
		cust_timer_restart(&gpioKey.timer);
	}

	if(inputStatus==0){
		if(cust_timer_expired(&gpioKey.timer)&&gpioKey.released){
			gpioKey.pressed=1;
			gpioKey.released=0;
		}
	}else{
		gpioKey.released=1;
	}
	
	gpioKey.preStatus=inputStatus;
}

#endif
