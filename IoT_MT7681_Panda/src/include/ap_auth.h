#ifndef _AP_AUTH_H_
#define _AP_AUTH_H_

#include <stdio.h>
#include "rtmp_general_pub.h"
#include "queue.h"

/* Auth and Assoc mode related definitions */
#define AUTH_MODE_OPEN              0x00
#define AUTH_MODE_KEY               0x01

VOID ApSendDeauthAction(IN PUCHAR pAddr, IN USHORT Reason) XIP_ATTRIBUTE(".xipsec0");
VOID APPeerAuthReqAction(RX_BLK *pRxBlk) XIP_ATTRIBUTE(".xipsec0");
VOID APSendProbeAction(RX_BLK *pRxBlk) XIP_ATTRIBUTE(".xipsec0");
VOID APPeerDeauthReqAction(IN RX_BLK *pRxBlk) XIP_ATTRIBUTE(".xipsec0");
VOID APPeerAuthSimpleRspGenAndSend(
    IN PHEADER_802_11 pHdr, 
    IN USHORT Alg, 
    IN USHORT Seq, 
    IN USHORT StatusCode) XIP_ATTRIBUTE(".xipsec0");

#endif /*_AP_AUTH_H_*/
