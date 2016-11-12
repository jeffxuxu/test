#include "iot_api.h"
#include "iot_custom.h"
#include "..\xMem\xmem.h"
#include "xJSON.h"
#include "..\network\xlite.h"

extern IOT_ADAPTER   IoTpAd;
extern STA_FOTA_CONFIG IoTStaFOTACfg;
void xJSON_InitObject(xJSON * PJSON,char * buff)
{
	PJSON->string=buff;
	PJSON->string[0]='{';
	PJSON->length=1;
}

void xJSON_putInt(xJSON * json,char * key,int value)
{
	int len=0;
	if(NULL==json||NULL==key)
		return;

	sprintf(&json->string[json->length],"\"%s\":%d,",key,value);
	json->length+=strlen(key)+3;	
	if(value<0){
		len++;
		value=0-value;
	}
	do{
		value/=10;
		len++;
	}while(value>0);
	json->length+=len+1;
}

void xJSON_putString(xJSON * json,char * key,char * value)
{
	if(NULL==json||NULL==key||NULL==value)
		return;

	sprintf(&json->string[json->length],"\"%s\":\"%s\",",key,value);
	json->length+=strlen(key)+3+strlen(value)+3;	
}

void xJSON_ObjectStart(xJSON * json,char * key,UCHAR conn)
{
	if(NULL==json)
		return;
	if(conn!=0){
		json->string[json->length]=',';
		json->length++;			
	}
	if(NULL!=key){
		sprintf(&json->string[json->length],"\"%s\":",key);
		json->length+=strlen(key)+3;
	}
	json->string[json->length]='{';
	json->length++;	
}

void xJSON_ObjectComplete(xJSON * json)
{
	if(json->string[json->length-1]==','){
		json->string[json->length-1]='}';
	}else{
		json->string[json->length]='}';
		json->length++;	
	}
}

void xJSON_ArrayStart(xJSON * json,char * key,UCHAR conn)
{
	if(NULL==json||NULL==key)
		return;
	if(conn!=0){
		json->string[json->length]=',';
		json->length++;			
	}
	sprintf(&json->string[json->length],"\"%s\":[",key);
	json->length+=strlen(key)+3+1;
}


void xJSON_ArrayComplete(xJSON * json)
{
	json->string[json->length]=']';
	json->length++;	
}
