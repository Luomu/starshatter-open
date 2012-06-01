/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetMsg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    User level network message
*/


#ifndef NetMsg_h
#define NetMsg_h

#include <windows.h>
#include "NetAddr.h"
#include "NetGram.h"
#include "NetSock.h"
#include "List.h"

// +-------------------------------------------------------------------+

class NetMsg
{
public:
   static const char* TYPENAME() { return "NetMsg"; }

   enum FLAGS { RELIABLE = 0x01, PRIORITY = 0x02, SCATTER = 0x04 };
   enum TYPES { INVALID    = 0,
                RESERVED   = 0xF0,
                MULTIPART  = 0xF1
   };
   enum       { MAX_SIZE   = 250 };

   NetMsg(DWORD nid, void* d, int l, BYTE f=0);
   NetMsg(DWORD nid, BYTE type, const char* text, int len, BYTE f=0);
   ~NetMsg();

   int operator == (const NetMsg& m) const { return msgid == m.msgid &&
                                                    netid == m.netid;   }
   int operator <  (const NetMsg& m) const;

   DWORD       Sequence()        const { return msgid;            }
   DWORD       NetID()           const { return netid;            }
   const BYTE* Data()            const { return data;             }
   BYTE        Type()            const { return data ? *data : 0; }
   int         Length()          const { return len;              }
   BYTE        Flags()           const { return flags;            }

   bool        IsReliable()      const { return flags & RELIABLE ? true : false; }
   bool        IsPriority()      const { return flags & PRIORITY ? true : false; }
   bool        IsScatter()       const { return flags & SCATTER  ? true : false; }

   void        SetSequence(DWORD s)    { msgid = s;               }

private:
   DWORD msgid;
   DWORD netid;
   BYTE* data;
   int   len;
   BYTE  flags;
};

// +-------------------------------------------------------------------+

struct NetMsgMultipart {
   BYTE  type;
   BYTE  len;
   DWORD msgid;
   DWORD partno;
   DWORD nparts;
   BYTE  payload[256];
};

// +-------------------------------------------------------------------+

#endif NetMsg_h