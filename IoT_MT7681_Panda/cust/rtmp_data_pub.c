/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_data.c

	Abstract:
	Data path subroutines

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/
#include <stdio.h>
#include "types.h"
#include "config.h"
#include "queue.h"
#include "tcpip_main.h"
#include "iot_api.h"
#if (ATCMD_ATE_SUPPORT == 1)
#include "ate.h"
#endif
#ifdef CONFIG_SOFTAP
#include "ap_pub.h"
#include "ap_auth.h"
#endif
#include "wpa_cmm.h"
#include "wifi_task.h"

/*****************************************************************
  Defination
******************************************************************/
#if (IOT_SMP_RSSI_COLLECT == 1)
RX_RSSI_INFO gRxRSSIInfo;
VOID Update_Rssi_Sample_Cust(IN RX_RSSI_INFO *pRssi, IN RXWI_STRUC *pRxWI);
#endif

/*****************************************************************
  Extern Paramter
******************************************************************/
extern MLME_STRUCT 	 *pIoTMlme;
extern UCHAR EAPOL[2];	//  = {0x88, 0x8e};
extern u8_t uip_sourceAddr[6];
#ifdef CONFIG_SOFTAP
extern UCHAR NUM_BIT8[8];
extern AP_ADMIN_CONFIG  *pIoTApCfg;
#endif

#if (ATCMD_ATE_SUPPORT == 1)
extern ATE_INFO gATEInfo;
#endif

/*****************************************************************
  Extern Function
******************************************************************/



/*****************************************************************
  Functions
******************************************************************/
VOID RestartBCNTimer(VOID)
{	
	cnmTimerStopTimer (&pIoTMlme->BeaconTimer);
	cnmTimerStartTimer(&pIoTMlme->BeaconTimer, BEACON_LOST_TIME);
}

/*
    ==========================================================================
    Description:
        function to be executed at timer thread when beacon lost timer expires
	IRQL = DISPATCH_LEVEL
    ==========================================================================
 */
/* Beacon loss timeout handler */
VOID BeaconTimeoutAction(UINT_32 param, UINT_32 param2) 
{
	IoT_Cmd_LinkDown(REASON_DISASSPC_AP_UNABLE);
}

#ifdef CONFIG_SOFTAP
VOID APHandleRxControlFrame(
	IN	RX_BLK			*pRxBlk)
{
	PHEADER_802_11	pHeader = pRxBlk->pHeader;
	PMAC_TABLE_ENTRY  pEntry;

	pEntry = MacTableLookup(pRxBlk->pHeader->Addr2);
	if(pEntry==NULL)
		return;
	
	switch (pHeader->FC.SubType)
	{
		/* handle PS-POLL here */
		case SUBTYPE_PS_POLL:
		{
			USHORT Aid = pHeader->Duration & 0x3fff;

			if(pEntry->Aid != Aid)
				return;		/**/
			
			#if 0
			if (Aid < MAX_LEN_OF_MAC_TABLE)
				APHandleRxPsPoll(pAd, pHeader->Addr2, Aid, FALSE);
			#endif

			#if 0	/*temprary cancel response NULL, in order to let station keep awake*/
			APSendNullFrame(pEntry->Addr, PWR_ACTIVE);
			#endif
			pEntry->NoDataIdleCount = 0;
			
			#ifndef IOT_BCN_TIM_KEEP_ON
			WLAN_MR_TIM_BIT_CLEAR(Aid);
			#endif
		}
		break;
		case SUBTYPE_ACK:
		default:		
			break;
	}

	return;
}


//
// All Rx routines use RX_BLK structure to hande rx events
// It is very important to build pRxBlk attributes
//  1. pHeader pointer to 802.11 Header
//  2. pData pointer to payload including LLC (just skip Header)
//  3. set payload size including LLC to DataSize
//  4. set some flags with RX_BLK_SET_FLAG()
// 
VOID APHandleRxDataFrame(
	RX_BLK*			pRxBlk)
{
#if (CFG_SUPPORT_TCPIP == 0)
	PUCHAR rawpacket;
	UINT16 rawpacketlength;
#endif

/*************************************************
*data frame parsing only in SMTCN and CONNECTED
**************************************************/
    PMAC_TABLE_ENTRY  pEntry = NULL;
	
	/*Only Handle the MyBSS with ToDS flag enable packets  for packet forwarding to the destination address*/
	if ((pRxBlk->pRxINFO->MyBss==1) && 
		(pRxBlk->pHeader->FC.ToDs==1) && (pRxBlk->pHeader->FC.FrDs==0))
	{
		/*if DestAddr(add3) is not same as BSSID MACAddr(add1), forwarding the Rx frame to DestAddr*/
		//if(memcmp(pRxBlk->pHeader->Addr3, pIoTApCfg->MBSSID.Bssid, MAC_ADDR_LEN))
		if(memcmp(pRxBlk->pHeader->Addr1, pRxBlk->pHeader->Addr3, MAC_ADDR_LEN))
		{
			/*forward packets*/
			AP_Legacy_Frame_Forward_Tx(pRxBlk, GetClearFrameFlag());

			/*Drop the packets which DA is not Broadcast/Multicast/BSSID address, after packet forwarding*/
			if ((pRxBlk->pHeader->Addr3[0] & 0x1) == 0)
			{
				printf("Drop packets after forwarding: 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x \n", 
					       PRINT_MAC(pRxBlk->pHeader->Addr3));
				return;
			}
		}		
	}

	//skip 802.11 and SNAP header
	RTMPRawDataSanity(pRxBlk);
	
#if CFG_SUPPORT_4WAY_HS
	//check is EAPOL package
	if (!memcmp(pRxBlk->pData, EAPOL, 2))
	{
		/* Confirm incoming is EAPOL-KEY frame */
		if( *(pRxBlk->pData + 3) == EAPOLKey)
		{	
			printf("EAPOL KEY +++\n");

			/*pData include 2Byte Protocol type(0x888E) of 802.2 LLC header, so delete Protocol type  */
			/*and only transfer Eapol data and len to  WpaEAPOLKeyAction()*/
			pRxBlk->pData	  += 2;   
			pRxBlk->DataSize  -= 2;

			/* process EAPOL KEY 4-way handshake and 2-way group handshake */
			WpaEAPOLKeyAction(pRxBlk);
		}
	}
	else
#endif
	{
		/* some AP will sent Broadcast/Muliticast Data frame */
		pEntry = MacTableLookup(pRxBlk->pHeader->Addr2);
		if (pEntry == NULL)
		{
			printf("%s,%d  ApSendDeauthAction  Rxfilter(0x%x): \n",
				__FUNCTION__,__LINE__,IoT_Get_RxFilter());
			//dump(pRxBlk->pHeader, sizeof(HEADER_802_11));

			/*Assume RX filter drop the notMyBssid Frame*/
			ApSendDeauthAction(pRxBlk->pHeader->Addr2, REASON_CLS3ERR);
		 	return;
		}
		
		pEntry->NoDataIdleCount = 0;

		#ifndef IOT_BCN_TIM_KEEP_ON
		WLAN_MR_TIM_BIT_CLEAR(pEntry->Aid);
		#endif
		
	 	if(pEntry->PortSecured == WPA_802_1X_PORT_SECURED)
	 	{
			printf("%s,%d  RX(Aid)=%d: Type/SubType:(0x%x,0x%x) \n",
				__FUNCTION__,__LINE__,pEntry->Aid, pRxBlk->pHeader->FC.Type, pRxBlk->pHeader->FC.SubType);
			//dump(pRxBlk->pData, pRxBlk->DataSize);
			
		    pRxBlk->pData	-= 6;
		    memcpy(pRxBlk->pData, pRxBlk->pHeader->Addr3, 6);
		    pRxBlk->DataSize += 6;

		    pRxBlk->pData	-= 6;
		    memcpy(pRxBlk->pData, pRxBlk->pHeader->Addr1, 6);
		    pRxBlk->DataSize += 6;

			#if CFG_SUPPORT_TCPIP
			netif_rx(pRxBlk->pData, pRxBlk->DataSize);
			#endif
	 	}
	}

	return;
}

VOID APHandleRxMgmtFrame(
	RX_BLK*			pRxBlk)
{
	//printf("BTYPE_MGMT subtype : %d\n", pRxBlk->pHeader->FC.SubType);
	switch (pRxBlk->pHeader->FC.SubType)
	{
		case SUBTYPE_PROBE_REQ:
			APSendProbeAction(pRxBlk);
			break;
			
		case SUBTYPE_AUTH:
			APPeerAuthReqAction(pRxBlk);
			Printf_High("%s,%d, SUBTYPE_AUTH: \n",__FUNCTION__,__LINE__);
			//dumpMacTable();
			break;

		case SUBTYPE_DEAUTH:
			APPeerDeauthReqAction(pRxBlk);

			Printf_High("%s,%d, SUBTYPE_DEAUTH: \n",__FUNCTION__,__LINE__);
			//dumpMacTable();
			break;

		case SUBTYPE_ASSOC_REQ:
			APAssocReqActionAndSend(pRxBlk, 0);
			Printf_High("%s,%d, SUBTYPE_ASSOC_REQ: \n",__FUNCTION__,__LINE__);
			break;

		case SUBTYPE_REASSOC_REQ:
			APAssocReqActionAndSend(pRxBlk, 1);
			Printf_High("%s,%d, SUBTYPE_REASSOC_REQ: \n",__FUNCTION__,__LINE__);
			break;

		case SUBTYPE_DISASSOC:
			APPeerDisassocReqAction(pRxBlk);

			Printf_High("%s,%d, SUBTYPE_DISASSOC: \n",__FUNCTION__,__LINE__);
			//dumpMacTable();
			break;

		default :
			break;
	}
}

#endif

#ifdef CONFIG_STATION
//
// All Rx routines use RX_BLK structure to hande rx events
// It is very important to build pRxBlk attributes
//  1. pHeader pointer to 802.11 Header
//  2. pData pointer to payload including LLC (just skip Header)
//  3. set payload size including LLC to DataSize
//  4. set some flags with RX_BLK_SET_FLAG()
// 
VOID STAHandleRxDataFrame(
	RX_BLK*			pRxBlk)
{
#if (CFG_SUPPORT_TCPIP == 0)
	PUCHAR rawpacket;
	UINT16 rawpacketlength;
#endif

/*************************************************
*data frame parsing only in SMTCN and CONNECTED
**************************************************/
#if (ATCMD_ATE_SUPPORT == 1)
	/*Do nothing*/

#else
   switch (pIoTMlme->CurrentWifiState){
       case WIFI_STATE_SMTCNT:
			#if (CFG_SUPPORT_MTK_SMNT == 1)
				/*implement MTK smart connection if there is no customer callback for smart connection*/
				SMTCNRXFrameHandle(pRxBlk->pData, pRxBlk->pRxWI->MPDUtotalByteCount);
			#else
				{
					//customer smart connect process
					//collect smnt connection packet
				}
			#endif
		   break;
		
	   // [Arron Modify] : for 4-way handshake
	   case WIFI_STATE_4WAY:		
	   case WIFI_STATE_CONNED:
			//skip 802.11 and SNAP header
	   		RTMPRawDataSanity(pRxBlk);
#if CFG_SUPPORT_4WAY_HS
			//check is EAPOL package
			if (!memcmp(pRxBlk->pData, EAPOL, 2))
			{
				/* Confirm incoming is EAPOL-KEY frame */
				if( *(pRxBlk->pData + 3) == EAPOLKey)
				{	
					printf("EAPOL KEY +++\n");
					
					/*pData include 2Byte Protocol type(0x888E) of 802.2 LLC header */
					/*Only transfer Eapol data and len to  WpaEAPOLKeyAction()*/
					pRxBlk->pData	  += 2;   
					pRxBlk->DataSize  -= 2;

					/* process EAPOL KEY 4-way handshake and 2-way group handshake */
					WpaEAPOLKeyAction(pRxBlk);
				}
				break;
			}
#endif
            /*
             * 2014/05/23,terrence,MT7681 STA power saving mode
             * here,we check the received UC/BMC packet which parepared for power saving
             */
           if (pIoTMlme->CurrentWifiState == WIFI_STATE_CONNED)
           {
#if (MT7681_POWER_SAVING == 1)
               STACheckRxDataForPS(pRxBlk);
#endif
#if CFG_SUPPORT_TCPIP
			pRxBlk->pData    -= 6;
			memcpy(pRxBlk->pData, pRxBlk->pHeader->Addr3, 6);
			pRxBlk->DataSize += 6;

			pRxBlk->pData    -= 6;
			memcpy(pRxBlk->pData, pRxBlk->pHeader->Addr1, 6);
			pRxBlk->DataSize += 6;

			netif_rx(pRxBlk->pData, pRxBlk->DataSize);
#else
	   	   if (pRxBlk->pRxINFO->Bcast)
		   {
				DBGPRINT(RT_DEBUG_INFO,("UDP0\n"));
				rawpacket = ParsingBCData(pRxBlk, &rawpacketlength);

				if(rawpacket != NULL)
				{
					DBGPRINT(RT_DEBUG_INFO,("UDP2\n"));
					IoT_process_app_packet(rawpacket, rawpacketlength);
				}
	   	   	}
#endif       

		   }


	   	    break;
	   default:
	   	    break;
   	}
#endif

	return;
}

VOID STAHandleRxMgmtFrame(
	RX_BLK*			pRxBlk)
{
#if (ATCMD_ATE_SUPPORT == 1)
	/*Do nothing*/

#else
	switch (pIoTMlme->CurrentWifiState)
	{
		/*Scan SM*/
		case WIFI_STATE_SCAN:
			ScanProcessMgmt(pRxBlk);
			break;
			
		/*Auth SM*/
		case WIFI_STATE_AUTH:
			MlmeAuthProcess(pRxBlk);
			break;
			
		/*Assoc SM*/
		case WIFI_STATE_ASSOC:
			MlmeAssocProcess(pRxBlk);
			break;
		
		/*Connected SM*/
		case WIFI_STATE_CONNED:
			if (pRxBlk->pRxINFO->U2M)
			{
				if (pRxBlk->pHeader->FC.SubType == SUBTYPE_DEAUTH)
				{   
				    //keep awake
				    //printf("PeerDeauthAction2\n");
					wifi_state_chg(WIFI_STATE_INIT, 0);	
				}
				if (pRxBlk->pHeader->FC.SubType == SUBTYPE_DISASSOC)
				{   
				    //keep awake
				    //printf("PeerDISASSOCAction2\n");
					PeerDisassocAction(pRxBlk, 
								pRxBlk->pData,
								pRxBlk->DataSize, 
								pRxBlk->pHeader->FC.SubType);
				}
			}
			
			if (pRxBlk->pRxINFO->MyBss)
			{
				if(pRxBlk->pHeader->FC.SubType == SUBTYPE_BEACON)
				{
					//printf("beacon received,time:%d\n",kal_get_systime());

					/* Go initial state if the PrimaryCH information in receiving Beacon is not equal to current primary channel*/
					if (PeerBeaconCHInfoParse(pRxBlk))
					{
						RestartBCNTimer();
					}
					else
					{
						BeaconTimeoutAction(0,0);
					}

					/*
					* 2014/05/23,terrence,MT7681 STA power saving mode
					* here,we check the received beacon packet which parepared for power saving
					*/
					#if (MT7681_POWER_SAVING == 1) 
					STACheckRxBeaconForPS(pRxBlk);
					#endif	                          
				 }
			}
			break;

		default:
			break;
	}
#endif
}

#endif /*CONFIG_STATION*/

/*
	========================================================================

	Routine Description:
		Process RxDone interrupt, running in DPC level

	Arguments:
		pAd Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
		This routine has to maintain Rx ring read pointer.
		Need to consider QOS DATA format when converting to 802.3
	========================================================================
*/
BOOLEAN STARxDoneInterruptHandle(
	pBD_t pBufDesc)
{
	PHEADER_802_11	pHeader;
	RX_BLK			RxCell;
	PRXWI_STRUC     pRxWI;
	PRXINFO_STRUC   pRxINFO;
	PUCHAR          pBuff;
	
#ifdef CONFIG_SOFTAP
	/* NULL packet(length=52) , PS-Poll Packet(Length=44) are the normal packets in AP mode*/
	if (pBufDesc->Length < sizeof(HEADER_802_11) + RXINFO_SIZE + RXWI_SIZE) 
	{
		if (pBufDesc->Length != (16 + RXINFO_SIZE + RXWI_SIZE))
		{
			/*not  NULL  or PS-Poll  Packet,  and invalid packet length */
			DBGPRINT_HIGH(RT_DEBUG_TRACE,("Err len=%d\n", pBufDesc->Length));
			return FALSE;
		}
		
		/*PS-Poll Packet should be deal with in APHandleRxControlFrame()*/
	}
#else
	if (pBufDesc->Length <= sizeof(HEADER_802_11) +RXINFO_SIZE +RXWI_SIZE)
	{
		// invalid packet length
		//DBGPRINT_HIGH(RT_DEBUG_TRACE,("Err len=%d\n", pBufDesc->Length));
		return FALSE;
	}
#endif


	/* RX_FCEINFO  */
	/* RXINFO      */
	/* RXWI        */
	/* Payload     */
	
	/* check MIC error by pRxD attribute */
	/* single frame MIC is calculated by HW */
	pBuff   = (PUCHAR)pBufDesc->pBuf;
	pRxINFO = (PRXINFO_STRUC)(pBuff);
	pRxWI   = (PRXWI_STRUC) (pBuff +RXINFO_SIZE);
	pHeader = (PHEADER_802_11) (pBuff +RXINFO_SIZE +RXWI_SIZE);
	StorePhyMode(pRxWI->PHYMode);

	/*move RTMPCheckRxError() into RxFsIntFilterOut() which be implemented in ISR */
	
	/* fill RxBLK */
	RxCell.pRxINFO = pRxINFO;
	RxCell.pRxWI   = pRxWI;
	RxCell.pHeader = pHeader;
	RxCell.pRxPacket = pBufDesc;
	RxCell.pData = (UCHAR *) pHeader;
	RxCell.DataSize = pRxWI->MPDUtotalByteCount;
	RxCell.Flags = 0;

	/* if remote wakeup is triggered, all the packets should be ignored */
	//if(pAd.remoteWakeupFlag ==1)
	//	return FALSE;

#if (RX_CLASS123 == 1)	
	if(!STARxFrameClassVfy(&RxCell, pHeader->FC.Type))
	{
		return FALSE;
	}
#endif


#if (ATCMD_ATE_SUPPORT == 1)
	/* Increase Total receive byte counter after real data received no mater any error or not */
	gATEInfo.WlanCounters.ReceivedFragmentCount.QuadPart++;

	if ((RxCell.pRxINFO->Bcast) || (RxCell.pRxINFO->Mcast))
	{
		gATEInfo.RxBMTotalCnt++;
		gATEInfo.OneSecCountersTmp.RxCntBM1S++;
	}
	else
	{
		gATEInfo.RxU2MTotalCnt++;
		gATEInfo.OneSecCountersTmp.RxCntU2M1S++;
	}

	/*only summary RSSI for u2m packet*/
	//ATESampleRssi(RxCell.pRxWI);	 /*The old method for ATE, no exactly*/
	Update_Rssi_Sample(&gATEInfo, RxCell.pRxWI);

	if ((pHeader->FC.Type == BTYPE_MGMT) || 
		(pHeader->FC.Type == BTYPE_CNTL))
	{
		gATEInfo.RxMgmtCntlTotalCnt++;
		gATEInfo.OneSecCountersTmp.RxCntMgmtCntl1S++;
	}
#endif

#if (IOT_SMP_RSSI_COLLECT == 1)
    Update_Rssi_Sample_Cust(&gRxRSSIInfo, RxCell.pRxWI);
#endif

	printf("%s,%d	BufDescLen(%d) , Type/SubType:(0x%x,0x%x) \n",
		__FUNCTION__,__LINE__,pBufDesc->Length, pHeader->FC.Type, pHeader->FC.SubType);

	switch (pHeader->FC.Type)
	{	
		/* CASE I, receive a MANAGEMENT frame */
		case BTYPE_MGMT:
		{
			/* process Management frame */
			#ifdef CONFIG_STATION
			STAHandleRxMgmtFrame(&RxCell);
			#else
			APHandleRxMgmtFrame(&RxCell);
			#endif
			
			break;
		}
		/* CASE II, receive a DATA frame */
		case BTYPE_DATA:
		{	
			if((pHeader->FC.ToDs == 0)&&(pHeader->FC.FrDs == 1))
			  memcpy(uip_sourceAddr, pHeader->Addr3, 6);
			
		#ifdef CONFIG_STATION
			//jinchuan  (Only deal with Data frame in scan or Connected State,	otherwise we need response Deauth)
			if(pIoTMlme->DataEnable == 1) 
				STAHandleRxDataFrame(&RxCell);
		#else
			APHandleRxDataFrame(&RxCell);
		#endif

			break;
		}
		/* CASE III, receive a CONTROL frame */
		case BTYPE_CNTL:
		{
		#ifdef CONFIG_SOFTAP	
			APHandleRxControlFrame(&RxCell);
		#endif
			break;
		}
		default:
			DBGPRINT(RT_DEBUG_INFO,("DF "));
			/* free control */
			break;
	}
 
	return TRUE;
}


extern MLME_STRUCT *pIoTMlme;

/*
 * ISR context
 * SW Filter to exclude unexpected packets
 * be carefully,that API is called in FCE RX ISR handler to filter out packet
 * TRUE:  discard the packet directly in FCE RX interrupt handler
 * FALSE: allow to receive the packet in FCE RX interrupt handler
 */
BOOLEAN RxFsIntFilterOut(pBD_t RxpBufDesc)
{
    PUCHAR          pBuff;
    PRXINFO_STRUC   pRxINFO;
    PRXWI_STRUC     pRxWI;
    PHEADER_802_11  pHeader;
#if (MT7681_POWER_SAVING == 1)
    RX_BLK			RxBlk;
#endif
    //USHORT          DataSize = 0; 
    //UINT8           U2M = 0;
    //UINT8           Mcast = 0;  
    UINT8           type;
    UINT8           subtype;

  
    pBuff   = (PUCHAR)RxpBufDesc->pBuf;
    pRxINFO = (PRXINFO_STRUC)(pBuff);
    pRxWI   = (PRXWI_STRUC)(pBuff + RXINFO_SIZE);
    pHeader = (PHEADER_802_11)(pBuff + RXINFO_SIZE + RXWI_SIZE);
    type = pHeader->FC.Type;
    subtype = pHeader->FC.SubType;
    //DataSize = pRxWI->MPDUtotalByteCount;
    //U2M = pRxINFO->U2M;
    //Mcast = pRxINFO->Mcast;  

	if(RTMPCheckRxError(pHeader, pRxWI, pRxINFO)== NDIS_STATUS_FAILURE)
	{	/* free packet */
		return TRUE;
	}

#ifdef CONFIG_STATION
    switch  (pIoTMlme->CurrentWifiState)
    {
		case WIFI_STATE_INIT:
		case WIFI_STATE_SMTCNT:
             if (type == BTYPE_DATA)
             {
                if ((subtype == SUBTYPE_NULL_FUNC) || (subtype == SUBTYPE_QOS_NULL))
                    return TRUE;
             }
#if (CFG_SUPPORT_MTK_SMNT == 1) 
             else if (type == BTYPE_MGMT)
             {
                 if (subtype == SUBTYPE_BEACON)
                    return TRUE;
             }      
#endif
			 break;      
        case WIFI_STATE_CONNED:
    
             /*The Mcast, Bcast bits shall be set as 1, if Received Packet is Broadcast  */
             /*The Mcast shall be set as 1, Bcast bit shall be set as 0, if Received Packet is Multicast  */
             if ((pRxINFO->Mcast) && (pRxINFO->Bcast == 0))
             {
                /*
                 * 2014/05/23,terrence,MT7681 STA power saving mode
                 * here,we check the received UC/BMC packet which parepared for power saving
                 */
 #if (MT7681_POWER_SAVING == 1)
                 /* fill RxBLK */
              	 RxBlk.pRxINFO = pRxINFO;
              	 RxBlk.pRxWI   = pRxWI;
              	 RxBlk.pHeader = pHeader;
              	 RxBlk.pRxPacket = RxpBufDesc;
              	 RxBlk.pData = (UCHAR *) pHeader;
              	 //RxBlk.DataSize = pRxWI->MPDUtotalByteCount;
              	 //RxBlk.Flags = 0;
            	
                 STACheckRxDataForPS(&RxBlk);
 #endif
                 /*Improve Rx Performance by drop Multicast frame while MT7681 connected with AP router */
                 return TRUE;
             }
             break;
        default:
            break;
    }
#endif

#ifdef CONFIG_SOFTAP
	if (type == BTYPE_MGMT)
	{
		/*There are too many Beacons from other AP routers, or management frame not send to MyBSS */
		/*Drop notMyBss management frame, except ProbeReq for reponsing Active scan from STA*/
	   if ((pRxINFO->MyBss == 0) && (subtype != SUBTYPE_PROBE_REQ))
			return TRUE;
	}
#endif

    return FALSE;
}

#if (IOT_SMP_RSSI_COLLECT == 1)
VOID Update_Rssi_Sample_Cust(
	IN RX_RSSI_INFO *pRssi,
	IN RXWI_STRUC *pRxWI)
{
	CHAR rssi[3];
	BOOLEAN bInitial = FALSE;

	if (!(pRssi->AvgRssi0 | pRssi->AvgRssi0X8 | pRssi->LastRssi0))
	{
		bInitial = TRUE;
	}
	
	if (pRxWI->SNR2 != 0)
	{
		pRssi->LastRssi0 = ConvertToRssi((CHAR)(pRxWI->SNR2), RSSI_0, pRxWI->SNR1, pRxWI->bw);
		if (bInitial)
		{
			pRssi->AvgRssi0X8 = pRssi->LastRssi0 << 3;
			pRssi->AvgRssi0  = pRssi->LastRssi0;
		}
		else
			pRssi->AvgRssi0X8 = (pRssi->AvgRssi0X8 - pRssi->AvgRssi0) + pRssi->LastRssi0;
 
		pRssi->AvgRssi0 = pRssi->AvgRssi0X8 >> 3;
	}
}
#endif

