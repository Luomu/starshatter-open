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
    FILE:         NetAuth.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    This class authenticates a user connecting to the multiplayer lobby
*/


#ifndef NetAuth_h
#define NetAuth_h

#include "Types.h"
#include "NetAddr.h"
#include "NetLobby.h"
#include "Text.h"

// +-------------------------------------------------------------------+

class NetAuth
{
public:
    enum AUTH_STATE {
        NET_AUTH_INITIAL  = 0,
        NET_AUTH_FAILED   = 1,
        NET_AUTH_OK       = 2
    };

    enum AUTH_LEVEL {
        NET_AUTH_MINIMAL  = 0,
        NET_AUTH_STANDARD = 1,
        NET_AUTH_SECURE   = 2
    };

    static int     AuthLevel();
    static void    SetAuthLevel(int n);

    static Text    CreateAuthRequest(NetUser* u);
    static Text    CreateAuthResponse(int level, const char* salt);
    static bool    AuthUser(NetUser* u, Text response);
};

// +-------------------------------------------------------------------+

#endif NetAuth_h