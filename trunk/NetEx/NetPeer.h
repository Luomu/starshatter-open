/*  Starshatter OpenSource Distribution
     Copyright (c) 1997-2004, Destroyer Studios LLC.
     All Rights Reserved.

     Redistribution and use in source and binary forms, with or without
     modification, are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
     * Neither the name "Destroyer Studios" nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
     AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
     ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
     LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
     CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
     SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
     INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
     ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
     POSSIBILITY OF SUCH DAMAGE.

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