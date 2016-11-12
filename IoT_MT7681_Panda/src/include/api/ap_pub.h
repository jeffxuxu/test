#ifndef _AP_PUB_H_
#define _AP_PUB_H_

#include "types.h"
#include "iot_api.h"

/***************************************************************************
  *	Customization Macro
  **************************************************************************/
#define IOT_BCN_TIM_KEEP_ON   /*If open this Macro,  the TIM value in Beacon is 1 for each MAC Entry*/

/***************************************************************************
  *	AP Common Macro
  **************************************************************************/
#define CLIENT_STATUS_SET_FLAG(_pEntry,_F)      ((_pEntry)->ClientStatusFlags |= (_F))
#define CLIENT_STATUS_CLEAR_FLAG(_pEntry,_F)    ((_pEntry)->ClientStatusFlags &= ~(_F))
#define CLIENT_STATUS_TEST_FLAG(_pEntry,_F)     (((_pEntry)->ClientStatusFlags & (_F)) != 0)

/* STA_CSR4.field.TxResult */
#define TX_RESULT_SUCCESS           0
#define TX_RESULT_ZERO_LENGTH       1
#define TX_RESULT_UNDER_RUN         2
#define TX_RESULT_OHY_ERROR         4
#define TX_RESULT_RETRY_FAIL        6


/* MCS for CCK.  BW.SGI.STBC are reserved */
#define MCS_LONGP_RATE_1                      0	/* long preamble CCK 1Mbps */
#define MCS_LONGP_RATE_2                      1	/* long preamble CCK 1Mbps */
#define MCS_LONGP_RATE_5_5                    2
#define MCS_LONGP_RATE_11                     3
#define MCS_SHORTP_RATE_1                      4	/* long preamble CCK 1Mbps. short is forbidden in 1Mbps */
#define MCS_SHORTP_RATE_2                      5	/* short preamble CCK 2Mbps */
#define MCS_SHORTP_RATE_5_5                    6
#define MCS_SHORTP_RATE_11                     7
/* To send duplicate legacy OFDM. set BW=BW_40.  SGI.STBC are reserved */
#define MCS_RATE_6                      0	/* legacy OFDM */
#define MCS_RATE_9                      1	/* OFDM */
#define MCS_RATE_12                     2	/* OFDM */
#define MCS_RATE_18                     3	/* OFDM */
#define MCS_RATE_24                     4	/* OFDM */
#define MCS_RATE_36                     5	/* OFDM */
#define MCS_RATE_48                     6	/* OFDM */
#define MCS_RATE_54                     7	/* OFDM */
/* HT */
#define MCS_0		0	/* 1S */
#define MCS_1		1
#define MCS_2		2
#define MCS_3		3
#define MCS_4		4
#define MCS_5		5
#define MCS_6		6
#define MCS_7		7
#define MCS_8		8	/* 2S */
#define MCS_9		9
#define MCS_10		10
#define MCS_11		11
#define MCS_12		12
#define MCS_13		13
#define MCS_14		14
#define MCS_15		15
#define MCS_16		16	/* 3*3 */
#define MCS_17		17
#define MCS_18		18
#define MCS_19		19
#define MCS_20		20
#define MCS_21		21
#define MCS_22		22
#define MCS_23		23
#define MCS_32		32
#define MCS_AUTO		33

#ifdef DOT11_N_SUPPORT
/* OID_HTPHYMODE */
/* MODE */
#define HTMODE_MM	0
#define HTMODE_GF	1
#endif /* DOT11_N_SUPPORT */

/* Fixed Tx MODE - HT, CCK or OFDM */
#define FIXED_TXMODE_HT		0
#define FIXED_TXMODE_CCK	1
#define FIXED_TXMODE_OFDM 	2
#define FIXED_TXMODE_VHT	3


#define MAX_NUMBER_OF_MAC		3 //32	/* if MAX_MBSSID_NUM is 8, this value can't be larger than 211 */
#define MAX_BEACON_SIZE		    512
#define MAX_SSID_LEN            32

#define TBTT_PRELOAD_TIME       384 /* usec. LomgPreamble + 24-byte at 1Mbps */
#define DEFAULT_DTIM_PERIOD     1

#define CAP_GENERATE(ess,ibss,priv,s_pre,s_slot,spectrum)  \
	(((ess) ? 0x0001 : 0x0000) | ((ibss) ? 0x0002 : 0x0000) | \
	((priv) ? 0x0010 : 0x0000) | ((s_pre) ? 0x0020 : 0x0000) | \
	((s_slot) ? 0x0400 : 0x0000) | ((spectrum) ? 0x0100 : 0x0000))

/***************************************************************************
  *	Multiple SSID related data structures
  **************************************************************************/
#define WLAN_MAX_NUM_OF_TIM			((MAX_LEN_OF_MAC_TABLE >> 3) + 1)	/* /8 + 1 */
#define WLAN_CT_TIM_BCMC_OFFSET		0	/* unit: 32B */

/* clear bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_CLEAR() \
	pIoTApCfg->MBSSID.TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] &= ~NUM_BIT8[0];

/* set bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_SET() \
	pIoTApCfg->MBSSID.TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] |= NUM_BIT8[0];

/* clear a station PS TIM bit */
#define WLAN_MR_TIM_BIT_CLEAR(wcid) \
	{	UCHAR tim_offset = wcid >> 3; \
		UCHAR bit_offset = wcid & 0x7; \
		pIoTApCfg->MBSSID.TimBitmaps[tim_offset] &= (~NUM_BIT8[bit_offset]); \
		APUpdateBeaconFrame(); }

/* set a station PS TIM bit */
#define WLAN_MR_TIM_BIT_SET(wcid) \
	{	UCHAR tim_offset = wcid >> 3; \
		UCHAR bit_offset = wcid & 0x7; \
		pIoTApCfg->MBSSID.TimBitmaps[tim_offset] |= NUM_BIT8[bit_offset];}


/*============================================================ */
/* ASIC WCID Table definition. */
/*============================================================ */
#define BSSID_WCID		1	/* in infra mode, always put bssid with this WCID */
#define MCAST_WCID	    0x0
#define BSS0Mcast_WCID	0x0
#define BSS1Mcast_WCID	0xf8
#define BSS2Mcast_WCID	0xf9
#define BSS3Mcast_WCID	0xfa
#define BSS4Mcast_WCID	0xfb
#define BSS5Mcast_WCID	0xfc
#define BSS6Mcast_WCID	0xfd
#define BSS7Mcast_WCID	0xfe
#define RESERVED_WCID	0xff

#define MAX_LEN_OF_MAC_TABLE          MAX_NUMBER_OF_MAC     /* The MAC Table length in pEntry which in SRAM, start from 0 */
#define MAX_LEN_OF_MAC_TABLE_IN_REG   (MAX_NUMBER_OF_MAC+1)	/* The MAC Table length in Register, start from 1*/

#define ENTRY_NONE			0
#define ENTRY_CLIENT		1

#define IS_ENTRY_NONE(_x)		((_x)->EntryType == ENTRY_NONE)
#define IS_ENTRY_CLIENT(_x)		((_x)->EntryType == ENTRY_CLIENT)

#define SET_ENTRY_NONE(_x)		((_x)->EntryType = ENTRY_NONE)
#define SET_ENTRY_CLIENT(_x)	((_x)->EntryType = ENTRY_CLIENT)

typedef enum _MAC_ENTRY_OP_MODE_ {
	ENTRY_OP_MODE_ERROR = 0x00,
	ENTRY_OP_MODE_CLI 	= 0x01,	/* Sta mode, set this TRUE after Linkup,too. */
	ENTRY_OP_MODE_WDS 	= 0x02,	/* This is WDS Entry. only for AP mode. */
	ENTRY_OP_MODE_APCLI = 0x04,	/* This is a AP-Client entry, only for AP mode which enable AP-Client functions. */
	ENTRY_OP_MODE_MESH 	= 0x08,	/* Peer conect with us via mesh. */
	ENTRY_OP_MODE_DLS 	= 0x10,	/* This is DLS Entry. only for STA mode. */
	ENTRY_OP_MODE_MAX 	= 0x20
} MAC_ENTRY_OP_MODE;

/* Values of LastSecTxRateChangeAction */
#define RATE_NO_CHANGE	0		/* No change in rate */
#define RATE_UP			1		/* Trying higher rate or same rate with different BF */
#define RATE_DOWN		2		/* Trying lower rate */

typedef union _HTTRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		USHORT MODE:2;	/* Use definition MODE_xxx. */
		USHORT iTxBF:1;
		USHORT rsv:1;
		USHORT eTxBF:1;
		USHORT STBC:2;	/*SPACE */
		USHORT ShortGI:1;
		USHORT BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		USHORT MCS:7;	/* MCS */
	} field;
#else
	struct {
		USHORT MCS:7;	/* MCS */
		USHORT BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		USHORT ShortGI:1;
		USHORT STBC:2;	/*SPACE */
		USHORT eTxBF:1;
		USHORT rsv:1;
		USHORT iTxBF:1;
		USHORT MODE:2;	/* Use definition MODE_xxx. */
	} field;
#endif
	USHORT word;
} HTTRANSMIT_SETTING, *PHTTRANSMIT_SETTING;


#ifdef RT_BIG_ENDIAN
typedef	union _BCN_TIME_CFG_STRUC {
	struct {
		UINT32 TxTimestampCompensate:8;
		UINT32 :3;
		UINT32 bBeaconGen:1;		/* Enable beacon generator */
		UINT32 bTBTTEnable:1;
		UINT32 TsfSyncMode:2;		/* Enable TSF sync, 00: disable, 01: infra mode, 10: ad-hoc mode */
		UINT32 bTsfTicking:1;		/* Enable TSF auto counting */
		UINT32 BeaconInterval:16;  /* in unit of 1/16 TU */
	} field;
	UINT32 word;
} BCN_TIME_CFG_STRUC;
#else
typedef union _BCN_TIME_CFG_STRUC {
	struct {
		UINT32 BeaconInterval:16;
		UINT32 bTsfTicking:1;
		UINT32 TsfSyncMode:2;
		UINT32 bTBTTEnable:1;
		UINT32 bBeaconGen:1;
		UINT32 :3;
		UINT32 TxTimestampCompensate:8;
	} field;
	UINT32 word;
} BCN_TIME_CFG_STRUC;
#endif

typedef struct _QUEUE_ENTRY {
	struct _QUEUE_ENTRY *Next;
} QUEUE_ENTRY, *PQUEUE_ENTRY;

/* Queue structure */
typedef struct _QUEUE_HEADER {
	PQUEUE_ENTRY Head;
	PQUEUE_ENTRY Tail;
	ULONG Number;
} QUEUE_HEADER, *PQUEUE_HEADER;

typedef enum _RT_802_11_PREAMBLE {
	Rt802_11PreambleLong,
	Rt802_11PreambleShort,
	Rt802_11PreambleAuto
} RT_802_11_PREAMBLE, *PRT_802_11_PREAMBLE;

/*for-wpa value domain of pMacEntry->WpaState  802.1i D3   p.114 */
typedef enum _ApWpaState {
	AS_NOTUSE,		/* 0 */
	AS_DISCONNECT,		/* 1 */
	AS_DISCONNECTED,	/* 2 */
	AS_INITIALIZE,		/* 3 */
	AS_AUTHENTICATION,	/* 4 */
	AS_AUTHENTICATION2,	/* 5 */
	AS_INITPMK,		/* 6 */
	AS_INITPSK,		/* 7 */
	AS_PTKSTART,		/* 8 */
	AS_PTKINIT_NEGOTIATING,	/* 9 */
	AS_PTKINITDONE,		/* 10 */
	AS_UPDATEKEYS,		/* 11 */
	AS_INTEGRITY_FAILURE,	/* 12 */
	AS_KEYUPDATE,		/* 13 */
} AP_WPA_STATE;


typedef enum _NDIS_802_11_PRIVACY_FILTER {
	Ndis802_11PrivFilterAcceptAll,
	Ndis802_11PrivFilter8021xWEP
} NDIS_802_11_PRIVACY_FILTER, *PNDIS_802_11_PRIVACY_FILTER;

/* Value domain of pMacEntry->Sst */
typedef enum _Sst {
    SST_NOT_AUTH,   /* 0: equivalent to IEEE 802.11/1999 state 1 */
    SST_AUTH,       /* 1: equivalent to IEEE 802.11/1999 state 2 */
    SST_ASSOC       /* 2: equivalent to IEEE 802.11/1999 state 3 */
} SST;

/* value domain of pMacEntry->AuthState */
typedef enum _AuthState {
    AS_NOT_AUTH,
    AS_AUTH_OPEN,       /* STA has been authenticated using OPEN SYSTEM */
    AS_AUTH_KEY,        /* STA has been authenticated using SHARED KEY */
    AS_AUTHENTICATING   /* STA is waiting for AUTH seq#3 using SHARED KEY */
} AUTH_STATE;


/*============================================================ */
/* ASIC WCID Table definition. */
/*============================================================ */
typedef struct GNU_PACKED _MAC_TABLE_ENTRY {
	/* WPA/WPA2 4-way database */
	TIMER_T EnqueueStartForPSKTimer;        /* A timer which enqueue EAPoL-Start for triggering PSK SM */
	UCHAR EnqueueEapolStartTimerRunning;	/* Enqueue EAPoL-Start for triggering EAP SM */

	UCHAR EntryType;    /*0:Invalid,  1: Client*/

	/*jan for wpa */
	/* record which entry revoke MIC Failure , if it leaves the BSS itself, AP won't update aMICFailTime MIB */
	UCHAR RSNIE_Len;
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR ANonce[LEN_KEY_DESC_NONCE];
	UCHAR SNonce[LEN_KEY_DESC_NONCE];
	UCHAR R_Counter[LEN_KEY_DESC_REPLAY];
	UCHAR PTK[64];
	UCHAR ReTryCounter;
#ifdef JCB4WAYTIMER
	TIMER_T RetryTimer;
#endif
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;	/* This should match to whatever microsoft defined */
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_802_11_WEP_STATUS GroupKeyWepStatus;
	UINT8 WpaState;
	UINT8 GTKState;
	USHORT PortSecured;
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */
	CIPHER_KEY PairwiseKey;

//	UCHAR PsMode;
	SST Sst;
	AUTH_STATE AuthState;	/* for SHARED KEY authentication state machine used only */
	BOOLEAN IsReassocSta;	/* Indicate whether this is a reassociation procedure */
	USHORT Aid;
	USHORT CapabilityInfo;
	UINT32 StaIdleTimeout;	/* idle timeout per entry */
	ULONG NoDataIdleCount;
	UINT16 StationKeepAliveCount;	/* unit: second */

	UCHAR MaxSupportedRate;
	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;	/* For transmit phy setting in TXWI. */
	ULONG AssocDeadLine;
} MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;

typedef struct _MAC_TABLE {
	MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	USHORT Size;
} MAC_TABLE, *PMAC_TABLE;


/*AP configure*/
typedef struct GNU_PACKED _COMMON_CONFIG {
		BOOLEAN bCountryFlag;
		UCHAR CountryCode[3];

		UCHAR CountryRegion;		/* Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel */
		UCHAR CountryRegionForABand;/* Enum of country region for A band */
		UCHAR PhyMode;
		UCHAR cfg_wmode;
		UCHAR SavedPhyMode;
		USHORT Dsifs;				/* in units of usec */

		USHORT BeaconPeriod;
		UCHAR Channel;
		UCHAR CentralChannel;		/* Central Channel when using 40MHz is indicating. not real channel. */
	
		UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
		UCHAR SupRateLen;
		UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
		UCHAR ExtRateLen;
		UCHAR DesireRate[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
		UCHAR MaxDesiredRate;
		UCHAR ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];
	
		ULONG BasicRateBitmap;		/* backup basic ratebitmap */
		ULONG BasicRateBitmapOld;	/* backup basic ratebitmap */

		
		UCHAR TxPower;				/* in unit of mW */
		ULONG TxPowerPercentage;	/* 0~100 % */
		ULONG TxPowerDefault;		/* keep for TxPowerPercentage */

		ULONG TxPreamble;			/* Rt802_11PreambleLong, Rt802_11PreambleShort, Rt802_11PreambleAuto */
		BOOLEAN bUseZeroToDisableFragment;	/* Microsoft use 0 as disable */
		ULONG UseBGProtection;		/* 0: auto, 1: always use, 2: always not use */
		BOOLEAN bUseShortSlotTime;	/* 0: disable, 1 - use short slot (9us) */
		BOOLEAN bEnableTxBurst;		/* 1: enble TX PACKET BURST (when BA is established or AP is not a legacy WMM AP), 0: disable TX PACKET BURST */
		BOOLEAN bAggregationCapable;/* 1: enable TX aggregation when the peer supports it */
		BOOLEAN bPiggyBackCapable;	/* 1: enable TX piggy-back according MAC's version */
		BOOLEAN bIEEE80211H;		/* 1: enable IEEE802.11h spec. */
		UCHAR RDDurRegion; 			/* Region of radar detection */
		ULONG DisableOLBCDetect;	/* 0: enable OLBC detect; 1 disable OLBC detect */
		UINT8 PwrConstraint;

	#ifdef BNC_UPDATE_PERIOD /*Default turn off,  will cause system halt*/
		BOOLEAN IsUpdateBeacon;
		//BEACON_SYNC_STRUCT *pBeaconSync;
		//RALINK_TIMER_STRUCT BeaconUpdateTimer;
		UINT32 BeaconAdjust;
		UINT32 BeaconFactor;
		UINT32 BeaconRemain;
	#endif	
		
} COMMON_CONFIG, *PCOMMON_CONFIG;

typedef union _DESIRED_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		USHORT rsv:3;
		USHORT FixedTxMode:2;	/* If MCS isn't AUTO, fix rate in CCK, OFDM or HT mode. */
		USHORT PhyMode:4;
		USHORT MCS:7;	/* MCS */
	} field;
#else
	struct {
		USHORT MCS:7;	/* MCS */
		USHORT PhyMode:4;
		USHORT FixedTxMode:2;	/* If MCS isn't AUTO, fix rate in CCK, OFDM or HT mode. */
		USHORT rsv:3;
	} field;
#endif
	USHORT word;
} DESIRED_TRANSMIT_SETTING, *PDESIRED_TRANSMIT_SETTING;


typedef struct _MULTISSID_STRUCT {

	UCHAR Bssid[MAC_ADDR_LEN];
	UCHAR SsidLen;
	UCHAR Ssid[MAX_LEN_OF_SSID+1];
	USHORT CapabilityInfo;

	UCHAR MaxStaNum;				/* Limit the STA connection number per BSS */
	UCHAR StaCount;

	UCHAR RSNIE_Len[2];
	UCHAR RSN_IE[2][MAX_LEN_OF_RSNIE];

	NDIS_802_11_AUTHENTICATION_MODE AuthMode;
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_802_11_WEP_STATUS GroupKeyWepStatus;
	WPA_MIX_PAIR_CIPHER WpaMixPairCipher;

	/*CIPHER_KEY SharedKey[SHARE_KEY_NUM]; // ref pAd->SharedKey[BSS][4] */
	UCHAR Passphase[CIPHER_TEXT_LEN];
	UINT8 PassphaseLen;
	UCHAR DefaultKeyId;

	UCHAR TxRate; 					/* RATE_1, RATE_2, RATE_5_5, RATE_11, ... */
	UCHAR DesiredRates[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
	UCHAR DesiredRatesIndex;
	UCHAR MaxTxRate; 				/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	DESIRED_TRANSMIT_SETTING DesiredTransmitSetting;	/* Desired transmit setting. this is for reading registry setting only. not useful. */
	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;	/* For transmit phy setting in TXWI. */

	/*ULONG TimBitmap; */      		// bit0 for broadcast, 1 for AID1, 2 for AID2, ...so on
	/*ULONG TimBitmap2; */     		// b0 for AID32, b1 for AID33, ... and so on */
	UCHAR TimBitmaps[WLAN_MAX_NUM_OF_TIM];

	/* WPA */
	UCHAR GMK[32];
	UCHAR PMK[32];
	UCHAR GTK[32];
	UCHAR GNonce[32];
	UCHAR PortSecured;
	CIPHER_KEY PairwiseKey;
	CIPHER_KEY SharedKey[1][4]; 	/* STA always use SharedKey[BSS0][0..3] */

	/* outgoing BEACON frame buffer and corresponding TXWI */
	/* TXWI_STRUC *BeaconTxWI; */
	CHAR BeaconBuf[MAX_BEACON_SIZE];/* NOTE: BeaconBuf should be 4-byte aligned */

	BOOLEAN bHideSsid;
	//UINT16 StationKeepAliveTime;	/* unit: second */

	/* used in if beacon send or stop */
	BOOLEAN bBcnSntReq;
	UCHAR BcnBufIdx;
	UCHAR PhyMode;

	UCHAR TimIELocationInBeacon;
	UCHAR CapabilityInfoLocationInBeacon;
} MULTISSID_STRUCT, *PMULTISSID_STRUCT;



typedef struct GNU_PACKED _AP_ADMIN_CONFIG {
	/***********************************************/
	/*Reference to BSSID */
	/***********************************************/
	MULTISSID_STRUCT MBSSID;
	
	COMMON_CONFIG CommonCfg;

	TXWI_STRUC BeaconTxWI;
	UINT8 TXWISize;
	USHORT Aid;
	UCHAR DtimCount;	/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;	/* default = 3 */
	
#ifdef BNC_UPDATE_PERIOD /*Default turn off,  will cause system halt*/
	ULONG TbttTickCount;	/* beacon timestamp work-around */
#endif

	/*MAC Table*/
	MAC_TABLE MacTab;	/* ASIC on-chip WCID entry table.  At TX, ASIC always use key according to this on-chip table. */

	/*chip cap*/
	USHORT WcidHwRsvNum;

	//NDIS_SPIN_LOCK MacTabLock;
	UCHAR	EntryClientCount;
} AP_ADMIN_CONFIG, *PAP_ADMIN_CONFIG;


VOID APAuthPMKSet(void)  XIP_ATTRIBUTE(".xipsec0");
VOID APCfgUpdate(UINT8 *pSSID, UINT8 AuthMode, UINT8 *pPassword, UINT8 Channel) XIP_ATTRIBUTE(".xipsec0");
INT AsicSetPreTbttInt(BOOLEAN enable) XIP_ATTRIBUTE(".xipsec0");
VOID AsicEnableBssSync(VOID) XIP_ATTRIBUTE(".xipsec0");
VOID APMakeBssBeacon(VOID) XIP_ATTRIBUTE(".xipsec0");
VOID APUpdateBeaconFrame(VOID) XIP_ATTRIBUTE(".xipsec0");
VOID APStartUp(VOID) XIP_ATTRIBUTE(".xipsec0");
VOID APStop(VOID);
VOID PreApCfgInit(VOID)  XIP_ATTRIBUTE(".xipsec0");
INT  APInitialize(VOID) XIP_ATTRIBUTE(".xipsec0");

VOID set_entry_phy_cfg(MAC_TABLE_ENTRY *pEntry) XIP_ATTRIBUTE(".xipsec0");
VOID APSendNullFrame(IN PUCHAR pAddr, IN UINT8 pwrMgmt) XIP_ATTRIBUTE(".xipsec0");
void MacTableMaintenance();
MAC_TABLE_ENTRY *MacTableLookup(PUCHAR pAddr) XIP_ATTRIBUTE(".xipsec0");
BOOLEAN MacTableDeleteEntry(IN USHORT wcid) XIP_ATTRIBUTE(".xipsec0");
MAC_TABLE_ENTRY *MacTableInsertEntry(IN  PUCHAR	pAddr,IN BOOLEAN CleanAll) XIP_ATTRIBUTE(".xipsec0");
VOID MacTableReset(VOID) XIP_ATTRIBUTE(".xipsec0");

VOID store_ap_cfg(VOID);
BOOLEAN reset_ap_cfg(VOID);

#ifdef BNC_UPDATE_PERIOD /*Default turn off,  will cause system halt*/
VOID BeaconUpdateExec(IN PVOID pParm1, IN PVOID pParm2);
#endif
VOID BeaconFrameSendAction(UINT_32 param, UINT_32 param2);

#endif //_AP_PUB_H_

