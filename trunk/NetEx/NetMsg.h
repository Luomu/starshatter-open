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
     FILE:         NetMsg.h
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     User level network message
*/


#ifndef NetMsg_h
#define NetMsg_h

#include <windows.h>
#include "NetAddr.h"
#include "NetGram.h"
#include "NetSock.h"
#include "List.h"

// +-------------------------------------------------------------------+

class NetMsg
{
public:
    static const char* TYPENAME() { return "NetMsg"; }

    enum FLAGS { RELIABLE = 0x01, PRIORITY = 0x02, SCATTER = 0x04 };
    enum TYPES { INVALID    = 0,
                     RESERVED   = 0xF0,
                     MULTIPART  = 0xF1
    };
    enum       { MAX_SIZE   = 250 };

    NetMsg(DWORD nid, void* d, int l, BYTE f=0);
    NetMsg(DWORD nid, BYTE type, const char* text, int len, BYTE f=0);
    ~NetMsg();

    int operator == (const NetMsg& m) const { return msgid == m.msgid &&
                                                     netid == m.netid;   }
    int operator <  (const NetMsg& m) const;

    DWORD       Sequence()        const { return msgid;            }
    DWORD       NetID()           const { return netid;            }
    const BYTE* Data()            const { return data;             }
    BYTE        Type()            const { return data ? *data : 0; }
    int         Length()          const { return len;              }
    BYTE        Flags()           const { return flags;            }

    bool        IsReliable()      const { return flags & RELIABLE ? true : false; }
    bool        IsPriority()      const { return flags & PRIORITY ? true : false; }
    bool        IsScatter()       const { return flags & SCATTER  ? true : false; }

    void        SetSequence(DWORD s)    { msgid = s;               }

private:
    DWORD msgid;
    DWORD netid;
    BYTE* data;
    int   len;
    BYTE  flags;
};

// +-------------------------------------------------------------------+

struct NetMsgMultipart {
    BYTE  type;
    BYTE  len;
    DWORD msgid;
    DWORD partno;
    DWORD nparts;
    BYTE  payload[256];
};

// +-------------------------------------------------------------------+

#endif NetMsg_h
