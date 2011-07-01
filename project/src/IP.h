#ifndef _IP_h
#define _IP_h
//======================================================================
//             IP.H
//
// DESCRIPTION: This file contains the API for Building the IP Packet
//
// AUTHOR: DENSO INTERNATIONAL AMERICA, INC.
//         LA Laboratories
//         Bassam Masri
//
//
// VERSION: This is a preliminary version being provided to the VSCC
//          for information purposes.  DENSO reserves the right to 
//          make changes without prior approval from the VSCC.
//
//======================================================================
#ifndef BIG_ENDIAN
#define BIG_ENDIAN    1
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 0
#endif

/*
 * Definitions for internet protocol version 4.
 * Per RFC 791, September 1981.
 */
#define IPVERSION         4
#define IPROTOCOL         255
#define PORT              1024
#define IP_HEADER         60

/*
 * Internet implementation parameters.
 */
#define MAXTTL      255     /* maximum time to live (seconds) */

/* IP Header strcuture definition */
struct iphdr 
{
    #if BYTE_ORDER == LITTLE_ENDIAN
        uint8_t  ip_hl:4,      /* header length */
                 ip_v:4;       /* version */
    #endif

    #if BYTE_ORDER == BIG_ENDIAN
        uint8_t  ip_v:4,       /* version */
                 ip_hl:4;      /* header length */
    #endif

        uint8_t  ip_tos;       /* type of service */
        uint16_t ip_len;       /* total length */
        uint16_t ip_id;        /* identification */
        uint16_t ip_off;       /* fragment offset field */

        uint8_t  ip_ttl;       /* time to live */
        uint8_t  ip_p;         /* protocol */
        uint16_t ip_sum;       /* checksum */
        uint32_tp ip_src, ip_dst; /* source and dest address */
};

/* Option field structure definition */
struct ipoptions_header
{
    #if BYTE_ORDER == LITTLE_ENDIAN
        uint8_t  ipopt_option:5,   /* Option number */
                 ipopt_class:2,    /* Class */
                 ipopt_cpy:1;      /* copy flag */
    #endif
    #if BYTE_ORDER == BIG_ENDIAN              
        uint8_t   ipopt_cpy:1,      /* copy flag */
                  ipopt_class:2,    /* Class */
                  ipopt_option:5;   /* Option number */
    #endif
        uint8_t  ipopt_len;        /* total length */
};


//=====================================================
//  WAVE specific fields
//=====================================================
/*
 * Structure of WAVE Tx Options field.

 */
struct wave_tx_options
{
        ipoptions_header ipopts;
        uint16_t ch_pwr_data;      /* Channel, Power, and data rate */
        uint16_t fragmentation_threshold;
        uint16_t rts_cts_threshold;
        uint8_t  mac_address[MAX_MAC_ADDRESS];
    
    #if BYTE_ORDER == LITTLE_ENDIAN
        uint8_t   bandwidth:2,      /* Bandwidth */
                  antenna:2,        /* which antenna to use */
                  unit_mode:2,      /* Unit Mode (OBU or RSU) */
                  service_mode:2;   /* Service Mode (Private or Public) */
    #endif
    #if BYTE_ORDER == BIG_ENDIAN              
        uint8_t   service_mode:2,   /* Service Mode (Private or Public) */
                  unit_mode:2,      /* Unit Mode (OBU or RSU) */
                  antenna:2,        /* which antenna to use */
                  bandwidth:2;      /* Bandwidth */
    #endif
        uint8_t  rsv;              /* RESERVED */
};

/*
 * Structure of WAVE Configuration Options field.
 */
struct wave_cfg_options
{
        ipoptions_header ipopts;
        uint16_t ch_pwr_data;      /* Channel, Power, and data rate */
        uint16_t fragmentation_threshold;
        uint16_t rts_cts_threshold;
    #if BYTE_ORDER == LITTLE_ENDIAN
        uint8_t   bandwidth:2,      /* Bandwidth */
                  antenna:2,        /* which antenna to use */
                  unit_mode:2,      /* Unit Mode (OBU or RSU) */
                  service_mode:2;   /* Service Mode (Private or Public) */
        sint8_t   ant1comp;
        sint8_t   ant2comp;
    #endif
    #if BYTE_ORDER == BIG_ENDIAN              
        uint8_t   service_mode:2,   /* Service Mode (Private or Public) */
                  unit_mode:2,      /* Unit Mode (OBU or RSU) */
                  antenna:2,        /* which antenna to use */
                  bandwidth:2;      /* Bandwidth */
        sint8_t   ant1comp;
        sint8_t   ant2comp;
    #endif
        uint8_t   rsv;              /* RESERVED */
        uint8_t   mac_address[MAX_MAC_ADDRESS];
        uint16_t  rsv1;             /* RESERVED */
};

/*
 * Structure of Default WAVE request.
 */

struct wave_default_req
{
        ipoptions_header ipopts;
        uint16_t         rsv;               /* RESERVED */
};

/*
 * Structure of Configuration request.
 */
struct wave_cfg_req
{
        ipoptions_header ipopts;
        uint16_t         rsv;               /* RESERVED */
};

/*
 * Structure of an WAVE Rx Options field.
 */
struct wave_rx_options
{
        ipoptions_header ipopts;
        sint16_t rssi;
        uint8_t  mac_header[MAX_MAC_HEADER];
    #if BYTE_ORDER == BIG_ENDIAN              
        uint8_t  antenna:2,    /* The antenna which the data was received */
                 rsv1:6;
    #endif
    #if BYTE_ORDER == LITTLE_ENDIAN
        uint8_t  rsv1:6,
                 antenna:2;    /* The antenna which the data was received */
    #endif
                           
        uint8_t  rsv;         
};

/*
 * Structure of an WRM Tallies Options field.
 */
struct wrm_tallies_req
{
        ipoptions_header ipopts;
        uint16_t         rsv;               /* RESERVED */
};
                                    
//=========================================================================================
//   FUNCTION:     STATUS_TYPE build_ip_pkt(/* Input  */ void *payload, uint32_t payload_len, 
//                                          uint8_t *src_ip, uint8_t *dst_ip, 
//                                          struct ipoptions_header * wave_options, 
//                                          /* output */ void *ip_pkt)
// 
//
//   DESCRIPTION:  Build IP Packet with the Payload
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE build_ip_pkt(/* Input  */ const void *payload, uint32_tp payload_len, 
                         in_addr_t ipSrc, in_addr_t ipDest, struct ipoptions_header * wave_options, 
                         /* output */ void *ip_pkt);
  
#endif  /* _IP_h */
