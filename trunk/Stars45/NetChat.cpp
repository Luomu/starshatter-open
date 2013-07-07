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

    SUBSYSTEM:    Stars.exe
    FILE:         NetChat.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Single chat message and sender
*/


#include "MemDebug.h"
#include "NetChat.h"
#include "NetLayer.h"

// +-------------------------------------------------------------------+

static int chat_id_key = 1000;

// +-------------------------------------------------------------------+

NetChatEntry::NetChatEntry(const NetUser* u, const char* s)
: id(chat_id_key++), msg(s)
{
    if (u) {
        user = u->Name();
        color = u->GetColor();
    }
    else {
        user = "unknown";
        color = Color::Gray;
    }

    time = NetLayer::GetUTC();
}

NetChatEntry::NetChatEntry(int msg_id, const char* u, const char* s)
: id(msg_id), user(u), msg(s)
{
    color = Color::Gray;
    time = NetLayer::GetUTC();

    if (id >= chat_id_key)
    chat_id_key = id + 1;
}

NetChatEntry::~NetChatEntry()
{ }

