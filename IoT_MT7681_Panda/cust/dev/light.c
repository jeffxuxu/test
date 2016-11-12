#include "iot_api.h"
#include "iot_custom.h"
#include "light.h"
#include "..\network\xlite.h"
#include "dev.h"
#include "..\utils\cust_timer.h"

UINT32 gLightReportOption=0;
UINT32 gLightSetOption=0;
UINT8 gLightSetRetxCnt=0;
UINT8 gWifiOutletSetRetxCnt = 0;
UINT32 gTriggerOption=0;

IoTLightStatus gLightStatus;

static Timer_Cust lightPollTimer;

#if __PID_LIGHT__	

void lightInit()
{
	gLightStatus.power=1;
	gLightStatus.brightness=255;
	gLightStatus.color=0x00FFFFFF;
	gLightStatus.ctemp=4592;
	gLightReportOption|=LIGHT_OPT_REPORT;
	cust_timer_set(&lightPollTimer,CLOCK_SECOND/10);
	sharpCommandSend(SHCMD_LGT_STATUS_QUERY, 0,NULL,0);
}

void lightProcess()
{
	INT8 cmdbody[SH_CMD_BODY_LEN/2]={0};
	INT8 cmdType,index=0;

	if(cust_timer_expired(&lightPollTimer)&&(gLightSetOption&LIGHT_OPT_SET)&&gLightSetRetxCnt){	
		cust_timer_restart(&lightPollTimer);
		switch(gLightStatus.chid){
			case LIGHT_CHANNEL_ID:
				cmdType=SHCMD_LGT_POWER;
				cmdbody[index]=(gLightStatus.power>0);
				index++;			
				
				cmdbody[index]=gLightStatus.brightness;
				cmdbody[index+1]=gLightStatus.ctemp;
				cmdbody[index+2]=(gLightStatus.color>>16)&0x000000FF;
				cmdbody[index+3]=(gLightStatus.color>>8)&0x000000FF;
				cmdbody[index+4]=gLightStatus.color&0x000000FF;		
				sharpCommandSend(cmdType, 0,cmdbody,SH_CMD_BODY_LEN/2);	
		}
		gLightSetRetxCnt--;	
		if(gLightSetRetxCnt==0){
			gLightSetOption=0;
		}
	}
}
#endif
