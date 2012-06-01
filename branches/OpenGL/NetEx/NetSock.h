/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetSock.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network (IP) Socket
*/

#ifndef NetSock_h
#define NetSock_h

#include <windows.h>
#include "NetAddr.h"
#include "Text.h"

// +-------------------------------------------------------------------+

#define NET_MAX_TIMEOUT 1e9

class NetSock
{
public:
   static const char* TYPENAME() { return "NetSock"; }

   enum SELECT_TYPE {
      SELECT_READ  = 1,
      SELECT_WRITE = 2
   };

   NetSock(bool stream=false);
   NetSock(const NetAddr& addr, bool stream=false);
   ~NetSock();

   int      bind(const NetAddr& addr);
   int      connect(const NetAddr& addr);
   int      listen(int max_connections=5);
   NetSock* accept(NetAddr* addr=0);
   int      available();
   int      send(Text msg);
   Text     recv();
   int      sendto(Text msg, const NetAddr& dest);
   Text     recvfrom(NetAddr* a=0);
   int      select(SELECT_TYPE t=SELECT_READ,
                   long seconds=0, long microseconds=0);

   int      shutdown_input();
   int      shutdown_output();
   int      set_timeout(int msecs);
   int      close();

   DWORD    max_packet_size() const;
   bool     is_stream()       const { return stream; }
   bool     is_closed()       const { return closed; }
   int      status()          const { return stat;   }

private:
   NetSock(SOCKET s, bool stream);

   SOCKET   s;
   bool     stream;
   bool     closed;
   int      stat;
   int      current_timeout;
};


#endif NetSock_h