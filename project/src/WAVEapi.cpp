//======================================================================
//             WAVEAPI.CPP
//
// DESCRIPTION: This file contains the API for communication with 
//              the WAVE Radio Module (WRM)
//
// AUTHOR: Nawaporn Wisitpongphan
//         Carnegie Mellon University
// 
// AUTHOR: DENSO INTERNATIONAL AMERICA, INC.
//         LA Laboratories
//         Bassam Masri
//
// Last Modified: 06/20/05
//
// VERSION: This is a modified linux-based API version which requires
//          libpcap library (libpcap-0.8.3) with additional pcap_sendpacket
//          function added to pcap-linux.c located at /usr/src/libpcap-0.8.3.
//
// History: Converted original code to linux-based code
//          Got rid of unused functions
//          
//
//======================================================================


#include <sys/socket.h>
#include <arpa/inet.h>
#include <qthread.h>
#include "Global.h"
#include "WAVEapi.h"
#include "queue.h"
#include "IP.h"
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include "unpifi.h"
#include "Network.h"
#include "Logger.h"
#include "StringHelp.h"
//#include <iostream.h>

#define Max_Num_Adapter 10

#define WAVE_TX_OPTION       25
#define WAVE_RX_OPTION       26
#define WAVE_DEFAULT_REQ     27
#define WAVE_CFG_REQ         28
#define WAVE_CFG_OPTION      29
#define WAVE_TALLIES_REQ     15

#define TIMOUT               500 //Units
#define TIMOUT_TIME          10  //ms


/* Temporary storage for the last received WRM
Configuration Option */

wrm_configuration_parameters_type current_config_option;
wrm_tallies_parameters_type       current_tallies_option;

/* Indicate whether the WRM received a configuration 
packet. This flag is set by the receive thread 
to inform the application that a configuration packets is pending. 
The application thread clears this flag after it reads
the configuration */
bool config_pending = false;

//Linux Code
/*Receive thread ID*/

class PacketRXThread: public QThread
{
public:
	PacketRXThread();
	virtual ~PacketRXThread();
	
	void Start();
	void Stop();

protected:
	virtual void run();

	bool m_bCancelled;
};

PacketRXThread * g_pRXThread;

size_t stacksize = 0;
/*Critical Sections replaced by mutex*/
//pthread_mutex_t configuration_pkt_section;
QMutex g_configuration_pkt_mutex;

/* IP Address of the host device */
/* Broadcast IP Address */
//char src_ip[20];
//char BroadcastIP[20];
in_addr_t g_ipBroadcast;
//char DestIP[20];
//char WRMIP[20];
in_addr_t g_ipWRM;

/* Queue for all WRM received packets.
When WRM receives a packet, the packet 
is queued until it is retrieved by the application thread */
Queue rx_pkt_queue = NULL;



pcap_t *Txconn = NULL;
pcap_t *Rxconn = NULL;

WRM_CONNECTION_STATUS ConnectionStatus = DISCONNECTED;

/* This function is  the receive thread handler
It process all of the incoming packets
from the WRM */
extern unsigned int rx_packets_thread(void* pParam);  //Linux : change return type
extern int ReceivePacket (char rxpacket[]);
extern STATUS_TYPE SendPacket(const char output_pkt[],int pktsize);
void save_WAVE_cfg_msg (wave_cfg_options *wcfg);
extern WRM_CONNECTION_STATUS Init_PCAP (in_addr_t ipSrc, in_addr_t ipSubnet, const QString & strDeviceName);
extern STATUS_TYPE OpenWinpcapDriver (const QString & strDeviceName);
//extern STATUS_TYPE GetAdapterNameFromIp(PCHAR Name);
//extern char* MySock_ntop_host(const struct sockaddr *sa,socklen_t salen);

PacketRXThread::PacketRXThread()
: QThread(), m_bCancelled(false)
{
}

PacketRXThread::~PacketRXThread()
{
}

void PacketRXThread::Start()
{
	m_bCancelled = false;
	QThread::start();
}

#define MAX_DEADLOCK 30000

void PacketRXThread::Stop()
{
	m_bCancelled = true;
	while (!QThread::wait(MAX_DEADLOCK))
		QThread::terminate();
}

void PacketRXThread::run()
{
    ElementType packet;
    char        rx_packet[IP_HEADER+MAX_PAYLOAD_SIZE];
    int         msg_len = 0;
    in_addr_t   ipSrc = htonl(GetIPAddress());
    iphdr *phdr = NULL;
    struct      timespec delay = {0, 10000000};    
    
    /* Open receiving Broadcast Socket */
    memset (rx_packet, 0, sizeof(rx_packet));
    
    while (!m_bCancelled)
    {       
        msg_len = ReceivePacket (rx_packet);
        
        /* bytes received? - Process the packet */
        if (msg_len > 0L)
        {
            /* Point to the packet header */
            phdr = (iphdr *) rx_packet;
            
            /* Filter out non IP messages */
            if (phdr->ip_hl <= 5)
                continue;
            
                /* Igonre all packets that are 
                sent from this Host Device or 
            that are not address to this device */
            if ( ((unsigned)phdr->ip_src != ipSrc) &&
                (((unsigned)phdr->ip_dst == ipSrc) ||
                (phdr->ip_dst & 0xff000000) == 0xff000000))
                
            {                   
                /* Extract the option field */
                ipoptions_header *iopt = (ipoptions_header *)((char *)rx_packet + sizeof(iphdr));
                
                /* Check the option field type */
                switch (iopt->ipopt_option)
                {
                case WAVE_RX_OPTION:
               
                    /* We received a packet, save it to queue */
                    memcpy (packet.Data, rx_packet, msg_len);
                    packet.len = msg_len;
                    /* Queue the packet, the application
                    thread will retrieve it later using
                    receive_WAVE_rx_pkt */
                    Enqueue(packet, rx_pkt_queue);
                    break;
                    
                case WAVE_CFG_OPTION:
                   
                /* Received WRM configuration packet, the application
                thread will retrieve it later using
                    get_WRM_configuration */
                    //in.S_un.S_addr = phdr->ip_src;           //WINDOW
                    g_ipWRM = phdr->ip_src;

                    save_WAVE_cfg_msg ((wave_cfg_options *)iopt);
                    break;

                default:
                    break;
                }
            }        
        }
        nanosleep(&delay, NULL);
    }
}


//=========================================================================================
//   FUNCTION:     STATUS_TYPE send_WAVE_pkt(uint8_t *payload, uint16_t payload_len, char * dst_ip, 
//                                           char * src_ip,struct ipoptions_header * wave_options)
//
//   DESCRIPTION:  This function builds an ip packet with the specified option field and 
//                 sends it to the WRM
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//
//=========================================================================================
STATUS_TYPE send_WAVE_pkt(const uint8_t *payload, uint16_t payload_len, in_addr_t ipDest, 
                          in_addr_t ipSrc, struct ipoptions_header * wave_options)
{
    char * output_pkt;

    STATUS_TYPE rvalue = SUCCESS;

    STATUS_TYPE tmp;

    if ((Txconn != NULL) && (payload_len <= MAX_PAYLOAD_SIZE))
    {
        /* Allccate memory to send the packet */

        if( (output_pkt=(char *) malloc(sizeof(struct iphdr) + wave_options->ipopt_len + payload_len) ) != NULL )
        {
            memset (output_pkt, 0, sizeof(struct iphdr) + wave_options->ipopt_len + payload_len);
            /* Build IP packet including the header and payload */
            if (build_ip_pkt(payload, payload_len, ipSrc, ipDest, wave_options, output_pkt) != -1)
            {
                /* Success - Send the IP packet */
                /* Sending packet to the WRM */
                tmp = SendPacket(output_pkt, sizeof(struct iphdr) + wave_options->ipopt_len + payload_len);

            }
            else
            {
                /* Error occured */
                perror("build_ip_pkt failed:");
                free(output_pkt);
                rvalue = FAILURE;

            }
            /* Free allocated memory */
            free(output_pkt);
        }
        else 
        { 
            /* Error occured */
            perror("Malloc failed:"); 
            rvalue = FAILURE;
        }
    }
    else
    {
        /* Error occured */
        perror("Invalid Socket:\n"); 
        rvalue = FAILURE;
    }
    
    return rvalue;
}

//=========================================================================================
//   FUNCTION:     STATUS_TYPE send_WAVE_tx_pkt (wrm_configuration_parameters_type *pconfig, 
//                                               char *payload, uint16_t payloadLen)
//
//   DESCRIPTION:  This function sends configuration parameters and the payload to the WRM
//                 The WRM sets its configuration to the specified parameters in IP Options field
//                 and transmits the payload.  Applications use this for packet transmission.
//
//   INPUT         wrm_configuration_parameters_type: WRM configuration data structure.
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
                                       in_addr_t ipDest, const unsigned char *payload, uint16_t payloadLen)
{
    /* WAVE TX Option packet */
    wave_tx_options pOpts;
    STATUS_TYPE rvalue = FAILURE;
    
    if (pconfig)
    {
        /* Set the option header */
        pOpts.ipopts.ipopt_cpy = 1;
        pOpts.ipopts.ipopt_class = 2;
        /* Option type according to the WAVE 
        Interface specifications */
        pOpts.ipopts.ipopt_option = WAVE_TX_OPTION;
        pOpts.ipopts.ipopt_len = sizeof(struct wave_tx_options);
        
        /* check ranges of the configuration parameters */
        if ( ( (pconfig->channel >= CHANNEL_172  &&
            (pconfig->channel <= CHANNEL_161) ) &&
            ( (pconfig->servicemode >= SERVICE_MODE_PUBLIC) &&
            (pconfig->servicemode <= SERVICE_MODE_PRIVATE) ) &&
            ( (pconfig->unitmode >= UNIT_MODE_OBU) &&
            (pconfig->unitmode <= UNIT_MODE_RSU) ) &&

            ( (pconfig->bandwidth >= BANDWIDTH_10) &&
            (pconfig->bandwidth <= BANDWIDTH_20) ) &&
            ( (pconfig->antenna   >= ANTENNA_1) &&
            (pconfig->antenna   <= ANTENNA_BEST) ) &&
            ( (pconfig->datarate  >= DATARATE_06MBPS_20MHz) &&
            (pconfig->datarate  <= DATARATE_54MBPS_20MHz) ) &&
            ( (pconfig->fragthreshold  >= MIN_THRESHOLD) &&
            (pconfig->fragthreshold  <= MAX_THRESHOLD) ) &&
            ( (pconfig->rtsctsthreshold  >= MIN_THRESHOLD) &&
            (pconfig->rtsctsthreshold  <= MAX_THRESHOLD) ) &&
            ( ((pconfig->txpower  >= MIN_TXPOWER) &&
            (pconfig->txpower  <= MAX_TXPOWER)) ||
            (pconfig->txpower  == FULL_TXPOWER) ) ) )
        {
            /* Copy the configuration parameters to the option field */
            pOpts.ch_pwr_data = htons((pconfig->channel << 10) | 
                (pconfig->txpower << 4)  | 
                (pconfig->datarate));
            pOpts.fragmentation_threshold = htons(pconfig->fragthreshold);
            pOpts.rts_cts_threshold = htons(pconfig->rtsctsthreshold);
            pOpts.service_mode = pconfig->servicemode;
            pOpts.unit_mode = pconfig->unitmode;
            pOpts.antenna = pconfig->antenna;
            pOpts.bandwidth = pconfig->bandwidth;
            pOpts.rsv = 0x00;
            /* copy the MAC address */

            memcpy (pOpts.mac_address, pconfig->mac_address, sizeof(pOpts.mac_address));
            
            /* Send the packet and with the payload */
            rvalue = send_WAVE_pkt(payload, payloadLen, ipDest, htonl(GetIPAddress()), (ipoptions_header *) &pOpts);
        }
    }
    
    return rvalue ;
}



//=========================================================================================
//   FUNCTION:     STATUS_TYPE _stdcall send_WAVE_tx_pkt_wo_config (char *payload, uint16_t payloadLen)
//
//   DESCRIPTION:  This function sends the payload to the WRM without configuration
//
//   INPUT         payload: outgoing payload data
//                 payloadLen: outgoing payload data length in bytes (up to MAX_PAYLOAD_SIZE)
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//
//=========================================================================================
STATUS_TYPE send_WAVE_tx_pkt_wo_config (in_addr_t ipDest, const unsigned char *payload, uint16_t payloadLen)
{
    /* WAVE TX Option packet */
    wave_tx_options pOpts;
    STATUS_TYPE rvalue = FAILURE;
    
    /* Zero out the configuration
    THis will be translated as no Option field*/
    memset (&pOpts, 0, sizeof(pOpts));
    
    /* Send the packet and with the payload */
    rvalue = send_WAVE_pkt(payload, payloadLen, ipDest, htonl(GetIPAddress()), (ipoptions_header *) &pOpts);
    
    return rvalue ;
}
//=========================================================================================
//   FUNCTION:     SendPacket(char output_pkt[],int pktsize)
//
//   DESCRIPTION:  This function Sends IP packet to the Ethernet Interface
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE SendPacket (const char output_pkt[], int pktsize)
{
    int res = 0;
    static BYTE packet[10000];
    
    /* Set the Destination MAC */
    packet[0] = packet[1] = packet[2] = packet[3] = packet[4] = packet[5] = 0xff;
    /* Set the Source MAC */
    packet[6] = packet[7] = packet[8] = packet[9] = packet[10] = packet[11] = 0xff;
    
    /* Set the protocol */
    packet[12] = 0x08; packet[13] = 0x00;
        memcpy ((packet + 14), output_pkt, pktsize);
    
    /* Read the packets */

//    if(NULL == Txconn) cout << "\nIn SendPacket(): Invalid connection\n";
    
    res = pcap_sendpacket((pcap_t*)Txconn, (unsigned char *) packet, pktsize + 14);
    
    if(res == 0)  {
        return FAILURE;
    }
    else return SUCCESS;
}
//=========================================================================================
//   FUNCTION:     save_WAVE_cfg_msg (wave_cfg_options *wcfg)
//
//   DESCRIPTION:  This function saves the WRM received configuration packet for
//                 the application to retrieve it later.
//
//=========================================================================================
void save_WAVE_cfg_msg (wave_cfg_options *wcfg)
{
    /* use ntohs to convert the data to host device format */
    
    /* Only one thread can modify the current_config_option at one time */
//    pthread_mutex_lock(&configuration_pkt_section); //Linux
    g_configuration_pkt_mutex.lock();
    
/* Extract Channel, power and data rate */
    current_config_option.channel = (WRM_CHANNEL) (ntohs(wcfg->ch_pwr_data) >> 10 & 0x001f);
    current_config_option.txpower = ntohs(wcfg->ch_pwr_data) >> 4 & 0x003f;
    current_config_option.datarate = ntohs(wcfg->ch_pwr_data) & 0x00f;
    /* Extract the rest of the configuration parameters */
    current_config_option.fragthreshold = ntohs(wcfg->fragmentation_threshold);
    current_config_option.rtsctsthreshold = ntohs(wcfg->rts_cts_threshold);
    current_config_option.servicemode = (WRM_SERVICE_MODE) wcfg->service_mode;
    current_config_option.unitmode =    (WRM_UNIT_MODE) wcfg->unit_mode;
    current_config_option.antenna =     (WRM_ANTENNA_MODE) wcfg->antenna;
    current_config_option.bandwidth =   (WRM_BANDWIDTH) wcfg->bandwidth;
    current_config_option.ant1comp = wcfg->ant1comp;
    current_config_option.ant2comp = wcfg->ant2comp;
    memcpy (current_config_option.mac_address, wcfg->mac_address, sizeof(current_config_option.mac_address));
    /* Set the config_pending flag to true to inform the application
    of configuration packet is waiting */
    config_pending = true;
    
/* Allow other threads to access the current_config_option */
//    pthread_mutex_unlock(&configuration_pkt_section);  //Linux
    g_configuration_pkt_mutex.unlock();
}

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
STATUS_TYPE check_WAVE_rx_pkt (void)
{

    STATUS_TYPE rvalue = FAILURE;
    
    /* Check if packet is pending */
    if (rx_pkt_queue && !IsEmpty(rx_pkt_queue))
    {
    /* if packet pending return
        success */
        rvalue = SUCCESS;
    }
    
    return rvalue;
}

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
//                 payload: incoming packet data
//                 payloadLen: incoming packet data length in bytes (up to MAX_PAYLOAD_SIZE)
//
//   RETURN        SUCCESS if a packet was successfully received
//                 FAILURE if rx_pkt_configuration_parameters_type parameter is NULL
//=========================================================================================
STATUS_TYPE receive_WAVE_rx_pkt (rx_pkt_configuration_parameters_type *pconfig,
                                          struct in_addr * pSrcIP, unsigned char *payload, uint16_t *payloadlen)

{
    ElementType packet;
    STATUS_TYPE rvalue = FAILURE;

    if (pconfig)
    {
        /* Check the receive thread queue */
        if (rx_pkt_queue && !IsEmpty(rx_pkt_queue))
        {
            /* Retrieve the next packet from the queue */
            packet = FrontAndDequeue(rx_pkt_queue);
            /* Point to the packet header */

            iphdr *phdr = (iphdr *) packet.Data;
            // Extract Source IP address
            if (pSrcIP)
                pSrcIP->s_addr = phdr->ip_src;          //Linux
            /* Pointer to the option header */
            ipoptions_header *iopt = (ipoptions_header *)((char *)packet.Data + sizeof(iphdr));
            /* Pointer to the configuration option header */
            wave_rx_options  *wrx  = (wave_rx_options  *)iopt;
            /* Extract the received packet configuration Option field */
            pconfig->rssi = ntohs(wrx->rssi);
            pconfig->antenna = (WRM_ANTENNA_MODE) wrx->antenna;
            memcpy (pconfig->mac_header, (char *) wrx->mac_header, sizeof(wrx->mac_header));
            /* Calculate the payload length which is the packet length minus all the headers */
            *payloadlen = ntohs(phdr->ip_len) - (phdr->ip_hl << 2);
            /* Extract the payload */
            memcpy (payload, (char *)packet.Data + sizeof(struct iphdr)+iopt->ipopt_len, packet.len);

            /* return success */
            rvalue = SUCCESS;
        }
    }
    return rvalue;
}

//=========================================================================================
//   FUNCTION:     ReceivePacket (char rxpacket[]) 
//
//   DESCRIPTION:  This function receives IP packet from the Ethernet Interface if one pending
//
//   OUTPUT        the received packet
//
//   RETURN        Packet Length
//=========================================================================================
int ReceivePacket (char rxpacket[])
{
    int res;
    struct pcap_pkthdr *header;
    u_char *pkt_data;
    /* Read the packets */
    res = pcap_next_ex (Rxconn, &header, (const unsigned char **) &pkt_data);
    
    if(res == 0 || res == -1)
        /* Timeout elapsed */
        return 0;
    
    int pktsize = header->caplen - 14;
    
    if (pktsize > (IP_HEADER+MAX_PAYLOAD_SIZE))

        pktsize = IP_HEADER + MAX_PAYLOAD_SIZE;
    
    memcpy (rxpacket, (pkt_data + 14), pktsize);
    return pktsize;
}




//=========================================================================================
//   FUNCTION:     void start_receive_thread(void)
//
//   DESCRIPTION:  This function start the packet receive thread.
//                 It contains Windows Specific function calls
//
//=========================================================================================
STATUS_TYPE start_receive_thread()
{
//    size_t uiThreadId = 0;
    STATUS_TYPE rvalue = FAILURE;
    
    /* Queue for incoming packets */
    rx_pkt_queue = CreateQueue( MAX_QUEUE_SIZE );
   
 /* Create Cricical section resource
    to allow only one process to access this queue */
//    pthread_mutex_init(&rx_pkt_queue_section,NULL);    //Linux
   
    /* Create Cricical section resource
    to allow only one process to access the configuration option at one time */
//    pthread_mutex_init(&configuration_pkt_section,NULL);   //Linux
   
    /* Create a thread to receive packets */

//    cout << "Create listening thread\n";
//    if (pthread_create(&hThread,NULL,(void*(*)(void *))rx_packets_thread,0)==0) {
    if ((g_pRXThread = new PacketRXThread()) != NULL)
    {
      g_pRXThread->Start();
      rvalue = SUCCESS;
    }
    return rvalue;

}
//======================================================================
//   FUNCTION:     UINT WINAPI rx_packets_thread(LPVOID pParam)
//
//   DESCRIPTION:  This function is the main receive thread procedure 
//                 to receive packets from the WRM Rxconn socket
//
//   RETURN        1 allways
//======================================================================
unsigned int rx_packets_thread(void* pParam)
{
    ElementType packet;
    char        rx_packet[IP_HEADER+MAX_PAYLOAD_SIZE];
    int         msg_len = 0;
    in_addr_t   ipSrc = htonl(GetIPAddress());
    struct      timespec delay = {0, 10000000};    
    
    /* Open receiving Broadcast Socket */
    memset (rx_packet, 0, sizeof(rx_packet));
    
    while (Rxconn != NULL)
    {       
        msg_len = ReceivePacket (rx_packet);
        
        /* bytes received? - Process the packet */
        if (msg_len > 0L)
        {
            /* Point to the packet header */
            iphdr *phdr = (iphdr *) rx_packet;
            
            /* Filter out non IP messages */
            if (phdr->ip_hl <= 5)
                continue;
            
                /* Igonre all packets that are 
                sent from this Host Device or 
            that are not address to this device */
            if ( ((unsigned)phdr->ip_src != ipSrc) &&
                (((unsigned)phdr->ip_dst == ipSrc) ||
                ((phdr->ip_dst >> 24) & 0x000000ff) == 0xff))
                
            {                   
                /* Extract the option field */
                ipoptions_header *iopt = (ipoptions_header *)((char *)rx_packet + sizeof(iphdr));
                
                /* Check the option field type */
                switch (iopt->ipopt_option)
                {
                case WAVE_RX_OPTION:
               
                    /* We received a packet, save it to queue */
                    memcpy (packet.Data, rx_packet, msg_len);
                    packet.len = msg_len;
                    /* Queue the packet, the application
                    thread will retrieve it later using
                    receive_WAVE_rx_pkt */
                    Enqueue(packet, rx_pkt_queue);
                    break;
                    
                case WAVE_CFG_OPTION:
                   
                /* Received WRM configuration packet, the application
                thread will retrieve it later using
                    get_WRM_configuration */
                    //in.S_un.S_addr = phdr->ip_src;           //WINDOW
                    g_ipWRM = phdr->ip_src;

                    save_WAVE_cfg_msg ((wave_cfg_options *)iopt);
                    break;

                default:
                    break;
                }
            }        
        }
        nanosleep(&delay, NULL);
    }
    
   
    pthread_exit(0);  //Linux
    
    return 1L;
} 


//=========================================================================================
//   FUNCTION:     void stop_receive_thread(void)
//
//   DESCRIPTION:  This function stop the packets receive thread.
//                 It contains Windows Specific function calls
//
//=========================================================================================

void stop_receive_thread(void)
{
    /* Terminate the receive thread */
    g_pRXThread->Stop();
    Rxconn = NULL;
//    pthread_join(hThread,NULL);   //Linux
    // Destroy the thread object.
//    pthread_cancel(hThread);       //Linux
    delete g_pRXThread;
    g_pRXThread = NULL;
    /* Free the received packets queue */
    DisposeQueue( rx_pkt_queue );
    rx_pkt_queue = NULL;
    /* Free Critical sections */
//    pthread_mutex_destroy(&configuration_pkt_section);   //Linux
//    pthread_mutex_destroy(&rx_pkt_queue_section);        //Linux
}



//======================================================================
//   FUNCTION:     STATUS_TYPE init_WAVE_comm(void)
//
//   DESCRIPTION:  This function Setup and Open the WAVE socket with the WRM.  
//                 Applications should call this function once upon initialization.
//
//   RETURN        SUCCESS if the function is successful

//                 FAILURE if the function is unsuccessful
//======================================================================
STATUS_TYPE init_WAVE_comm(in_addr_t ipSrc, in_addr_t ipSubnet, const QString & strDeviceName)
{

    STATUS_TYPE rvalue = FAILURE;

    /* Retrieve the Host Device IP address,
    this IP address will be included in every 
    outgoing packet */
    /* get the host name */
    /* Initialize Socket */

    if (get_Connection_status() == DISCONNECTED)
    {
        if (Init_PCAP (ipSrc, ipSubnet, strDeviceName) == CONNECTED)
            rvalue = SUCCESS;
    }
    return rvalue;
}



//=========================================================================================
//   FUNCTION:     void term_WAVE_comm(void)
//
//   DESCRIPTION:  This function terminates Close WAVE Socket with the WRM.  
//                 Applications should call this function once upon closing.
//
//=========================================================================================

void term_WAVE_comm (void)
{
    /* Terminate the receiving thread */

    stop_receive_thread();
    pcap_close(Txconn);
    ConnectionStatus = DISCONNECTED;
    /* Socket Closed */
    Txconn = NULL;
}



//=========================================================================================
//   FUNCTION:     STATUS_TYPE set_WRM_configuration (wrm_configuration_parameters_type *pconfig) 
//
//   DESCRIPTION:  This function sends WAVE configuration parameters to the WRM. It initialize
//                 WAVE Tx Packet with Options field and without payload

//
//   INPUT         wrm_configuration_parameters_type: WRM configuration data structure
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE set_WRM_configuration (wrm_configuration_parameters_type *pconfig)

{
    /* WAVE CFG Option packet */
    wave_cfg_options pOpts;
    STATUS_TYPE rvalue = FAILURE;
    
    if (pconfig)

    {
        /* Set the option header */
        pOpts.ipopts.ipopt_cpy = 1;
        pOpts.ipopts.ipopt_class = 2;
        /* Option type according to the WAVE 

        Interface specifications */
        pOpts.ipopts.ipopt_option = WAVE_CFG_OPTION;
        pOpts.ipopts.ipopt_len = sizeof(struct wave_cfg_options);
        
        /* check the range of the configuration parameters */
        if ( ( (pconfig->channel >= CHANNEL_172  &&
            (pconfig->channel <= CHANNEL_161) ) &&
            ( (pconfig->servicemode >= SERVICE_MODE_PUBLIC) &&
            (pconfig->servicemode <= SERVICE_MODE_PRIVATE) ) &&
            ( (pconfig->unitmode >= UNIT_MODE_OBU) &&
            (pconfig->unitmode <= UNIT_MODE_RSU) ) &&
            ( (pconfig->bandwidth >= BANDWIDTH_10) &&
            (pconfig->bandwidth <= BANDWIDTH_20) ) &&
            ( (pconfig->antenna   >= ANTENNA_1) &&
            (pconfig->antenna   <= ANTENNA_BEST) ) &&
            ( (pconfig->datarate  >= DATARATE_06MBPS_20MHz) &&
            (pconfig->datarate  <= DATARATE_54MBPS_20MHz) ) &&
            ( (pconfig->fragthreshold  >= MIN_THRESHOLD) &&
            (pconfig->fragthreshold  <= MAX_THRESHOLD) ) &&
            ( (pconfig->rtsctsthreshold  >= MIN_THRESHOLD) &&
            (pconfig->rtsctsthreshold  <= MAX_THRESHOLD) ) &&
            ( (pconfig->ant1comp  >= MIN_ANT_COMP) &&
            (pconfig->ant1comp  <= MAX_ANT_COMP) ) &&
            ( (pconfig->ant2comp  >= MIN_ANT_COMP) &&
            (pconfig->ant2comp  <= MAX_ANT_COMP) ) &&
            ( ((pconfig->txpower  >= MIN_TXPOWER) &&
            (pconfig->txpower  <= MAX_TXPOWER)) ||
            (pconfig->txpower  == FULL_TXPOWER) ) ) )
        {
            /* Copy the configuration parameters to the option field */
            pOpts.ch_pwr_data = htons((pconfig->channel << 10) | 
                (pconfig->txpower << 4)  | 
                (pconfig->datarate));
            pOpts.fragmentation_threshold = htons(pconfig->fragthreshold);
            pOpts.rts_cts_threshold = htons(pconfig->rtsctsthreshold);
            pOpts.service_mode = pconfig->servicemode;
            pOpts.unit_mode = pconfig->unitmode;

            pOpts.antenna  = pconfig->antenna;
            pOpts.ant1comp = pconfig->ant1comp;

            pOpts.ant2comp = pconfig->ant2comp;
            pOpts.bandwidth = pconfig->bandwidth;
            memset(pOpts.mac_address, 0, sizeof(pOpts.mac_address));
            pOpts.rsv = 0x00;
            pOpts.rsv1 = 0x00;
   
            /* Send the packet and without payload, 
            however, Mr Windows requires at least two bytes of payload
            give it to him, no harm */
            
            rvalue = send_WAVE_pkt((unsigned char *)"  ", 2, g_ipBroadcast, htonl(GetIPAddress()), (ipoptions_header *) &pOpts);
        }
    }
    return rvalue;
}


//=========================================================================================
//   FUNCTION:     STATUS_TYPE set_WRM_WAVEdefault (void) 
//
//   DESCRIPTION:  This function configures the WRM to the WAVE default parameters.
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE set_WRM_WAVEdefault (void)
{
    /* WAVE Default Option packet */
    wave_default_req pOpts;
    
    /* Set the option header */
    pOpts.ipopts.ipopt_cpy = 1;
    pOpts.ipopts.ipopt_class = 2;
    /* Option type according to the WAVE 
    Interface specifications */
    pOpts.ipopts.ipopt_option = WAVE_DEFAULT_REQ;
    pOpts.ipopts.ipopt_len = sizeof(struct wave_default_req);
    pOpts.rsv = 0x0;
    
    /* Send the packet and without payload, 
    however, Mr Windows requires at least two bytes of payload
    give it to him, no harm */
    return send_WAVE_pkt((unsigned char *)"  ", 2, g_ipBroadcast, htonl(GetIPAddress()), (ipoptions_header *) &pOpts);
}

//=========================================================================================
//   FUNCTION:     STATUS_TYPE send_WRM_configuration_req (void) 
//
//   DESCRIPTION:  This function sends configuration request to the WRM.
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE send_WRM_configuration_req (void)
{
   
    /* WAVE configuration request packet */
    wave_cfg_req pOpts;
    
    config_pending = false;
    /* Set the option header */
    pOpts.ipopts.ipopt_cpy = 1;
    pOpts.ipopts.ipopt_class = 2;
    /* Option type according to the WAVE 
    Interface specifications */
    pOpts.ipopts.ipopt_option = WAVE_CFG_REQ;
    pOpts.ipopts.ipopt_len = sizeof(struct wave_cfg_req);
    pOpts.rsv = 0x0;
    
    /* Send the packet and without payload, 
    however, Windows requires at least two bytes of payload
    give it to him, no harm */
    return send_WAVE_pkt((unsigned char *)"  ", 2, g_ipBroadcast, htonl(GetIPAddress()), (ipoptions_header *) &pOpts);
}



//=========================================================================================
//   FUNCTION:     get_WRM_configuration (wrm_configuration_parameters_type *pconfig) 
//
//   DESCRIPTION:  This function gets WAVE configuration parameters from the WRM using
//                 WAVE cfg Options field.
//
//   OUTPUT        wrm_configuration_parameters_type: WRM configuration data structure
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE get_WRM_configuration (wrm_configuration_parameters_type *pconfig, in_addr_t & ipWRM)
{
    printf("**** In get_WRM_configuration\n");
    STATUS_TYPE rvalue = FAILURE;

    int timeout = TIMOUT;
    //Linux timer in millisecond granularity
    struct timespec to;
    to.tv_sec = 0;
    to.tv_nsec = TIMOUT_TIME*1000000; //10 millisecond
    
    if (pconfig)
    {

        /* Send WRM Configuration request */
        send_WRM_configuration_req ();
        /* Wait for the response back */
        while (timeout-- >= 0)
        {
        /* Check if there is a configuration packet
            waiting for the application */

            if (config_pending == true)
            {
         
                /* Only one thread can modify current_config_option at one time */
//                pthread_mutex_lock(&configuration_pkt_section);  //Linux
                g_configuration_pkt_mutex.lock();
                /* Retrieve the packet from the temporary
                storage */
                *pconfig = current_config_option;
                /* Clear the pending flag */
                config_pending = false;        
                /* Allow other threads to access the current_config_option */
//                pthread_mutex_unlock(&configuration_pkt_section);    //Linux
                g_configuration_pkt_mutex.unlock();
                ipWRM = g_ipWRM;
                /* return success */
                rvalue = SUCCESS;
                //break;
                return rvalue;
            }

            //else Sleep (TIMOUT_TIME); //WINDOW
            else nanosleep(&to,NULL); //Linux
        }

    }
  
    return rvalue;
}


//=========================================================================================
//   FUNCTION:     OpenWincapDriver (char ip_addr[]) 
//
//   DESCRIPTION:  This function opens WinCap handler to the interface card
//
//   OUTPUT        fp: Pointer to the interface card
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
STATUS_TYPE OpenWinpcapDriver (const QString & strDeviceName)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    /* Open the device */
    if ( (Txconn = Rxconn = pcap_open_live(strDeviceName,
        MAX_PACKET_SIZE /*snaplen*/,
        //PCAP_OPENFLAG_PROMISCUOUS flags,
        1 /*flags*/,
        20 /*read timeout*/,
//        NULL /* remote authentication */,
        errbuf)
        ) == NULL)
    {
        perror("Error opening adapter");
//        cout << "Error opening Adapter\n";
        return FAILURE;
    } else {
		if (pcap_setnonblock(Rxconn, 1, errbuf) != 0)
			return FAILURE;
    }
    return SUCCESS;
    
}



//=========================================================================================
//   FUNCTION:     void Init_PCAP (void)
//
//   DESCRIPTION:  This function initializes the PCAP Library
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
WRM_CONNECTION_STATUS Init_PCAP (in_addr_t ipSrc, in_addr_t ipSubnet, const QString & strDeviceName)
{
//	STATUS_TYPE  rvalue = SUCCESS;
//	char     name[1024];
	//PHOSTENT hostinfo;        //WINDOW
//	hostent *hostinfo;
	struct in_addr ip;
//	int sfd;
	
//	if(0 > (sfd = socket(AF_INET, SOCK_STREAM, 0))) {
//		perror("socket()");
//		exit(1);
//	}
	
	/* build the broadcast IP address */
	uint32_t host_addr = htonl(ipSrc);           // local IP addr
	uint32_t net_mask = htonl(ipSubnet);
	uint32_t net_addr = host_addr & net_mask;         // 192.168.1.0
	ip.s_addr = (net_addr | (~net_mask));             //Linux
	g_ipBroadcast = ip.s_addr;

	// initialize the winpcap driver
	OpenWinpcapDriver(strDeviceName);

	// Now intialize the receiving thread and monitor the socket for incoming packets
	start_receive_thread();
	
	ConnectionStatus = CONNECTED;
	return ConnectionStatus;
}

//=========================================================================================
//   FUNCTION:     CONNECTION_TYPE CALLBACK get_Connection_status(void)
//
//   DESCRIPTION:  This function retrieves the WRM connection Status
//
//   RETURN        CONNECTED if the function is successful
//                 DISCONNECTED if the function is unsuccessful
//=========================================================================================
WRM_CONNECTION_STATUS get_Connection_status (void)
{
    return ConnectionStatus;
}

//=========================================================================================
//   FUNCTION:     GetAdapterNameFromIp(byte a1, byte a2, byte a3, byte a4, PCHAR Name)
//
//   DESCRIPTION:  This function Retrieves the device name used from the Adapter list
//
//   OUTPUT        Name of the adapter for the designated IP address
//
//   RETURN        SUCCESS if the function is successful
//                 FAILURE if the function is unsuccessful
//=========================================================================================
/*STATUS_TYPE GetAdapterNameFromIp(PCHAR Name)
{
    DWORD       i = 0;
    char*        Address;
    BYTE        *Address2;
    struct ifi_info *ifi, *ifihead;
    struct sockaddr        *Addr2;
    Address = src_ip;
    //Linux Code
    for (ifihead = ifi=Get_ifi_info(AF_INET,0);ifi!=NULL;ifi=ifi->ifi_next){
      Addr2 = ifi->ifi_addr;
      Address2 = (unsigned char *)MySock_ntop_host(Addr2,sizeof(*Addr2));
      if(memcmp(Address2,Address,4)==0)
      {
        strncpy(Name,ifi->ifi_name,1024);
        return SUCCESS;
      }
    }
    return FAILURE;
}

      
char* MySock_ntop_host(const struct sockaddr *sa,socklen_t salen)
{
  static char str[128];
  switch (sa->sa_family) {
    case AF_INET: {
      struct sockaddr_in *sin = (struct sockaddr_in*)sa;
      if (inet_ntop(AF_INET, &sin->sin_addr, str ,sizeof(str)) == NULL)
      {

        perror("sock_ntop_host error");
        return NULL;
      }
      return(str);
    }

    default:
      return(str);
  }
   return (NULL);   
}*/
      
