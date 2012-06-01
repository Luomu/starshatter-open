/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         HttpServletExec.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Server Pump for HTTP Server
*/


#ifndef HttpServletExec_h
#define HttpServletExec_h

#include "HttpServer.h"

// +-------------------------------------------------------------------+

class HttpServlet;
class HttpSession;

// +-------------------------------------------------------------------+

class HttpServletExec : public HttpServer
{
public:
   static const char* TYPENAME() { return "HttpServletExec"; }

   HttpServletExec(WORD port, int poolsize=1);
   virtual ~HttpServletExec();

   int operator == (const HttpServletExec& l) const { return addr == l.addr; }

   virtual bool         DoGet(HttpRequest& request,  HttpResponse& response);

   virtual HttpServlet* GetServlet(HttpRequest& request);
   virtual HttpSession* GetSession(HttpRequest& request);

   virtual DWORD        CheckSessions();

   virtual int          GetSessionTimeout()  const { return session_timeout; }
   virtual void         SetSessionTimeout(int t)   { session_timeout = t;    }

protected:
   virtual void         DoSyncedCheck();

   List<HttpSession>    sessions;
   int                  session_timeout;
   HANDLE               hsession;
   bool                 exec_shutdown;
};

#endif HttpServletExec_h