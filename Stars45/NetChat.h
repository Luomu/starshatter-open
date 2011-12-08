/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NetChat.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Single chat message and sender
*/


#ifndef NetChat_h
#define NetChat_h

#include "Types.h"
#include "NetUser.h"

// +-------------------------------------------------------------------+

class NetChatEntry
{
public:
   static const char* TYPENAME() { return "NetChatEntry"; }

   NetChatEntry(const NetUser* user, const char* msg);
   NetChatEntry(int id, const char* user, const char* msg);
   ~NetChatEntry();

   int operator == (const NetChatEntry& c) const { return id == c.id; }
   int operator <  (const NetChatEntry& c) const { return id <  c.id; }

   int            GetID()        const { return id;         }
   const Text&    GetUser()      const { return user;       }
   Color          GetColor()     const { return color;      }
   const Text&    GetMessage()   const { return msg;        }
   DWORD          GetTime()      const { return time;       }

private:
   int            id;
   Text           user;
   Text           msg;
   Color          color;
   DWORD          time;
};

// +-------------------------------------------------------------------+

#endif NetChat_h