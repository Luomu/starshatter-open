/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetClient.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Stream-oriented network client class
*/


#ifndef NetClient_h
#define NetClient_h

#include <windows.h>
#include "NetAddr.h"
#include "NetGram.h"
#include "NetSock.h"
#include "List.h"

// +-------------------------------------------------------------------+

class NetClient
{
public:
   static const char* TYPENAME() { return "NetClient"; }

   NetClient(const NetAddr& server_addr);
   virtual ~NetClient();

   int operator == (const NetClient& c)   const { return this == &c; }

   NetAddr           GetServerAddr()      const { return addr;       }

   bool              Send(Text msg);
   Text              Recv();
   Text              SendRecv(Text msg);

   int               GetLastError()       const { return err;        }
   DWORD             GetTime()            const { return delta;      }

protected:
   NetAddr           addr;
   NetSock*          sock;
   DWORD             delta;
   DWORD             time;
   int               err;

public:
   enum ERRS {
      ERR_INTR                = 10004,
      ERR_BADF                = 10009,
      ERR_ACCES               = 10013,
      ERR_FAULT               = 10014,
      ERR_INVAL               = 10022,
      ERR_MFILE               = 10024,

      ERR_WOULDBLOCK          = 10035,
      ERR_INPROGRESS          = 10036,
      ERR_ALREADY             = 10037,
      ERR_NOTSOCK             = 10038,
      ERR_DESTADDRREQ         = 10039,
      ERR_MSGSIZE             = 10040,
      ERR_PROTOTYPE           = 10041,
      ERR_NOPROTOOPT          = 10042,
      ERR_PROTONOSUPPORT      = 10043,
      ERR_SOCKTNOSUPPORT      = 10044,
      ERR_OPNOTSUPP           = 10045,
      ERR_PFNOSUPPORT         = 10046,
      ERR_AFNOSUPPORT         = 10047,
      ERR_ADDRINUSE           = 10048,
      ERR_ADDRNOTAVAIL        = 10049,
      ERR_NETDOWN             = 10050,
      ERR_NETUNREACH          = 10051,
      ERR_NETRESET            = 10052,
      ERR_CONNABORTED         = 10053,
      ERR_CONNRESET           = 10054,
      ERR_NOBUFS              = 10055,
      ERR_ISCONN              = 10056,
      ERR_NOTCONN             = 10057,
      ERR_SHUTDOWN            = 10058,
      ERR_TOOMANYREFS         = 10059,
      ERR_TIMEDOUT            = 10060,
      ERR_CONNREFUSED         = 10061,
      ERR_LOOP                = 10062,
      ERR_NAMETOOLONG         = 10063,
      ERR_HOSTDOWN            = 10064,
      ERR_HOSTUNREACH         = 10065,
      ERR_NOTEMPTY            = 10066,
      ERR_PROCLIM             = 10067,
      ERR_USERS               = 10068,
      ERR_DQUOT               = 10069,
      ERR_STALE               = 10070,
      ERR_REMOTE              = 10071
   };
};


#endif NetClient_h