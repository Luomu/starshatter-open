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

    SUBSYSTEM:    Stars
    FILE:         NetClientConfig.h
    AUTHOR:       John DiCamillo

*/

#ifndef NetClientConfig_h
#define NetClientConfig_h

#include "Types.h"
#include "Game.h"
#include "Text.h"
#include "List.h"

#include "NetAddr.h"

// +--------------------------------------------------------------------+

class NetLobbyClient;
class NetServerInfo;

// +--------------------------------------------------------------------+

class NetClientConfig
{
public:
    NetClientConfig();
    ~NetClientConfig();

    void                    AddServer(const char* name,
    const char* addr,
    WORD        port,
    const char* password,
    bool        save=false);
    void                    DelServer(int index);

    List<NetServerInfo>&    GetServerList()   { return servers;       }
    NetServerInfo*          GetServerInfo(int n);
    void                    Download();
    void                    Load();
    void                    Save();

    void                    SetServerIndex(int n)   { server_index = n;    }
    int                     GetServerIndex()  const { return server_index; }
    void                    SetHostRequest(bool n)  { host_request = n;    }
    bool                    GetHostRequest()  const { return host_request; }
    NetServerInfo*          GetSelectedServer();

    void                    CreateConnection();
    NetLobbyClient*         GetConnection();
    bool                    Login();
    bool                    Logout();
    void                    DropConnection();

    static void             Initialize();
    static void             Close();
    static NetClientConfig* GetInstance()     { return instance;      }

private:
    List<NetServerInfo>     servers;
    int                     server_index;
    bool                    host_request;

    NetLobbyClient*         conn;

    static NetClientConfig* instance;
};

#endif NetClientConfig_h
