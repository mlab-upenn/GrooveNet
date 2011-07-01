//======================================================================
//             IP.CPP
//
// DESCRIPTION: This file contains the API for building IP Packets
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
//#include "stdafx.h"
#include "Global.h"
#include "WAVEapi.h"
#include "IP.h"
#include <stddef.h>
#include <sys/types.h>
//#include <net/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>


//=========================================================================================
//   FUNCTION:     uint16_t checksum(uint16_t *data,uint32_t len) 
//
//   DESCRIPTION:  This function calculates the IP Packet Checksum including the header
//
//   RETURN        Calculated checksum
//=========================================================================================
uint16_t checksum(uint16_t *data,uint32_tp len) 
{
    /* data is the one for which we are calculating checksum and its size
       len in bytes */

    register uint32_tp sum=0;
    register uint32_tp odd = len;
    register uint16_t *temp=data;
    uint16_t rvalue;

    /* adding 16 bits every time to sum */
    while (odd > 1) 
    {
       sum += *(temp++);
       odd -= 2;
    }

    /* if length is odd the last 8 bits will also get added here */
    if (odd)
       sum += *(uint8_t *)temp;

    sum = (sum & 0xffff) + (sum >> 16);
    /* truncating to 16 bits */
    rvalue = (uint16_t) ~sum; 
    
    return rvalue;
}

//=========================================================================================
//   FUNCTION:     STATUS_TYPE build_iphdr(/* Input */ void *header_buffer, uint32_t payload_len, 
//                                         uint8_t *src_ip, uint8_t *dst_ip, 
//                                         struct ipoptions_header * wave_options) 

//
//   DESCRIPTION:  This function build an IP Packet header
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE build_iphdr(/* Input */ void *header_buffer, uint32_tp payload_len, in_addr_t ipSrc, 
                     in_addr_t ipDest, struct ipoptions_header * wave_options)
{
    STATUS_TYPE rvalue = SUCCESS;
    
    /* Pointer to the allocated IP hdr structure */
    struct iphdr *ip=(struct iphdr *) header_buffer;

    /* Build the IP header */
    ip->ip_tos = 0;
    ip->ip_id  = 0;
    ip->ip_off = 0;
    ip->ip_v   = IPVERSION;
    ip->ip_ttl = MAXTTL;
    ip->ip_p   = IPROTOCOL;
    ip->ip_hl  = (sizeof(struct iphdr) + wave_options->ipopt_len) >> 2 ; // Divide by 4 for the size in long words
    ip->ip_len = htons(sizeof(struct iphdr) + wave_options->ipopt_len + payload_len);

    /* Add the IP Packet Source Address */
    ip->ip_src = ipSrc;
    /* IP Packet Destination Address */
    ip->ip_dst = ipDest;

    /* Copy IP Option field */
    memcpy ((char *)header_buffer + sizeof(iphdr), wave_options, wave_options->ipopt_len);

    /* Calculate the checksum and copy it to the IP header */
    ip->ip_sum = htons(checksum((unsigned short *)ip, sizeof(struct iphdr) + wave_options->ipopt_len));

    return rvalue;
}

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
                       in_addr_t ipSrc, in_addr_t ipDest, 
                       struct ipoptions_header * wave_options, 
                       /* output */ void *ip_pkt)
{
    STATUS_TYPE rvalue = SUCCESS;
    
    /* Append the payload to the IP Packet header */
    if (payload != NULL && payload_len != 0)
        memcpy (((char *)ip_pkt + sizeof(iphdr)) + wave_options->ipopt_len, payload, payload_len);

    /* Build IP Packet Header */
    if( (build_iphdr (ip_pkt, payload_len, ipSrc, ipDest, wave_options) < 0 ))
    { 
        perror("Build IP failed:"); 
        rvalue = FAILURE;
    }

    return rvalue;
}
