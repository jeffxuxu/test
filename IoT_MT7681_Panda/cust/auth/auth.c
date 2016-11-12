#include <stdio.h>
#include "string.h"
#include "types.h"
#include "iot_api.h"
//#include "Xlite.h"

#define MAJOR		1
#define MINOR		2
#define REVISION		3
#define FW_VERSION	(MAJOR<<8+MINOR<<4+REVISION)
#define PRODUCT_ID	0xABCD
#define VENDOR_NAME	"likeabird"

#define EFUSE_SERIAL_OFFSET		0x52
#define SERAIL_LEN				4
#define EFUSE_VER_OFFSET		0x56
#define VER_LEN					2


#define VERCODE_LEN	6
#define DEVID_LEN	16
#define NUM_MAX		0x7FFFFFF
#define NUM_MIN		0x1220CEB
#define CHECK_DIGIT_VECTOR	
#define TO_BIGENDDIAN(x)	((x)>>24|(((x)&0xFF)<<24)|(((x)&0xFF00)<<8)|(((x)&0xFF0000)>>8))
//F5E3D4C1A2B0C0DB

static char vercode[VERCODE_LEN+1]={0};
static char devid[DEVID_LEN+1]={0};
char * getVersionCode()
{
	if(vercode[0]==0)
		sprintf(vercode,"%d.%d.%d",MAJOR,MINOR,REVISION);

	return vercode;
}


char * getDeviceID()
{
	uint8 i;
	uint32 bigenddiannum,checkdigit;
	uint8 id[DEVID_LEN]={0};
	uint8 vector[DEVID_LEN-1]={1, 6, 11, 2, 5, 7, 10, 12, 3, 14, 15,  8, 13, 4, 9};
	uint8 major,minor,revision;
	uint32 num;
	uint16 ver;
	
	serial_read(&num,&ver);
	
	//Printf_High("devid[0]:%d,num:%x\n",devid[0],num);
	
	if((devid[0]==0)/*&&(((num<=NUM_MAX)&&(num>=NUM_MIN))||(num==0))*/){
		major=(ver>>8);
		minor=((ver&0xFF)>>4);
		revision=(ver&0x0F);		
		bigenddiannum=((TO_BIGENDDIAN(num)>>4)|((num&0x0F000000)>>24));
		id[0]=(0xFF-major)>>4;		id[1]=(bigenddiannum>>24)&0x0F,
		id[2]=(0xFF-major)&0x0F;	id[3]=(bigenddiannum>>20)&0x0F,
		id[4]=(0x0F-minor)&0x0F;	id[5]=(bigenddiannum>>16)&0x0F,
		id[6]=(0x0F-revision)&0x0F;	id[7]=(bigenddiannum>>12)&0x0F,
		id[8]=(PRODUCT_ID>>12)&0x0F;id[9]=(bigenddiannum>>8)&0x0F,
		id[10]=(PRODUCT_ID>>8)&0x0F;id[11]=(bigenddiannum>>4)&0x0F,
		id[12]=(PRODUCT_ID>>4)&0x0F;id[13]=(bigenddiannum)&0x0F,
		id[14]=(PRODUCT_ID)&0x0F;

		checkdigit=0;
		for(i=0;i<15;i++){
			checkdigit+=(id[i]*vector[i]);
			
			sprintf(devid+i,"%X",id[i]);
		}
		id[15]=(checkdigit%15);
		sprintf(devid+15,"%X",id[15]);
	}
	return devid;
}


void serial_write(uint32 num,uint16 ver)
{
	uint8 i;
	for(i=0;i<SERAIL_LEN;i++);
		Set_ATE_Efuse_Write(EFUSE_SERIAL_OFFSET+i,num>>(i*8));

	for(i=0;i<VER_LEN;i++)
		Set_ATE_Efuse_Write(EFUSE_VER_OFFSET+i,num>>(i*8));
}

void serial_read(uint32 * num,uint16 * ver)
{
	uint8 i,v;

	*num=0;
	for(i=0;i<SERAIL_LEN;i++){
		Set_ATE_Efuse_Read(EFUSE_SERIAL_OFFSET+i,&v);
		*num+=(((int)v)<<(i*8));
	}

	*ver=0;
	for(i=0;i<VER_LEN;i++){
		Set_ATE_Efuse_Read(EFUSE_VER_OFFSET+i,&v);
		*ver+=(((int)v)<<(i*8));
	}
	
}

void id_aes_ecb_decryt(
    IN  PUCHAR pCipter,
    IN  PUINT32 pCipterLen,
    OUT PUCHAR pPlain,
    INOUT PUINT32 pPlainLen)
{
    uint32 index = 0;
    uint8  Key[AES_BLOCK_SIZES] ={0};
    uint32 iPlainBlkLen = AES_BLOCK_SIZES;


	RT_MD5(VENDOR_NAME,strlen(VENDOR_NAME),Key);
	
    if ((*pCipterLen % AES_BLOCK_SIZES) != 0||*pPlainLen != *pCipterLen) {
        return;
    }

    for (index=0; index<(*pCipterLen/AES_BLOCK_SIZES); index++) {
        RT_AES_Decrypt(
            pCipter + (index*AES_BLOCK_SIZES),
            AES_BLOCK_SIZES,
            Key,
            AES_BLOCK_SIZES,
            pPlain + (index*AES_BLOCK_SIZES),
            &iPlainBlkLen);
    }
}

#if 0
bool isHexString(char * str){
	while(*(str++)){
		if(!((*str>='A'&&*str<='F')||(*str>='a'&&*str<='f')||(*str>='0'&&*str<='9')))
			return FALSE;
	}

	return TRUE;
}
#endif

uint32 _HexToDecimal(char *cp,uint8 len)
{
	uint8 i,n;
	uint32 temp=0;

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

extern char *optarg;
uint16 IoT_exec_AT_cmd_Serial(PUCHAR pCmdBuf, INT16 at_cmd_len)
{
	char* argv[MAX_OPTION_COUNT];
	char *opString = "i:?";
	uint32 len,i;
	uint16 argc = 0;
	uint8 opt = 0;
	uint32 num;
	uint16 ver;
	uint8 plaintext[AES_BLOCK_SIZES]={0};

	split_string_cmd(pCmdBuf, at_cmd_len, &argc, argv);

	opt = getopt(argc, argv, opString);

	while (opt != -1)
	{	
		switch (opt)
		{
			case 'i':
				len=strlen(optarg);
				serial_read(&num,&ver);
				if((num!=0xFFFFFFFF)&&(num!=0)){
					//IoT_uart_output("SerialHaveWritten\n",18);
					//return 1;
				}
				
				if(AES_BLOCK_SIZES==len){
					memset(devid,0,DEVID_LEN+1);
					id_aes_ecb_decryt(optarg,len,plaintext,AES_BLOCK_SIZES);
					if(isHexString(plaintext)){
						num=_HexToDecimal(&plaintext[8],8);
						if(((num+_HexToDecimal(&plaintext[0],8))==0xFFFFFFFF)&&(num<=NUM_MAX)&&(num>=NUM_MIN)){
							serial_write(num,FW_VERSION);
							IoT_uart_output("SerialWriteSuccess\n",19);
							return 0;
						}
					}
				}
			case '?':
			default:
				break;
		}
		opt = getopt(argc, argv, opString);
	}
	IoT_uart_output("SerialWriteFailed\n",18);
	return 1;
}


