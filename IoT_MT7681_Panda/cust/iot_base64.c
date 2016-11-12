#include <stdio.h>
#include "string.h"
#include "types.h"
#include "iot_api.h"




#if 0
void Base64Encode(const char *src, char *dst, int src_len)
{

	 int i = 0, j = 0;
	 
	 
	 for (i; (src_len - i) / 3; i += 3) 
	 {
		 dst[j++] = (src[i] >> 2) & 0x3f;
		 dst[j++] = ((src[i] << 4) | (src[i + 1] >> 4)) & 0x3f;
		 dst[j++] = ((src[i + 1] << 2) | (src[i + 2] >> 6 )) & 0x3f;
		 dst[j++] = src[i + 2] & 0x3f;
	 }
	 
	 /*不足3的倍数*/
	 
	 if (src_len % 3 == 1) 
	 {
		 dst[j++] = (src[i] >> 2) & 0x3f;
		 dst[j++] = (src[i] << 4) & 0x3f;
		 dst[j++] = '=';
		 dst[j++] = '=';
	 }
	 else if (src_len % 3 == 2) 
	 {
		 dst[j++] = (src[i] >> 2) & 0x3f;
		 dst[j++] = ((src[i] << 4) | (src[i + 1] >> 4)) & 0x3f;
		 dst[j++] = (src[i + 1] << 2) & 0x3f;
		 dst[j++] = '=';
	}

	dst[j] = '\0';

}



//若是不是4的倍数，强制解码？

void Base64Decode(const char *src, char *dst, int src_len)
{

	 int i = 0, j = 0;
	 
	 if(src_len % 4 != 0)
	 {
		 Printf_High("sound code is not four time\n");
		 return;
	 }
	 
	 for (i; (src_len - i) / 4; i += 4) 
	 {
		 dst[j++] = src[i] << 2 | src[i + 1] >> 6;
		 dst[j++] = src[i + 1]<< 4 | src[i + 2] >> 2;
		 dst[j++] = src[i + 2]<< 6 | src[i + 3];
	 }
	 
	 dst[j] = '\0';

}
#endif


void Base64Encode(char* src, char* dst, int datalen){
	char* base64code;
	char inbuff[3];
	char outbuff[4];
	int i,j;
	int base64len;
	
	base64len=((datalen+2)/3)*4;
	
	for(i=0,j=0;i<datalen;i+=3,j+=4){
		inbuff[0]=inbuff[1]=inbuff[2]=0;
		memcpy(inbuff, src+i, datalen-i>3?3:datalen-i);
		outbuff[0]=(char) (0x80|inbuff[0]>>2);
		outbuff[1]=(char) (0x80|((inbuff[0]&0x03)<<4)|((inbuff[1]&0xF0)>>4));
		outbuff[2]=(char) (0x80|((inbuff[1]&0x0F)<<2)|((inbuff[2]&0xC0)>>6));
		outbuff[3]=(char) (0x80|(inbuff[2]&0x3F));
		memcpy(dst+j, outbuff, 4);
	}
	
}
    
void Base64Decode(char* src,char* dst, int len){
	int i,j;
	int datalen;
	char inbuff[4];
	char outbuff[3];
	
	datalen=(len/4)*3;
	
	for(i=0,j=0;j<len;i+=3,j+=4){
		inbuff[0]=inbuff[1]=inbuff[2]=inbuff[3]=0;
		memcpy(inbuff, src+j, 4);
		outbuff[0]=(char) ((0x3F&inbuff[0])<<2|((0x30&inbuff[1])>>4));
		outbuff[1]=(char) (((inbuff[1]&0x0F)<<4)|((inbuff[2]&0x3F)>>2));
		outbuff[2]=(char) (((inbuff[2]&0x03)<<6)|(inbuff[3]&0x3F));
		memcpy(dst+i, outbuff, 3);
	}
	return datalen;
}




