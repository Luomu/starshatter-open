/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetGram.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Generic Network Packet (Datagram) Implementation
*/


#include "MemDebug.h"
#include "NetGram.h"
#include "NetLayer.h"

// +-------------------------------------------------------------------+

static DWORD net_gram_sequence = 1;

// +-------------------------------------------------------------------+

/**
 * NetGram constructor for ACK packets
 */
NetGram::NetGram()
   : retries(0), packet_id(0), send_time(0)
{ }

/**
 * NetGram constructor for receiving packets from remote hosts
 */
NetGram::NetGram(const NetAddr& src, Text msg)
   : addr(src), retries(0), send_time(0)
{
   body = msg;

   if (body.length() >= NET_GRAM_HEADER_SIZE) {
      BYTE* data = (BYTE*) body.data();

      packet_id = (((DWORD) data[0]) << 24) +
                  (((DWORD) data[1]) << 16) +
                  (((DWORD) data[2]) <<  8) +
                   ((DWORD) data[3]);
   }
}

/**
 * NetGram constructor for composing packets to send to remote hosts
 */
NetGram::NetGram(const NetAddr& dst, Text user_data, int r)
   : addr(dst), retries(r)
{
   send_time = NetLayer::GetTime();
   packet_id = net_gram_sequence++;

   if (retries)
      packet_id |= NET_GRAM_RELIABLE;

   static BYTE buf[NET_GRAM_MAX_SIZE];
   buf[0] = (BYTE) (packet_id >> 24) & 0xff;
   buf[1] = (BYTE) (packet_id >> 16) & 0xff;
   buf[2] = (BYTE) (packet_id >>  8) & 0xff;
   buf[3] = (BYTE) (packet_id)       & 0xff;

   int len = user_data.length();
   if (len >= NET_GRAM_MAX_SIZE - NET_GRAM_HEADER_SIZE)
      len = NET_GRAM_MAX_SIZE - NET_GRAM_HEADER_SIZE - 1;

   CopyMemory(buf+NET_GRAM_HEADER_SIZE, user_data.data(), len);

   body = Text((char*) buf, len+NET_GRAM_HEADER_SIZE);
}

// +--------------------------------------------------------------------+

void
NetGram::Retry()
{
   if (retries > 0) {
      retries--;
      send_time = NetLayer::GetTime();
   }
}

// +--------------------------------------------------------------------+

NetGram
NetGram::Ack()
{
   NetGram ack;

   ack.packet_id  = packet_id | NET_GRAM_ACK;
   ack.send_time  = NetLayer::GetTime();

   static BYTE buf[NET_GRAM_HEADER_SIZE];
   buf[0] = (BYTE) (ack.packet_id >> 24) & 0xff;
   buf[1] = (BYTE) (ack.packet_id >> 16) & 0xff;
   buf[2] = (BYTE) (ack.packet_id >>  8) & 0xff;
   buf[3] = (BYTE) (ack.packet_id)       & 0xff;

   ack.body = Text((char*) buf, NET_GRAM_HEADER_SIZE);

   return ack;
}



