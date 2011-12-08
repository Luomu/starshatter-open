/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetServer.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Server Pump for HTTP Server
*/


#ifndef NetServer_h
#define NetServer_h

#include <windows.h>
#include "NetAddr.h"
#include "NetGram.h"
#include "NetSock.h"
#include "List.h"

// +-------------------------------------------------------------------+

class NetServer
{
public:
   static const char* TYPENAME() { return "NetServer"; }

   NetServer(WORD port, int poolsize=1);
   virtual ~NetServer();

   int operator == (const NetServer& l) const { return addr == l.addr; }

   virtual void      Shutdown();
   virtual DWORD     Listener();
   virtual DWORD     Reader(int index);

   virtual Text      ProcessRequest(Text request, const NetAddr& addr);
   virtual Text      DefaultResponse();
   virtual Text      ErrorResponse();

   const NetAddr&    GetAddress() const { return addr; }
   int               GetLastError() const { return err; }

protected:
   NetAddr           addr;
   NetSock           sock;
   NetAddr           client_addr;

   int               poolsize;
   HANDLE            hreader;
   HANDLE*           pool;
   NetSock**         conn;
   NetAddr*          clients;
   int               err;
   bool              server_shutdown;

   ThreadSync        sync;
};


#endif NetServer_h