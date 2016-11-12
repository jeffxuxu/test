#include "types.h"
#include "iot_api.h"
#include "..\utils\xutils.h"
#include "..\cJSON\cJSON.h"
#include "dev.h"
#include "light.h"
#include "..\network\xlite.h"
#include "..\xJSON\xJSON.h"


extern INT32 gLightReportOption;
extern INT32 gWifiOutletAckOption;
extern struct timer udp_client_reporttimer;
extern UINT8 gLightSetRetxCnt;
extern UINT32 gLightSetOption;
extern IoTLightStatus gLightStatus;
extern IOT_ADAPTER   	IoTpAd;
extern INT8 gSMNTflag;


extern UINT32 gWifiOutletSetOption;
extern IoT_WifiOutletStaus gWifiOutletStatus;
extern IoTWifiOutletCfg gWifiOutletCfg;
extern UINT16 TIMERCLOCK[16];
extern Timer_Cust update_timer;

extern STA_ADMIN_CONFIG *pIoTStaCfg;

#if (ATCMD_RECOVERY_SUPPORT != 1)
void onDeviceInit()
{
#if 0	
		watchDogInit();
#endif
	xMemInit();
	macStrConvert();
	//FOTA_Init();
#if __PID_LIGHT__	
	lightInit();
#endif
	
	status_led_set(STATUS_CLR,DEV_STATUS_CONN);

}

void onDeviceProcessTask()
{
#if 0
	feedWatchDog();
	key_detect();
#endif

	status_led_process();

#if __PID_LIGHT__		
	lightProcess();
#endif

#if CFG_SUPPORT_DNS	
	udp_client_process();
#endif
}


void sharpCommandSend(int type, int subtype,INT8 * cmdbody,INT8 cmdlen){
	INT8 i,index;
	UINT8 checksum=0;
	CHAR shcmd[SH_CMD_STD_LEN+2+1]={0};

	shcmd[0]='#';
	shcmd[SH_CMD_STD_LEN+1]='#';

	index=1;

	_convertToHex(&shcmd[index],&type,1);
	index+=SH_CMD_TYPE_LEN;

//	_convertToHex(shcmd[index],&subtype,1);
//	index+=SH_CMD_SUBTYPE_LEN;
	if(NULL!=cmdbody){
		_convertToHex(&shcmd[index],cmdbody,cmdlen);	
		index+=SH_CMD_BODY_LEN;
	}
	for(i=index;i<SH_CMD_STD_LEN-1;i++){
		shcmd[i]=0x30;
	}
	for(i=1;i<SH_CMD_STD_LEN-1;i++){
		checksum+=shcmd[i];
	}

	_convertToHex(&shcmd[SH_CMD_STD_LEN-1],&checksum,1);
	IoT_uart_output(shcmd,SH_CMD_STD_LEN+2+1);
}


void onDeviceConnected()
{
#if (!CFG_SUPPORT_DNS)
	udp_client_connect();
#endif
	if(1 == gSMNTflag){
		udp_broadcast_start();
		gSMNTflag = 0;
	}
	udp_local_listen();
	status_led_set(STATUS_SET,DEV_STATUS_CONN);
}

void onDeviceDisconnected() 
{
	status_led_set(STATUS_CLR,DEV_STATUS_CONN);	
}


INT32 IoT_AppFunction_Ctrl(INT32 chid,INT32 power,INT32 brtness,INT32 color)
{
#if __PID_LIGHT__	
	if(LIGHT_CHANNEL_ID==chid){
		gLightReportOption|=LIGHT_OPT_REPORT;
		timer_restart(&udp_client_reporttimer);			
		gLightSetRetxCnt=3;
		gLightStatus.chid=chid;
		gLightSetOption|=LIGHT_OPT_SET;

		if(power!=-1){
			gLightStatus.power=power;
			gLightSetOption|=LIGHT_OPT_POWER;
		}

		if(brtness!=-1){
			gLightStatus.brightness=brtness;
			gLightSetOption|=LIGHT_OPT_BRIGHTNESS;
		}

		if(color!=-1){
			gLightStatus.color=color;
			gLightSetOption|=LIGHT_OPT_COLOR;
		}
	}
#endif


	return 0;
}

void statusReportPacketBuild(UCHAR * respBuff,UINT32 * respLen)
{
	xJSON msg;
	uint8 connect=0;

	//gLightReportOption|=LIGHT_OPT_REPORT;
	xJSON_InitObject(&msg,respBuff);
		xJSON_ArrayStart(&msg, KEY_REPORT,0);
			xJSON_ObjectStart(&msg, NULL ,0);
				xJSON_putInt(&msg, KEY_CHANNEL, LIGHT_CHANNEL_ID);
				xJSON_ObjectStart(&msg, KEY_STAT ,1);
#if __PID_LIGHT__

				if((gLightReportOption&LIGHT_OPT_POWER)||(gLightReportOption&LIGHT_OPT_REPORT)){
					xJSON_putInt(&msg, KEY_SWITCH, gLightStatus.power);
				}
				if((gLightReportOption&LIGHT_OPT_BRIGHTNESS)||(gLightReportOption&LIGHT_OPT_REPORT)){
					xJSON_putInt(&msg, KEY_BRTNESS, gLightStatus.brightness);
				}
				if((gLightReportOption&LIGHT_OPT_COLOR)||(gLightReportOption&LIGHT_OPT_REPORT)){
					xJSON_putInt(&msg, KEY_COLOR, gLightStatus.color);
				}
#endif			
					xJSON_ObjectComplete(&msg);
				xJSON_ObjectComplete(&msg);	
			xJSON_ArrayComplete(&msg);
		xJSON_ObjectComplete(&msg);
		*respLen=msg.length;	
	gLightReportOption=0;
}

void statusQueryPacketBuild(UCHAR * respBuff,UINT32 * respLen)
{
	xJSON msg;
	uint8 connect=0;

	xJSON_InitObject(&msg,respBuff);
		xJSON_ArrayStart(&msg, KEY_RESULT,0);
			xJSON_ObjectStart(&msg, NULL ,0);
				xJSON_putInt(&msg, KEY_CHANNEL, LIGHT_CHANNEL_ID);
				xJSON_ObjectStart(&msg, KEY_STAT ,0);
#if __PID_LIGHT__
				if(gLightReportOption&LIGHT_OPT_POWER){
					xJSON_putInt(&msg, KEY_SWITCH, gLightStatus.power);
				}
				if(gLightReportOption&LIGHT_OPT_BRIGHTNESS){
					xJSON_putInt(&msg, KEY_BRTNESS, gLightStatus.brightness);
				}
				if(gLightReportOption&LIGHT_OPT_COLOR){
					xJSON_putInt(&msg, KEY_COLOR, gLightStatus.color);
				}
#endif			
					xJSON_ObjectComplete(&msg);
				xJSON_ObjectComplete(&msg);	
			xJSON_ArrayComplete(&msg);
		xJSON_ObjectComplete(&msg);
		*respLen=msg.length;	
		
		gLightReportOption=0;
}


void onDeviceTrigger(UCHAR * respBuff,UINT32 * respLen)
{
	/*
	{
		"triggerid":"xxxxxxxx"
	}
	*/
	xJSON msg;
	uint8 connect=0;

	
	
}

void onDeviceStatusQuery(UCHAR * respBuff,UINT32 * respLen)
{
	statusQueryPacketBuild(respBuff,respLen);
}

void onDeviceStatusReport(UCHAR * respBuff,UINT32 * respLen)
{
	statusReportPacketBuild(respBuff,respLen);
}

void onDataReceived(UINT32 opCode,cJSON *json)
{
	int32 chid,brtness,color,power;
	cJSON * objItem=NULL,*arrayItem=NULL,*childItem=NULL,*childarray=NULL;
	INT32 i,j,count,countattr,hostIP;
	PCHAR pStr=NULL;
	UINT16 interval;

	if(NULL==json){
		if(OPCODE_TYPE_STAT_REQ==opCode){
			#if __PID_LIGHT__
			gLightReportOption=(LIGHT_OPT_POWER|LIGHT_OPT_BRIGHTNESS|LIGHT_OPT_COLOR|LIGHT_OPT_CTEMP);
			#endif
		}
		return;
	}

		switch(opCode){

			case OPCODE_TYPE_STAT_REQ:
				Printf_High("sreg\n");
				gLightReportOption=0;	
				arrayItem=cJSON_GetObjectItem(json,KEY_STAT_QUERY);
				if(NULL!=arrayItem){
					count=cJSON_GetArraySize(arrayItem);
					
				for(i=0;i<count;i++){
					objItem=cJSON_GetArrayItem(arrayItem,i);
					if(NULL!=objItem){
						chid=cJSON_GetInt(objItem, KEY_CHANNEL, 0);
						childarray=cJSON_GetObjectItem(objItem,KEY_ATTR_R);
						if(NULL!=childarray){
							countattr=cJSON_GetArraySize(childarray);
							for(j=0;j<countattr;j++){
								childItem=cJSON_GetArrayItem(childarray,j);
								if(!memcmp(childItem->valuestring,VALUE_SWITCH,strlen(VALUE_SWITCH))){
									gLightReportOption|=LIGHT_OPT_POWER;
								}else if(!memcmp(childItem->valuestring,VALUE_BRTNESS,strlen(VALUE_BRTNESS))){
									gLightReportOption|=LIGHT_OPT_BRIGHTNESS;
								}else if(!memcmp(childItem->valuestring,VALUE_COLOR,strlen(VALUE_COLOR))){
									gLightReportOption|=LIGHT_OPT_COLOR;
								}
							}
						}
					}
				}
				}
				break;

			case OPCODE_TYPE_CTRL_REQ:
				Printf_High("cfg\n");
				arrayItem=cJSON_GetObjectItem(json,KEY_CTRL);
				if(NULL!=arrayItem){
					count=cJSON_GetArraySize(arrayItem);	
				for(i=0;i<count;i++){
					objItem=cJSON_GetArrayItem(arrayItem,i);
					if(NULL!=objItem){
						chid=cJSON_GetInt(objItem, KEY_CHANNEL, 0);
						childItem=cJSON_GetObjectItem(objItem,KEY_ATTR_W);
						if(NULL!=childItem){
							power=cJSON_GetInt(childItem,KEY_SWITCH,-1);
							brtness=cJSON_GetInt(childItem,KEY_BRTNESS,-1);
							color=cJSON_GetInt(childItem,KEY_COLOR,-1);
							Printf_High("chid:%d,power:%d,brtness:%d,color:%x\n",chid,power,brtness,color);
							IoT_AppFunction_Ctrl(chid,power,brtness,color);	
						}
					}
				}
				
				}
				break;

			case OPCODE_TYPE_CFG_REQ:
				Printf_High("cfg reg\n");
				objItem=cJSON_GetObjectItem(json,KEY_CONF);
				if(NULL!=objItem)
				{
					pStr=cJSON_GetString(objItem, KEY_SSID);
					memset(pIoTStaCfg->Ssid, '\0', sizeof(pIoTStaCfg->Ssid));
					memcpy(pIoTStaCfg->Ssid, pStr,  min(strlen(pStr), sizeof(pIoTStaCfg->Ssid)));
					
					pStr=cJSON_GetString(objItem, KEY_PASSPHRASE);
					memset(pIoTStaCfg->Passphase, '\0', sizeof(pIoTStaCfg->Passphase));
					memcpy(pIoTStaCfg->Passphase, pStr,  min(strlen(pStr), sizeof(pIoTStaCfg->Passphase)));
					
					hostIP=atoi(cJSON_GetString(objItem, KEY_HOST));
					IoTpAd.ComCfg.IoT_ServeIP[0] = (hostIP>>24)&&0xff;
					IoTpAd.ComCfg.IoT_ServeIP[1] = (hostIP>>16)&&0xff;
					IoTpAd.ComCfg.IoT_ServeIP[2] = (hostIP>>8)&&0xff;
					IoTpAd.ComCfg.IoT_ServeIP[3] = hostIP&&0xff;
					
					pStr=cJSON_GetString(objItem, KEY_PORT);
					IoTpAd.ComCfg.IoT_UDP_Srv_Port = atoi(pStr);
					
					interval=atoi(cJSON_GetString(objItem, KEY_INTERVAL));
					
					pStr=cJSON_GetString(objItem, KEY_PASSWD);
					memset(IoTpAd.ComCfg.CmdPWD, '\0', sizeof(IoTpAd.ComCfg.CmdPWD));
					memcpy(IoTpAd.ComCfg.CmdPWD, pStr,  min(strlen(pStr), sizeof(IoTpAd.ComCfg.CmdPWD)));
					
					IoTpAd.ComCfg.IoT_UDP_Srv_Interval=interval;
					store_sta_cfg();
					restore_com_cfg();	
				}

				objItem=cJSON_GetObjectItem(json,KEY_LABEL);
				if(NULL!=objItem)
				{
					chid=cJSON_GetInt(objItem, KEY_CHANNEL, 0);
					pStr=cJSON_GetString(objItem, KEY_CHANNEL_NAME);
					memset(IoTpAd.UsrCfg.ProductName, '\0', sizeof(IoTpAd.UsrCfg.ProductName));
					memcpy(IoTpAd.UsrCfg.ProductName, pStr,  min(strlen(pStr), sizeof(IoTpAd.UsrCfg.ProductName)));
					
				}
			break;

			case OPCODE_TYPE_TRIG_REQ:
				Printf_High("trig\n");
				objItem=cJSON_GetObjectItem(json,KEY_TRIGGER);
				if(NULL!=objItem)
				{
					childItem=cJSON_GetObjectItem(objItem,KEY_IF);
					if(NULL!=childItem)
					{
						arrayItem=cJSON_GetObjectItem(childItem,KEY_COND);
						if(NULL!=arrayItem)
						{
							count=cJSON_GetArraySize(arrayItem);	
							Printf_High("count1:%d\n",count);
							for(i=0;i<count;i++)
							{
								//if()
							}
						}
					}
				}
				break;

	}
}
#else
void onDeviceInit(){}
void onDeviceProcessTask(){}
void onDeviceConnected(){}
void onDeviceDisconnected() {}
INT32 IoT_AppFunction_Ctrl(INT32 chid,INT32 power,INT32 brtness,INT32 color)	{return 0;}
INT32 IoT_AppFunction_Query(INT32 pID,INT32 pt,INT32 pn){return 0;}
void onDataReceived(UINT32 opCode,cJSON *json){}
#endif
