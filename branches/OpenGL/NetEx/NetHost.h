/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetHost.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Host
*/

#ifndef NET_HOST_H
#define NET_HOST_H

#include <windows.h>
#include "NetAddr.h"
#include "Text.h"
#include "List.h"

// +-------------------------------------------------------------------+

class NetHost
{
public:
   static const char* TYPENAME() { return "NetHost"; }

   NetHost();
   NetHost(const char* host_addr);
   NetHost(const NetHost& n);
   ~NetHost();

   const char*    Name();
   NetAddr        Address();

   List<Text>&    Aliases()      { return aliases; }
   List<NetAddr>& AddressList()  { return addresses; }

private:
   void Init(const char* host_name);

   Text           name;
   List<Text>     aliases;
   List<NetAddr>  addresses;
};


#endif // NET_HOST_H