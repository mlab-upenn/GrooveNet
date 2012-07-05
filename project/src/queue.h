#ifndef _Queue_h
#define _Queue_h

//======================================================================
//             QUEUE.H
//
// DESCRIPTION: This file contains the Queue data structure API
//
// AUTHOR: DENSO INTERNATIONAL AMERICA, INC.
//         LA Laboratories
//         Bassam Masri
//
// Modified: April 30 2005
//
// VERSION: This is a preliminary version being provided to the VSCC
//          for information purposes.  DENSO reserves the right to 
//          make changes without prior approval from the VSCC.
//
//======================================================================
#include <qmutex.h>
#define MAX_QUEUE_SIZE   1000

/* maximum size of a packet */
#define MAX_PACKET_SIZE  1530

//OLD WINDOW CODE
//extern CRITICAL_SECTION rx_pkt_queue_section;
//extern pthread_mutex_t rx_pkt_queue_section; //LINUX
extern QMutex g_pkt_queue_mutex;
/* storage for received Packet Message */
typedef struct {
    int  len;
    char Data[MAX_PACKET_SIZE];
} QueueMessage;

typedef QueueMessage ElementType;

        struct QueueRecord;
typedef struct QueueRecord *Queue;

//======================================================
// Create Queue - Pass number of elements
//======================================================
Queue CreateQueue (int MaxElements);

//======================================================
// Check if the Queue is empty
//======================================================
int IsEmpty (Queue Q);

//======================================================
// Check if the Queue is Full
//======================================================
int IsFull (Queue Q);

//======================================================
// Free Queue Memory
//======================================================
void DisposeQueue (Queue Q);

//======================================================
// Empty the Queue
//======================================================
void MakeEmpty (Queue Q);

//======================================================
// Add element to the Queue
//======================================================
void Enqueue (ElementType X, Queue Q);

//======================================================
// Remove the first element in the Queue
//======================================================
void Dequeue (Queue Q);

//======================================================
// Retrieve the first element in the Queue without removing it
//======================================================
ElementType Front (Queue Q);

//======================================================
// Retrieve the first element and remove it from the queue
//======================================================
ElementType FrontAndDequeue (Queue Q);

#endif  /* _Queue_h */
