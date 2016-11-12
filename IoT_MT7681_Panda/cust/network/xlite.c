#include "uip.h"
#include "uip_timer.h"
#include "iot_api.h"
#include "xlite.h"

uint16 atomic=0;

extern UINT16 preSequenceNum;
int32 XLite_Packet_Build(pLiteHeader  pPacketHeader,short opCode,short errCode,UINT8 encryptMode,int length, char * jsonstring,UINT8 Direction)
{
	char* pBase64Buf;
	uint16 BufLen;
	if(pPacketHeader==NULL)
		return;
	memset(pPacketHeader,0,IOT_XLITE_HEADER_LEN);
	//memcpy(pPacketHeader->magic,XLITE_MAGIC,4);
	pPacketHeader->magic=XLITE_MAGIC;
	//Printf_High("send magic:%x\n",pPacketHeader->magic);
	pPacketHeader->version=XLITE_PROTOCOL_VER;
	pPacketHeader->encrypt=encryptMode;
	if(Direction==IOT_RESPONSE)
	{
		pPacketHeader->seqnum=preSequenceNum;
	}
	else
	{
		pPacketHeader->seqnum=atomic++;
	}
	pPacketHeader->opcode=opCode;
	pPacketHeader->timestamp=0;
	pPacketHeader->errcode=errCode;
	pPacketHeader->devtype=IOT_DEVICE;
	memcpy(pPacketHeader->devid,getDeviceID(),DEVICE_ID_LEN);

	#ifndef FOR_TEST
	if(encryptMode==ENCRYPT_MODE_BASE64){
		BufLen =BASE64_BUFF_SIZE_GET(IOT_CMD_HEADER_LEN+length);//for base64 encode
		pBase64Buf = (char*)malloc(BufLen);

		if(length>0&&NULL!=jsonstring)
			memcpy(((char *)pPacketHeader)+IOT_XLITE_HEADER_LEN,jsonstring,length);
		
		Base64Encode((char*)pPacketHeader+IOT_LITE_HEADER_LEN, pBase64Buf, IOT_CMD_HEADER_LEN+length);
		memcpy((char*)pPacketHeader+IOT_LITE_HEADER_LEN, pBase64Buf, BufLen);
		free(pBase64Buf);
	}
	else
	#endif
	{
		BufLen=length+IOT_CMD_HEADER_LEN;
	}
	
	
	pPacketHeader->length=BufLen;
	return IOT_LITE_HEADER_LEN+BufLen;
	//return IOT_XLITE_HEADER_LEN+length;
}