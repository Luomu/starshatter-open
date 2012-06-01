/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetMsg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    User level network message
*/


#include "MemDebug.h"
#include <windows.h>
#include "NetMsg.h"

// +-------------------------------------------------------------------+

static DWORD net_msg_sequence = 1;

// +-------------------------------------------------------------------+

NetMsg::NetMsg(DWORD nid, void* d, int l, BYTE f) 
   : msgid(net_msg_sequence++), netid(nid), len(l), flags(f)
{
   data = new(__FILE__,__LINE__) BYTE[len];

   if (data) {
      CopyMemory(data, d, len);

      if (len < MAX_SIZE)
         data[1] = len;
      else
         data[1] = 0;
   }
   else {
      len = 0;
   }
}

// +-------------------------------------------------------------------+

NetMsg::NetMsg(DWORD nid, BYTE type, const char* text, int l, BYTE f) 
   : msgid(net_msg_sequence++), netid(nid), len(2+l), flags(f)
{
   data = new(__FILE__,__LINE__) BYTE[len];

   if (data) {
      data[0] = type;

      if (len < MAX_SIZE)
         data[1] = len;
      else
         data[1] = 0;

      if (len > 2)
         CopyMemory(data+2, text, len-2);
   }
   else {
      len = 0;
   }
}

// +-------------------------------------------------------------------+

NetMsg::~NetMsg()
{
   delete [] data;
}

// +-------------------------------------------------------------------+

int NetMsg::operator < (const NetMsg& m) const
{
   if (data[0] == MULTIPART && m.data[0] == MULTIPART) {
      NetMsgMultipart* p1 = (NetMsgMultipart*) data;
      NetMsgMultipart* p2 = (NetMsgMultipart*) m.data;

      if (p1->msgid == p2->msgid)
         return p1->partno < p2->partno;

      return p1->msgid < p2->msgid;
   }

   return msgid <  m.msgid;
}
