/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NetBrokerClient.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Client for Starshatter.com GameNet Broker
*/


#ifndef NetBrokerClient_h
#define NetBrokerClient_h

#include "HttpClient.h"
#include "NetLobby.h"

// +-------------------------------------------------------------------+

class NetBrokerClient
{
public:
   static void Enable()  { broker_available = true;  }
   static void Disable() { broker_available = false; }

   static bool GameOn(const char* name,
                      const char* type,
                      const char* addr,
                      WORD        port,
                      const char* password);
   static bool GameList(const char* type, List<NetServerInfo>& server_list);

protected:
   static bool broker_available;
   static bool broker_found;
};


#endif NetBrokerClient_h