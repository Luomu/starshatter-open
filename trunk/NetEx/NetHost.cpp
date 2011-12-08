/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetHost.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Host
*/


#include "MemDebug.h"
#include "NetHost.h"
#include "NetLayer.h"
#include <ctype.h>

NetHost::NetHost()
{
   char host_name[256];
   gethostname(host_name, sizeof(host_name));

   Init(host_name);
}

NetHost::NetHost(const char* host_name)
{
   Init(host_name);
}

void NetHost::Init(const char* host_name)
{
   if (host_name && *host_name) {
      HOSTENT* h = 0;

      if (isdigit(*host_name)) {
         DWORD addr = inet_addr(host_name);
         h = gethostbyaddr((const char*) &addr, 4, AF_INET);   
      }
      else {
         h = gethostbyname(host_name);
      }

      if (h) {
         name = h->h_name;

         char** alias = h->h_aliases;
         while (*alias) {
            aliases.append(new Text(*alias));
            alias++;
         }

         char** addr = h->h_addr_list;
         while (*addr) {
            NetAddr* pna = new(__FILE__,__LINE__) NetAddr(**(DWORD**) addr);
            if (pna)
               addresses.append(pna);
            addr++;
         }
      }
   }
}

NetHost::NetHost(const NetHost& n)
{
   if (&n != this) {
      NetHost& nh = (NetHost&) n;

      name = nh.name;

      ListIter<Text> alias = nh.aliases;
      while (++alias)
         aliases.append(new Text(*alias.value()));

      ListIter<NetAddr> addr = nh.addresses;
      while (++addr)
         addresses.append(new NetAddr(*addr.value()));
   }
}

NetHost::~NetHost()
{
   aliases.destroy();
   addresses.destroy();
}

const char*
NetHost::Name()
{
   return name;
}

NetAddr
NetHost::Address()
{
   if (addresses.size())
      return *(addresses[0]);

   return NetAddr((DWORD) 0);
}