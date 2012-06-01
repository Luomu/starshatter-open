/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetGram.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Datagram (UDP) packet that implements the basic
    packet-oriented network protocol.
*/


#ifndef NetGram_h
#define NetGram_h

#include <windows.h>
#include "NetAddr.h"
#include "Text.h"

// +-------------------------------------------------------------------+

const int NET_GRAM_ACK           = 0x80000000;
const int NET_GRAM_RELIABLE      = 0x40000000;
const int NET_GRAM_SEQ_MASK      = 0x3fffffff;

const int NET_GRAM_HEADER_SIZE   = 4;
const int NET_GRAM_MAX_SIZE      = 1024;

// +-------------------------------------------------------------------+

class NetGram
{
public:
   static const char* TYPENAME() { return "NetGram"; }

   // for receiving packets from remote hosts:
   NetGram(const NetAddr& src, Text msg);

   // for composing packets to send to remote hosts:
   NetGram(const NetAddr& dst, Text user_data, int retries);

   int operator == (const NetGram& g) const { return packet_id == g.packet_id &&
                                                     addr      == g.addr;        }
   int operator <  (const NetGram& g) const { return Sequence() < g.Sequence();  }

   DWORD          PacketID()  const { return packet_id;                                   }
   DWORD          Sequence()  const { return packet_id & NET_GRAM_SEQ_MASK;               }
   DWORD          SendTime()  const { return send_time;                                   }
   BYTE*          Data()      const { return (BYTE*) body.data();                         }
   BYTE*          UserData()  const { return (BYTE*) body.data() + NET_GRAM_HEADER_SIZE;  }
   int            Size()      const { return body.length();                               }
   const Text&    Body()      const { return body;                                        }
   const NetAddr& Address()   const { return addr;                                        }

   bool        IsAck()        const { return packet_id & NET_GRAM_ACK      ? true : false; }
   bool        IsReliable()   const { return packet_id & NET_GRAM_RELIABLE ? true : false; }
   int         Retries()      const { return retries; }

   void        Retry();
   NetGram     Ack();
   void        ClearAck()           { packet_id &= ~NET_GRAM_ACK;                         }

protected:
   NetGram();

   NetAddr  addr;       // network address of remote host
   int      retries;    // number of retries remaining (reliable packets only)
   DWORD    send_time;  // time in msec of most recent send attempt

   DWORD    packet_id;  // copy of packet id from header in body
   Text     body;       // header plus user data
};


#endif NetGram_h