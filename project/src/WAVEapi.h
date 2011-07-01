#ifndef _WAVEAPI_H
#define _WAVEAPI_H

//======================================================================
//             WAVEAPI.H
//
// DESCRIPTION: This file contains the API for communication with 
//              the WAVE Radio Module (WRM)
//
// AUTHOR: DENSO INTERNATIONAL AMERICA, INC.
//         LA Laboratories
//         Bassam Masri
//
// DATE: June 18, 2004
//
// VERSION: This is a preliminary version being provided to the VSCC
//          for information purposes.  DENSO reserves the right to 
//          make changes without prior approval from the VSCC.
//
//======================================================================

#include <arpa/inet.h>

//======================================================
// All the values specified below are based on the WRM 
// Interface Specification.  Please refer to this
// document for further clarification
//======================================================

/* Maximum size of the payload */
#define MAX_PAYLOAD_SIZE       1500
/* Size of the source MAC header */
#define MAX_MAC_HEADER        30
/* Size of the WRM MAC address */
#define MAX_MAC_ADDRESS       6

/* Fragmentation and RTS/CTS threshold valid range */
#define MIN_THRESHOLD         256     /* in Bytes */
#define MAX_THRESHOLD         2346    /* in Bytes */

/* Valid range for antenna compensation factor */
#define MIN_ANT_COMP          -10     /* in dB */
#define MAX_ANT_COMP          +29     /* in dB */

/* Transmit power at SMA connector */
#define MIN_TXPOWER           00      /* Sets the WRM transmit power to 0 dBm 
                                         (minimum level supported by the WRM) */
#define MAX_TXPOWER           20      /* Sets the WRM transmit power to 20 dBm 
                                         (maximum level supported by the WRM) */
#define FULL_TXPOWER          0x3F    /* Sets the WRM transmit power to the full power in dBm
                                         May or may not be higher than 20 dBm */

typedef unsigned char BYTE;
typedef unsigned char byte;
typedef unsigned short WORD;
typedef char *PCHAR;
typedef long LONG;
typedef  BYTE uint8_t;
typedef  WORD uint16_t;
typedef  LONG uint32_tp;
typedef unsigned long DWORD;

typedef  signed char  sint8_t;
typedef  signed short sint16_t;
typedef  signed long  sint32_t;

/* This type is used for the WAVE function
   return status */
typedef enum
{
    SUCCESS = 0x00,
    FAILURE

} STATUS_TYPE;

/* type definition for configuration parameters */
typedef uint16_t WRM_TXPOWER;
typedef uint16_t WRM_THRESHOLD;
typedef sint16_t WRM_DATARATE;
typedef sint16_t WRM_RSSI;
typedef sint8_t  WRM_ANTENNA_COMP;

/* WRM Service mode Public safety or Private Service */
typedef enum 
{
    CONNECTED  = 0x00,    
    DISCONNECTED 

} WRM_CONNECTION_STATUS;

/* WRM Service mode Public safety or Private Service */
typedef enum 
{
    SERVICE_MODE_PUBLIC  = 0x00,    
    SERVICE_MODE_PRIVATE 

} WRM_SERVICE_MODE;


/* WRM Unit mode - OnBoard Unit or Road Side Unit 
   This enum effects the maximum allowable transmit power */
typedef enum 
{
    UNIT_MODE_OBU = 0x00,  
    UNIT_MODE_RSU    

} WRM_UNIT_MODE;

/* WRM transmit and receive antenna */
typedef enum 
{
    ANTENNA_1    = 0x00,  /* Transmit and receive on Antenna 1 */
    ANTENNA_2,            /* Transmit and receive on Antenna 2 */
    ANTENNA_BEST         /* Transmit on Antenna 1, enable receive diversity */

} WRM_ANTENNA_MODE;

/* Bandwidth settings are valid only for 802.11a channels
   see WRM_CHANNEL for a complete list of channels */
typedef enum 
{
    BANDWIDTH_10 = 0x00,  /* 10MHz channel */
    BANDWIDTH_20         /* 20MHz channel */

} WRM_BANDWIDTH;


typedef enum 
{
    /* List of valid 
       WAVE channels */
    CHANNEL_172  = 0x00,    /* 10MHz channel */
    CHANNEL_174,            /* 10MHz channel */
    CHANNEL_175,            /* 20MHz channel */
    CHANNEL_176,            /* 10MHz channel */
    CHANNEL_178,            /* 10MHz channel */
    CHANNEL_180,            /* 10MHz channel */
    CHANNEL_181,            /* 20MHz channel */
    CHANNEL_182,            /* 10MHz channel */
    CHANNEL_184,            /* 10MHz channel */
    /* List of valid 802.11a channels 
       20MHz channels */
    RESERVED1,
    RESERVED2,             /* 20MHz channel */
    RESERVED3,             /* 20MHz channel */
    RESERVED4,             /* 20MHz channel */
    CHANNEL_52,             /* 20MHz channel */
    CHANNEL_56,             /* 20MHz channel */
    CHANNEL_60,             /* 20MHz channel */
    CHANNEL_64,             /* 20MHz channel */
    CHANNEL_149,            /* 20MHz channel */
    CHANNEL_153,            /* 20MHz channel */
    CHANNEL_157,            /* 20MHz channel */
    CHANNEL_161             /* 20MHz channel */

} WRM_CHANNEL;

/* List of data rates for 20MHz channels */
typedef enum 
{
    DATARATE_06MBPS_20MHz = 0x01,
    DATARATE_09MBPS_20MHz,
    DATARATE_12MBPS_20MHz,
    DATARATE_18MBPS_20MHz,
    DATARATE_24MBPS_20MHz,
    DATARATE_36MBPS_20MHz,
    DATARATE_48MBPS_20MHz,
    DATARATE_54MBPS_20MHz

} WRM_20MHZ_DATA_RATE;

/* List of data rates for 10Mhz channels */
typedef enum 
{
    DATARATE_03MBPS_10MHz = 0x01,
    DATARATE_4_5MBPS_10MHz,
    DATARATE_06MBPS_10MHz,
    DATARATE_09MBPS_10MHz,
    DATARATE_12MBPS_10MHz,
    DATARATE_18MBPS_10MHz,
    DATARATE_24MBPS_10MHz,
    DATARATE_27MBPS_10MHz

} WRM_10MHZ_DATA_RATE;

/* WAVE Radio Module configuration 
   data structure - This structure is passed 
   to WAVE API functions to get or set the WRM configuration.
   This structure is used by the following functions:
   1.  set_WRM_configuration
   2.  get_WRM_configuration
   3.  send_WAVE_tx_pkt */

typedef struct 
{
    WRM_CHANNEL        channel;                 /* WRM transmit channel */
    WRM_TXPOWER        txpower;                 /* WRM Tx Power value */
    WRM_DATARATE       datarate;                /* WRM transmit data rate */


    WRM_THRESHOLD      fragthreshold;           /* WRM fragmentation threshold */
    WRM_THRESHOLD      rtsctsthreshold;         /* WRM RTS / CTS threshold */
    WRM_SERVICE_MODE   servicemode;             /* Service mode - Public or Private */
    WRM_UNIT_MODE      unitmode;                /* Unit mode OBU or RSU */
    WRM_ANTENNA_MODE   antenna;                 /* Transmit and receive antenna selection */
    WRM_BANDWIDTH      bandwidth;               /* Channel Bandwidth */
    /* Antenna Compensation factors are not used
       by the send_WAVE_tx_pkt function call */
    WRM_ANTENNA_COMP   ant1comp;                /* Antenna 1 Compensation Factor (dB) 
                                                   in two's complement. A positive value
                                                   represents a gain between the WRM
                                                   SMA connector and the antenna input.  
                                                   A negative value represents a loss */
    WRM_ANTENNA_COMP   ant2comp;                /* Antenna 2 Compensation Factor (dB) 
                                                   in two's complement */
    /* The usage of mac_address field is dependent on the calling function:
       - set_WRM_configuration function does not use this field
       - get_WRM_configuration returns the WRM MAC Address
       - send_WAVE_tx_pkt uses this field as the destination MAC Address */
    uint8_t            mac_address[MAX_MAC_ADDRESS];  
} wrm_configuration_parameters_type;
//typedef struct wrm_configuration_parameters_type *wrmapi;
/* WRM tallies structure */
typedef struct 
{
    /* Receive related stats */
    uint32_tp	GoodReceives;
    uint32_tp	RxUnicastFrames;
    uint32_tp	RxBroadcastFrames;
    uint32_tp	MulticastReceives;
    uint32_tp	RxDiscardFrames;
    uint32_tp	GoodReceiveBytes;
    uint32_tp	RxMgmtFrames;
    uint32_tp	RxCtrlFrames;
    uint32_tp	RxDataFrames;
    uint32_tp	RxBeacons;
    uint32_tp	RxProbeResponses;
    uint32_tp    ReceiveErrors;
    uint32_tp    RcvCrcErrors;
    uint32_tp    FcsFailCnt;
    uint32_tp    RcvKeyCacheMisses;
    uint32_tp    RcvDmaOverrunErrors;
    uint32_tp    RcvPhyErrors;
    uint32_tp    RcvDupSeqErrors;
    uint32_tp    MultipleRxDuplicates;
    uint32_tp    RcvDecryptCrcErrors;
    uint32_tp    RcvDecompCrcErrors;
    uint32_tp    RcvDecipherErrors;
    uint32_tp    RcvDemicErrors;
    uint32_tp    RcvDeMMHErrors;
    uint32_tp    RcvWEPExcludedCount;

    /* Transmit related stats */
    uint32_tp    GoodTransmits;
    uint32_tp    TxUnicastFrames;
    uint32_tp    TxBroadcastFrames;
    uint32_tp    TxMulticastFrames;
    uint32_tp    TxFramesDropped;
    uint32_tp    GoodTransmitBytes;
    uint32_tp    TxProbeRequests;
    uint32_tp    TxDataFrames;
    uint32_tp    TxMgmtFrames;
    uint32_tp    TxCtrlFrames;
    uint32_tp    TxInvalidFrames;
    uint32_tp    TransmitErrors;
    uint32_tp    TxExcessiveRetries;
    uint32_tp    TxDmaUnderrun;
    uint32_tp    AckRcvFailures;
    uint32_tp    RtsSuccessCnt;
    uint32_tp    RtsFailCnt;

    uint32_tp    compSuccessCnt;
    uint32_tp    CompCPC0Cnt;
    uint32_tp    CompCPC1Cnt;
    uint32_tp    CompCPC2Cnt;
    uint32_tp    CompCPC3Cnt;
    uint32_tp    TxFiltered;
    uint32_tp    TotalRetries;
    uint32_tp    RetryBins[16];
    uint32_tp    shortFrameRetryBins[16];
    uint32_tp    swRetryTotalCnt;
    uint32_tp    swRetryMaxRetriesExceeded;
    uint32_tp    psMulticastFrames;
    uint32_tp    psRequestFrames;
    uint32_tp    psAgedFrames;
    uint32_tp    psSavedFrames;
    sint32_t    ackRssi;
    sint32_t    rssi;
    uint32_tp    RcvRate;
    uint32_tp    txRateKb;           // Averaged rate used for transmit
    uint32_tp    RxAntenna;
    uint32_tp    AntCnt[2];
    uint32_tp    AntSwCnt;
    uint32_tp    DataRateMismatches; // Mismatch of basic rates

    /* MLME related stats */
    uint32_tp    Associations;
    uint32_tp    Reassociations;
    uint32_tp    DisAssociations;
    uint32_tp    DisAssocReasonCode;
    uint32_tp    Authentications;
    uint32_tp    DeAuthentications;
    uint32_tp    DeAuthReasonCode;
    uint32_tp    AuthReqRefused;
    uint32_tp    AssocReqRefused;
    uint32_tp    AssocTimeouts;
    uint32_tp    AuthTimeouts;
    uint32_tp    HungCount; 
    uint32_tp    hwResetCount;    
    uint32_tp    osResetCount;   
    uint32_tp    NdisSendRequest; 
    uint32_tp    NdisSendReRequest;
    uint32_tp    PrefAPMismatches;
    uint32_tp    txDisAssocReasonCode;
    uint32_tp    txDeAuthReasonCode;
    uint32_tp    txAuthenticateFailStatus;
    uint8_t     txDisAssociationSta[MAX_MAC_ADDRESS];  
    uint8_t     txDeAuthenticationSta[MAX_MAC_ADDRESS];
    uint8_t     txAuthenticateFailSta[MAX_MAC_ADDRESS]; 

} wrm_tallies_parameters_type;

/* Received packet configuration data structure */
typedef struct 
{
    uint8_t            mac_header[MAX_MAC_HEADER];
    WRM_RSSI           rssi;       /* in dBm */


    WRM_ANTENNA_MODE   antenna;    /* receive antenna */

} rx_pkt_configuration_parameters_type;

//======================================================================
//   FUNCTION:     STATUS_TYPE init_WAVE_comm(void)
//
//   DESCRIPTION:  This function initializes communication with the WRM.  
//                 Applications should call this function once upon initialization.
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//======================================================================
STATUS_TYPE init_WAVE_comm(in_addr_t ipSrc, in_addr_t ipSubnet, const QString & strDeviceName);

//=========================================================================================
//   FUNCTION:     CONNECTION_TYPE CALLBACK get_Connection_status(void)
//
//   DESCRIPTION:  This function retrieves the WRM connection Status
//
//   RETURN        CONNECTED if the function is successful
//                 DISCONNECTED if the function is unsuccessful
//=========================================================================================
WRM_CONNECTION_STATUS get_Connection_status (void);

//=========================================================================================
//   FUNCTION:     void term_WAVE_comm(void)
//
//   DESCRIPTION:  This function terminates communication with the WRM.  
//                 Applications should call this function once upon closing.
//
//=========================================================================================
void term_WAVE_comm(void);

//=========================================================================================
//   FUNCTION:     STATUS_TYPE set_WRM_WAVEdefault (void) 
//
//   DESCRIPTION:  This function configures the WRM to the WAVE default parameters.
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE set_WRM_WAVEdefault (void);

//=========================================================================================
//   FUNCTION:     STATUS_TYPE set_WRM_configuration (wrm_configuration_parameters_type *pconfig) 
//
//   DESCRIPTION:  This function sends WAVE configuration parameters to the WRM.
//
//   INPUT         wrm_configuration_parameters_type: WRM configuration data structure
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE set_WRM_configuration (wrm_configuration_parameters_type *pconfig);

//=========================================================================================
//   FUNCTION:     get_WRM_configuration (wrm_configuration_parameters_type *pconfig) 
//
//   DESCRIPTION:  This function gets WAVE configuration parameters from the WRM.
//
//   OUTPUT        wrm_configuration_parameters_type: WRM configuration data structure
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE get_WRM_configuration (wrm_configuration_parameters_type *pconfig, in_addr_t & ipWRM);

//=========================================================================================
//   FUNCTION:     STATUS_TYPE _stdcall send_WAVE_tx_pkt (wrm_configuration_parameters_type *pconfig, 
//                                                        char DestIP[], char *payload, uint16_t payloadLen);
//
//   DESCRIPTION:  This function sends configuration parameters and the payload to the WRM.
//                 The WRM sets its configuration to the specified parameters and 
//                 transmits the payload.  Applications use this for packet transmission.
//
//   INPUT         wrm_configuration_parameters_type: WRM configuration data structure.
//                 DestIP:  Destination of the Host Device IP address
//                 payload: outgoing payload data
//                 payloadLen: outgoing payload data length in bytes (up to MAX_PAYLOAD_SIZE)
//                 ant1comp, ant2comp parameters in wrm_configuration_parameters_type are not 
//                 used by this function call
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//
//=========================================================================================
STATUS_TYPE send_WAVE_tx_pkt (wrm_configuration_parameters_type *pconfig, 
                                                 in_addr_t ipDest, const unsigned char *payload, uint16_t payloadLen);


//=========================================================================================
//   FUNCTION:     STATUS_TYPE _stdcall send_WAVE_tx_pkt_wo_config (char *payload, uint16_t payloadLen)
//
//   DESCRIPTION:  This function sends the payload to the WRM without configuration
//
//   INPUT         DestIP:  Destination of the Host Device IP address
//                 payload: outgoing payload data
//                 payloadLen: outgoing payload data length in bytes (up to MAX_PAYLOAD_SIZE)
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//
//=========================================================================================
STATUS_TYPE send_WAVE_tx_pkt_wo_config (in_addr_t ipDest, const char unsigned *payload, uint16_t payloadLen);

//=========================================================================================
//   FUNCTION:     STATUS_TYPE check_WAVE_rx_pkt (void) 
//
//   DESCRIPTION:  This function checks if a packet has been received.  
//                 If a packet is pending, the application can retrieve it using
//                 the receive_WAVE_rx_pkt function.  This function enables an application
//                 to check for receive packets on a non-blocking basis.



//
//   RETURN        SUCCESS if packet is pending
//                 FAILURE if no packet pending
//=========================================================================================
STATUS_TYPE check_WAVE_rx_pkt (void);


//=========================================================================================
//   FUNCTION:     STATUS_TYPE receive_WAVE_rx_pkt (rx_pkt_configuration_parameters_type *pconfig,
//                                                  char *payload, uint16_t *payloadlen) 
//
//   DESCRIPTION:  This function gets a WAVE packet that was received from the WRM.  If no 
//                 packet has been received, this function will block (i.e., not return
//                 to the application) until a packet has been received.  Applications
//                 may use check_WAVE_rx_pkt if non-blocking operation is desired.
//
//   OUTPUT        rx_pkt_configuration_parameters_type: received packet configuration parameters
//                 SrcIP: Remote Host Device IP Address
//                 payload: incoming packet data
//                 payloadLen: incoming packet data length in bytes (up to MAX_PAYLOAD_SIZE)
//
//   RETURN        SUCCESS if a packet was successfully received
//                 FAILURE if rx_pkt_configuration_parameters_type parameter is NULL
//=========================================================================================
STATUS_TYPE receive_WAVE_rx_pkt (rx_pkt_configuration_parameters_type *pconfig,
                                                    struct in_addr * pSrcIP, unsigned char *payload, uint16_t *payloadlen);

//=========================================================================================
//   FUNCTION:     get_WRM_tallies (wrm_tallies_parameters_type *ptallies) 
//
//   DESCRIPTION:  This function gets WRM tallies
//
//   OUTPUT        wrm_tallies_parameters_type: WRM tallies data structure
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful

//=========================================================================================
STATUS_TYPE get_WRM_tallies (wrm_tallies_parameters_type *ptallies);

#endif /* _WAVEAPI_H */
