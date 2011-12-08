/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         HttpServletExec.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Server Pump for HTTP Server
*/


#include "MemDebug.h"
#include "HttpServletExec.h"
#include "HttpServlet.h"
#include "NetLayer.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// +-------------------------------------------------------------------+

class HttpTestServlet : public HttpServlet
{
public:
   HttpTestServlet()          { }
   virtual ~HttpTestServlet() { }

   virtual bool DoGet(HttpRequest& request,  HttpResponse& response)
   {
      char buffer[1024];
      Text content;

      content  = "<html><head><title>HttpTestServlet</title></head>\n";
      content += "<body bgcolor=\"#c0c0c0\" text=\"black\">\n<h1>HttpTestServlet</h1>\n";

      content += "<br><h3>HttpSessionId:</h3><p>\n";
      if (session)
         content += session->GetID();
      else
         content += "No Session Found";
      content += "<br>\n";

      content += "<br><h3>URI Requested:</h3><p>\n";
      content += request.URI();
      content += "<br>\n";

      if (request.GetQuery().size() > 0) {
         content += "<br><h3>Query Parameters:</h3>\n";

         ListIter<HttpParam> q_iter = request.GetQuery();
         while (++q_iter) {
            HttpParam* q = q_iter.value();
            sprintf(buffer, "<b>%s:</b> <i>%s</i><br>\n", q->name.data(), q->value.data());
            content += buffer;
         }
      }

      content += "<br><h3>Request Headers:</h3>\n";
      ListIter<HttpParam> h_iter = request.GetHeaders();
      while (++h_iter) {
         HttpParam* h = h_iter.value();
         sprintf(buffer, "<b>%s:</b> <i>%s</i><br>\n", h->name.data(), h->value.data());
         content += buffer;
      }

      if (request.GetCookies().size() > 0) {
         content += "<br><h3>Cookies:</h3>\n";
         ListIter<HttpParam> c_iter = request.GetCookies();
         while (++c_iter) {
            HttpParam* c = c_iter.value();
            sprintf(buffer, "<b>%s:</b> <i>%s</i><br>\n", c->name.data(), c->value.data());
            content += buffer;
         }
      }

      content += "</body></html>\n\n";

      response.SetStatus(HttpResponse::SC_OK);
      response.AddHeader("MIME-Version",  "1.0");
      response.AddHeader("Content-Type",  "text/html");
      response.SetContent(content);

      return true;
   }
};

// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

DWORD WINAPI HttpServletExecSessionProc(LPVOID link);

HttpServletExec::HttpServletExec(WORD port, int poolsize)
   : HttpServer(port, poolsize), session_timeout(60), exec_shutdown(false)
{
   http_server_name = "Generic HttpServletExec 1.0";

   DWORD thread_id = 0;
   hsession = CreateThread(0, 4096, HttpServletExecSessionProc,
                           (LPVOID) this, 0, &thread_id);
}

HttpServletExec::~HttpServletExec()
{
   if (!exec_shutdown)
      exec_shutdown = true;

   WaitForSingleObject(hsession, 1000);
   CloseHandle(hsession);
   hsession = 0;

   sessions.destroy();
}

// +--------------------------------------------------------------------+

HttpServlet*
HttpServletExec::GetServlet(HttpRequest& request)
{
   return new(__FILE__,__LINE__) HttpTestServlet;
}

// +--------------------------------------------------------------------+

HttpSession*
HttpServletExec::GetSession(HttpRequest& request)
{
   HttpSession* session = 0;
   Text         reqID   = request.GetCookie("SessionID");

   if (reqID.length() > 0) {
      ListIter<HttpSession> iter = sessions;
      while (++iter && !session) {
         HttpSession* s = iter.value();

         if (s->GetID() == reqID) {
            session = s;
            session->Access();
         }
      }
   }

   if (!session) {
      session = new(__FILE__,__LINE__) HttpSession;
      if (session) {
         sessions.append(session);

         ::Print("HttpServletExec created new session '%s' for request '%s'\n",
            (const char*) session->GetID(),
            (const char*) request.RequestLine());
      }
      else {
         ::Print("HttpServletExec out of memory for request '%s'\n",
            (const char*) request.RequestLine());
      }
   }

   return session;
}

// +--------------------------------------------------------------------+

bool
HttpServletExec::DoGet(HttpRequest& request, HttpResponse& response)
{
   bool         result  = false;
   HttpSession* session = GetSession(request);
   HttpServlet* servlet = GetServlet(request);

   if (servlet) {
      servlet->SetSession(session);
      result = servlet->Service(request, response);
      delete servlet;
   }

   if (result) {
      response.SetHeader("Server", http_server_name);

      if (session)
         response.SetCookie("SessionID", session->GetID());
   }

   return result;
}

// +--------------------------------------------------------------------+

DWORD WINAPI HttpServletExecSessionProc(LPVOID link)
{
   HttpServletExec* exec = (HttpServletExec*) link;

   if (exec)
      return exec->CheckSessions();

   return (DWORD) E_POINTER;
}

DWORD
HttpServletExec::CheckSessions()
{
   while (!exec_shutdown) {
      sync.acquire();

      if (sessions.size()) {
         ListIter<HttpSession> iter = sessions;
         while (++iter) {
            HttpSession* s = iter.value();

            if (NetLayer::GetUTC() - s->GetLastAccess() > session_timeout) {
               ::Print("HttpServletExec deleting expired session '%s'\n", (const char*) s->GetID());
               delete iter.removeItem();
            }
         }
      }

      DoSyncedCheck();

      sync.release();
      Sleep(100);
   }

   return 0;
}

void
HttpServletExec::DoSyncedCheck()
{
}