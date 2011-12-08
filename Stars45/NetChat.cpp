/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

