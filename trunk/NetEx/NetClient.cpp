/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetClient.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Server Pump for HTTP Server
*/


#include "MemDebug.h"
#include "NetClient.h"
#include "NetHost.h"
#include "NetLayer.h"
#include <mmsystem.h>

// +-------------------------------------------------------------------+

NetClient::NetClient(const NetAddr& server_addr)
   : addr(server_addr), sock(0), delta(0), time(0), err(0)
{
}

NetClient::~NetClient()
{
   delete sock;
}

// +--------------------------------------------------------------------+

bool
NetClient::Send(Text msg)
{
   if (msg.length() > 0) {
      if (sock)
         delete sock;

      sock  = new(__FILE__,__LINE__) NetSock(addr, true);
      delta = 0;
      time  = timeGetTime();

      if (!sock) {
         err = ERR_NOBUFS;
         return false;
      }

      err = sock->send(msg);
      if (err < 0) {
         err = NetLayer::GetLastError();
         return false;
      }

      err = sock->shutdown_output();
      if (err < 0) {
         err = NetLayer::GetLastError();
         return false;
      }

      return true;
   }

   else {
      delete sock;
      sock = 0;
   }

   return false;
}

Text
NetClient::Recv()
{
   Text response;

   if (sock) {
      int ready = sock->select();

      while (!ready && timeGetTime() - time < 2000) {
         Sleep(5);
         ready = sock->select();
      }

      if (ready) {
         Text msg = sock->recv();

         while (msg.length() > 0) {
            response += msg;
            msg = sock->recv();
         }

         delta = timeGetTime() - time;
      }

      delete sock;
      sock = 0;
   }

   return response;
}

Text
NetClient::SendRecv(Text msg)
{
   Text response;

   if (msg.length() > 0 && Send(msg)) {
      response = Recv();
   }

   return response;
}
