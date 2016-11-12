
#ifndef WPA_CMM_H
#define WPA_CMM_H

#include "types.h"
#include "dot11i_wpa.h"
#include "xip_ovly.h"

#ifdef CONFIG_SOFTAP
#include "ap_pub.h"
#endif

#ifndef CONFIG_STA_SUPPORT
#define CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
#define TX_EAPOL_BUFFER			768//1024

// Retry timer counter initial value
#define PEER_MSG1_RETRY_TIMER_CTR           0
#define PEER_MSG3_RETRY_TIMER_CTR           10
#define GROUP_MSG1_RETRY_TIMER_CTR          20

// WPA mechanism retry timer interval
#define PEER_MSG1_RETRY_EXEC_INTV           1000        		// 1 sec
#define PEER_MSG3_RETRY_EXEC_INTV           3000        		// 3 sec
#define GROUP_KEY_UPDATE_EXEC_INTV          1000				// 1 sec
#define PEER_GROUP_KEY_UPDATE_INIV			2000				// 2 sec

#define	EAPOL_MSG_INVALID	0
#define	EAPOL_PAIR_MSG_1	1
#define	EAPOL_PAIR_MSG_2	2
#define	EAPOL_PAIR_MSG_3	3
#define	EAPOL_PAIR_MSG_4	4
#define	EAPOL_GROUP_MSG_1	5
#define	EAPOL_GROUP_MSG_2	6

#define ENQUEUE_EAPOL_START_TIMER			200					// 200 ms

// group rekey interval
#define TIME_REKEY                          0
#define PKT_REKEY                           1
#define DISABLE_REKEY                       2
#define MAX_REKEY                           2

#define MAX_REKEY_INTER                     0x3ffffff

#define EAPOL_START_DISABLE					0
#define EAPOL_START_PSK						1
#define EAPOL_START_1X						2

//
// Common WPA state machine: states, events, total function #
//
#define WPA_PTK                      0
#define MAX_WPA_PTK_STATE            1

#define WPA_MACHINE_BASE             0
#define MT2_EAPPacket                0
#define MT2_EAPOLStart               1
#define MT2_EAPOLLogoff              2
#define MT2_EAPOLKey                 3
#define MT2_EAPOLASFAlert            4
#define MAX_WPA_MSG                  5

#define WPA_FUNC_SIZE                (MAX_WPA_PTK_STATE * MAX_WPA_MSG)



#define IS_WPA_CAPABILITY(a)       (((a) >= Ndis802_11AuthModeWPA) && ((a) <= Ndis802_11AuthModeWPA1PSKWPA2PSK))

/* 	
	WFA recommend to restrict the encryption type in 11n-HT mode.
 	So, the WEP and TKIP shall not be allowed to use HT rate. 
 */
#define IS_INVALID_HT_SECURITY(_mode)		\
	(((_mode) == Ndis802_11Encryption1Enabled) || \
	 ((_mode) == Ndis802_11Encryption2Enabled))

#define MIX_CIPHER_WPA_TKIP_ON(x)       (((x) & 0x08) != 0)
#define MIX_CIPHER_WPA_AES_ON(x)        (((x) & 0x04) != 0)
#define MIX_CIPHER_WPA2_TKIP_ON(x)      (((x) & 0x02) != 0)
#define MIX_CIPHER_WPA2_AES_ON(x)       (((x) & 0x01) != 0)



typedef enum _WpaRole {
    WPA_NONE,               // 0
    WPA_Authenticator,      // 1
    WPA_Supplicant,         // 2
    WPA_BOTH,               // 3: Authenticator and Supplicant
} WPA_ROLE;    

// For supplicant state machine states. 802.11i Draft 4.1, p. 97
// We simplified it
typedef	enum	_WpaState
{
	SS_NOTUSE,				// 0
	SS_START,				// 1
	SS_WAIT_MSG_3,			// 2
	SS_WAIT_GROUP,			// 3
	SS_FINISH,  			// 4
	SS_KEYUPDATE,			// 5
}	WPA_STATE;

// for-wpa value domain of pMacEntry->WpaState  802.1i D3   p.114
typedef enum _GTKState {
    REKEY_NEGOTIATING,
    REKEY_ESTABLISHED,
    KEYERROR,
} GTK_STATE;

//  for-wpa  value domain of pMacEntry->WpaState  802.1i D3   p.114
typedef enum _WpaGTKState {
    SETKEYS,
    SETKEYS_DONE,
} WPA_GTK_STATE;

/* WPA internal command type */
#define WPA_SM_4WAY_HS_START 	1
#define WPA_SM_DISCONNECT		0xff

/* WPA element IDs */
typedef enum _WPA_VARIABLE_ELEMENT_ID
{		
   	WPA_ELEM_CMD = 1,
   	WPA_ELEM_PEER_RSNIE,
   	WPA_ELEM_LOCAL_RSNIE,
   	WPA_ELEM_PMK,
   	WPA_ELEM_RESV
} WPA_VARIABLE_ELEMENT_ID;

#define GROUP_SUITE					0
#define PAIRWISE_SUITE				1
#define AKM_SUITE					2
#define RSN_CAP_INFO				3
#define PMKID_LIST					4
#define G_MGMT_SUITE				5


/* The internal command list for ralink dot1x daemon using */
typedef	enum	_Dot1xInternalCmd
{
	DOT1X_DISCONNECT_ENTRY, 			
	DOT1X_RELOAD_CONFIG,
}	DOT1X_INTERNAL_CMD;

// 802.1x authentication format
typedef	struct	_IEEE8021X_FRAME	{
	UCHAR	Version;					// 1.0
	UCHAR	Type;						// 0 = EAP Packet
	USHORT	Length;
}	IEEE8021X_FRAME, *PIEEE8021X_FRAME;

typedef struct GNU_PACKED _RSN_IE_HEADER_STRUCT	{
	UCHAR		Eid;
	UCHAR		Length;
	USHORT		Version;	// Little endian format
}	RSN_IE_HEADER_STRUCT, *PRSN_IE_HEADER_STRUCT;

// Cipher suite selector types
typedef struct GNU_PACKED _CIPHER_SUITE_STRUCT	{
	UCHAR		Oui[3];
	UCHAR		Type;
}	CIPHER_SUITE_STRUCT, *PCIPHER_SUITE_STRUCT;

// Authentication and Key Management suite selector
typedef struct GNU_PACKED _AKM_SUITE_STRUCT	{
	UCHAR		Oui[3];
	UCHAR		Type;
}	AKM_SUITE_STRUCT, *PAKM_SUITE_STRUCT;

// RSN capability
typedef struct	GNU_PACKED _RSN_CAPABILITY	{
	USHORT		Rsv:10;
	USHORT		GTKSAReplayCnt:2;
	USHORT		PTKSAReplayCnt:2;
	USHORT		NoPairwise:1;
	USHORT		PreAuth:1;
}	RSN_CAPABILITY, *PRSN_CAPABILITY;

#if 0	//move to rtmp_general_pub.h
typedef struct _CIPHER_KEY {
	UCHAR   Key[16];            	// 128 bits max
	UCHAR   TxMic[8];
	UCHAR   RxMic[8];
	UCHAR   TxTsc[16];           	// TSC value. Change it from 48bit to 128bit 
	UCHAR   RxTsc[16];           	// TSC value. Change it from 48bit to 128bit
	UCHAR   CipherAlg;           	// 0:none, 1:WEP64, 2:WEP128, 3:TKIP, 4:AES, 5:CKIP64, 6:CKIP128
 	UCHAR   KeyLen; 				// Key length for each key, 0: entry is invalid
#ifdef CONFIG_STA_SUPPORT
	UCHAR   BssId[6];
#endif // CONFIG_STA_SUPPORT //
	UCHAR   Type;               // Indicate Pairwise/Group when reporting MIC error
} CIPHER_KEY, *PCIPHER_KEY;
#endif

VOID GenRandom(
	IN	UCHAR	*macAddr,
	OUT	UCHAR	*random)     XIP_ATTRIBUTE(".xipsec0");

VOID AsicRemovePairwiseKeyEntry(
	IN USHORT Wcid) XIP_ATTRIBUTE(".xipsec0");

VOID WpaDeriveGTK(
    IN  UCHAR   *GMK,
    IN  UCHAR   *GNonce,
    IN  UCHAR   *AA,
    OUT UCHAR   *output,
    IN  UINT    len) XIP_ATTRIBUTE(".xipsec0");

VOID WPAInstallSharedKey(
	UINT8				GroupCipher,
	UINT8				BssIdx,
	UINT8				KeyIdx,
	UINT8				Wcid,
	BOOLEAN				bAE,
	PUINT8				pGtk) XIP_ATTRIBUTE(".xipsec0");

#if CFG_SUPPORT_4WAY_HS

VOID TKIP_GTK_KEY_UNWRAP(UCHAR *key, 
		UCHAR *iv, 
		UCHAR *input_text, 
		UINT32 input_len, 
		UCHAR *output_text);

VOID TKIP_GTK_KEY_WRAP( 
    UCHAR    *key,
    UCHAR	*iv,
    UCHAR    *input_text,
    UINT32    input_len,
    UCHAR   *output_text);


VOID    WPAInstallPairwiseKey(
		UINT8 			 	BssIdx,
#ifdef CONFIG_SOFTAP
		PMAC_TABLE_ENTRY 	pEntry,
#endif
		PUCHAR 				pPTK) XIP_ATTRIBUTE(".xipsec0");

VOID	_ConstructEapolMsg(
#ifdef CONFIG_SOFTAP
	IN	PMAC_TABLE_ENTRY	pEntry1,
#endif	  
	IN	UCHAR				GroupKeyWepStatus,
	IN	UCHAR				MsgType,  
	IN	UCHAR				DefaultKeyIdx,
	IN	UCHAR				*KeyNonce,
	IN	UCHAR				*TxRSC,
	IN	UCHAR				*GTK,
	IN	UCHAR				*RSNIE,
	IN	UCHAR				RSNIE_Len,
	OUT PEAPOL_PACKET       pMsg) OVLY_ATTRIBUTE(".ovlysec7"); //ram0 


VOID	ConstructEapolMsg(
#ifdef CONFIG_SOFTAP
	IN	PMAC_TABLE_ENTRY 	pEntry,
#endif	  
	IN	UCHAR				GroupKeyWepStatus,
	IN	UCHAR				MsgType,  
	IN	UCHAR				DefaultKeyIdx,
	IN	UCHAR				*KeyNonce,
	IN	UCHAR				*TxRSC,
	IN	UCHAR				*GTK,
	IN	UCHAR				*RSNIE,
	IN	UCHAR				RSNIE_Len,
	OUT PEAPOL_PACKET       pMsg) XIP_ATTRIBUTE(".xipsec0");


VOID    WpaEAPOLKeyAction(
	IN  RX_BLK* pRxBlk)  XIP_ATTRIBUTE(".xipsec0");
#endif

#endif // WPA_CMM_H //

