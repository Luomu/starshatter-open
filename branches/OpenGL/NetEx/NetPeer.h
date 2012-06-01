/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetPeer.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    One side of a UDP net link connection
*/


#ifndef NetPeer_h
#define NetPeer_h

#include <windows.h>
#include "NetAddr.h"
#include "List.h"
#include "ThreadSync.h"

// +-------------------------------------------------------------------+

class NetGram;
class NetMsg;

// +-------------------------------------------------------------------+

class NetPeer
{
public:
   static const char* TYPENAME() { return "NetPeer"; }

   enum STATUS { OK, SEND_OVERFLOW, RECV_OVERFLOW };

   NetPeer(const NetAddr& addr, DWORD id);
   ~NetPeer();

   int operator == (const NetPeer& p)  const { return netid == p.netid; }

   bool              SendMessage(NetMsg* msg);
   NetMsg*           GetMessage();

   NetGram*          ComposeGram();
   bool              ReceiveGram(NetGram* g, List<NetMsg>* q=0);

   const NetAddr&    Address()         const { return addr;       }
   DWORD             NetID()           const { return netid;      }
   DWORD             Sequence()        const { return sequence;   }

   int               GetMaxPPS()       const { return pps;        }
   void              SetMaxPPS(int p)        { pps = p;           }
   int               GetMaxBPS()       const { return bps;        }
   void              SetMaxBPS(int b)        { bps = b;           }
   int               GetMaxQSize()     const { return max_qsize;  }
   void              SetMaxQSize(int q)      { max_qsize = q;     }

   DWORD             GetChunkSize()    const { return chunk_size; }
   void              SetChunkSize(DWORD s)   { chunk_size = s;    }

   DWORD             LastReceiveTime() const { return last_recv_time; }
   void              SetLastReceiveTime(DWORD t) { last_recv_time = t; }

private:
   bool              OKtoSend()        const;
   void              CheckMultiRecv(List<NetMsg>* q);

   NetAddr           addr;          // remote network address
   DWORD             sequence;      // highest packet id received
   DWORD             netid;         // unique id for this peer
   int               pps;           // max packets per second
   int               bps;           // max bits per second
   int               max_qsize;     // max bytes in either queue
   int               status;        // ok or error code
   DWORD             chunk_size;    // size of multipart message chunk

   enum HIST { HIST_SIZE=8 };

   DWORD             last_recv_time;         // time of last received packet        
   DWORD             hist_time[HIST_SIZE];   // history for pps check
   DWORD             hist_size[HIST_SIZE];   // history for bps check
   int               hist_indx;              // index into history

   int               send_size;     // total bytes in send list
   int               recv_size;     // total bytes in recv list
   List<NetMsg>      send_list;     // queue of messages waiting to be sent
   List<NetMsg>      recv_list;     // queue of messages waiting to be read

   List<NetMsg>      multi_send_list;
   List<NetMsg>      multi_recv_list;

   ThreadSync        sync;
};


#endif NetPeer_h