/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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
