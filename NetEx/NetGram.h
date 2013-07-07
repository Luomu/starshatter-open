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
