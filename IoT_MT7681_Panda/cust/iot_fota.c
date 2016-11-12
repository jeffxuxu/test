/******************************************************************************
* MODULE NAME:     iot_xmodem.c
* PROJECT CODE:    __MT7681__
* DESCRIPTION:     
* DESIGNER:        
* DATE:            Jan 2013
*
* SOURCE CONTROL:
*
* LICENSE:
*     This source code is copyright (c) 2011 Ralink Tech. Inc.
*     All rights reserved.
*
* REVISION     HISTORY:
*   V1.0.0     Jan 2012    - Initial Version V1.0
*
*
* SOURCE:
* ISSUES:
*    First Implementation.
* NOTES TO USERS:
*
******************************************************************************/
#if (ATCMD_RECOVERY_SUPPORT == 1)


#include <stdio.h>
#include "string.h"
#include "flash_map.h"
#include "iot_api.h"
#include "eeprom.h"

extern STA_FOTA_CONFIG IoTStaFOTACfg;
#define FOTA_LOADING_SIZE	128
int IoT_OTA_Update_FW(VOID)
{
	uint8 xbuff[FOTA_LOADING_SIZE]; 
	int crc = 1;
	int len = 0;
	uint32 pktNum = 0;		  /*if packetno >255 , use this parameter*/
	
	int flash_error_count = 0;
	uint8 flash_write_result = 0;
	uint8  UartUpgHeader[UART_FW_HEADER_TOTAL_SIZE] = UART_FW_HEADER;
	int    UpgSize  = FLASH_UPG_FW_SIZE;
	uint8* pData	= NULL;
	uint8  type		= 0;

	while(1){
		int count = UpgSize - len;
		
		spi_flash_read((UINT32)(FLASH_OFFSET_AP_FW_START+pktNum*FOTA_LOADING_SIZE), xbuff, FOTA_LOADING_SIZE);
		pData = &xbuff[0];
		if (count > FOTA_LOADING_SIZE)
			count = FOTA_LOADING_SIZE;
		
		if (count > 0)
	  	{
			pktNum ++;
			
			if (pktNum == 1)
			{	
				/*compare first 5 bytes */
				if(!memcmp(pData, UartUpgHeader, UART_FW_HEADER_DATA_SIZE))
				{
					//UpgSize  = *((uint32 *)(pData+UART_FW_HEADER_DATA_SIZE));
					UpgSize  =(((uint32)(*(pData + UART_FW_HEADER_DATA_SIZE))) 	         | 
								((uint32)(*(pData + UART_FW_HEADER_DATA_SIZE + 1)) << 8) |
								((uint32)(*(pData + UART_FW_HEADER_DATA_SIZE + 2)) << 16)|
								((uint32)(*(pData + UART_FW_HEADER_DATA_SIZE + 3)) << 24));

					
					/*The 8st Byte of UartUpgHeader[] is update Type */
					type = pData[UART_FW_HEADER_DATA_SIZE + UART_FW_HEADER_LEN_SIZE];
					if ((type == 0) || (type >= UART_FlASH_UPG_ID_MAX)||UpgSize!=(IoTStaFOTACfg.imagesize-128))
					{
						Printf_High("type or size error,%d,%d,%d\n",type,UpgSize,IoTStaFOTACfg.imagesize);
						return -1;						
					}						
				}else{
					Printf_High("header error\n");
					return -2;					
				}
			}
			else
			{
				flash_write_result = spi_flash_update_fw(type, FOTA_LOADING_SIZE*(pktNum-2), pData, (uint16)count);

				if(flash_write_result != 0)
					flash_error_count++;

				len += count;
			}
		}	
		if(len%10240==0||len==UpgSize){
			Printf_High("[Update]%u/%u\n",len,UpgSize);
		}
		if(count<FOTA_LOADING_SIZE){
			if(flash_error_count == 0){	
				spi_flash_update_fw_done(type);
				return 0;				
			}else{
				Printf_High("\nErr:%d\n", flash_error_count);
				return -3;
			}
		}
	}
}

#endif
