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
     FILE:         NetMsg.cpp
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     User level network message
*/


#include "MemDebug.h"
#include <windows.h>
#include "NetMsg.h"

// +-------------------------------------------------------------------+

static DWORD net_msg_sequence = 1;

// +-------------------------------------------------------------------+

NetMsg::NetMsg(DWORD nid, void* d, int l, BYTE f) 
    : msgid(net_msg_sequence++), netid(nid), len(l), flags(f)
{
    data = new(__FILE__,__LINE__) BYTE[len];

    if (data) {
        CopyMemory(data, d, len);

        if (len < MAX_SIZE)
            data[1] = len;
        else
            data[1] = 0;
    }
    else {
        len = 0;
    }
}

// +-------------------------------------------------------------------+

NetMsg::NetMsg(DWORD nid, BYTE type, const char* text, int l, BYTE f) 
    : msgid(net_msg_sequence++), netid(nid), len(2+l), flags(f)
{
    data = new(__FILE__,__LINE__) BYTE[len];

    if (data) {
        data[0] = type;

        if (len < MAX_SIZE)
            data[1] = len;
        else
            data[1] = 0;

        if (len > 2)
            CopyMemory(data+2, text, len-2);
    }
    else {
        len = 0;
    }
}

// +-------------------------------------------------------------------+

NetMsg::~NetMsg()
{
    delete [] data;
}

// +-------------------------------------------------------------------+

int NetMsg::operator < (const NetMsg& m) const
{
    if (data[0] == MULTIPART && m.data[0] == MULTIPART) {
        NetMsgMultipart* p1 = (NetMsgMultipart*) data;
        NetMsgMultipart* p2 = (NetMsgMultipart*) m.data;

        if (p1->msgid == p2->msgid)
            return p1->partno < p2->partno;

        return p1->msgid < p2->msgid;
    }

    return msgid <  m.msgid;
}
