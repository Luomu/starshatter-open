/*  Starshatter OpenSource Distribution
     Copyright (c) 1997-2004, Destroyer Studios LLC.
     All Rights Reserved.

     Redistribution and use in source and binary forms, with or without
     modification, are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
     * Neither the name "Destroyer Studios" nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
     AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
     ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
     LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
     CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
     SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
     INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
     ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
     POSSIBILITY OF SUCH DAMAGE.

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
                sprintf_s(buffer, "<b>%s:</b> <i>%s</i><br>\n", q->name.data(), q->value.data());
                content += buffer;
            }
        }

        content += "<br><h3>Request Headers:</h3>\n";
        ListIter<HttpParam> h_iter = request.GetHeaders();
        while (++h_iter) {
            HttpParam* h = h_iter.value();
            sprintf_s(buffer, "<b>%s:</b> <i>%s</i><br>\n", h->name.data(), h->value.data());
            content += buffer;
        }

        if (request.GetCookies().size() > 0) {
            content += "<br><h3>Cookies:</h3>\n";
            ListIter<HttpParam> c_iter = request.GetCookies();
            while (++c_iter) {
                HttpParam* c = c_iter.value();
                sprintf_s(buffer, "<b>%s:</b> <i>%s</i><br>\n", c->name.data(), c->value.data());
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
    hsession = CreateThread(0, 4096, HttpServletExecSessionProc, (LPVOID) this, 0, &thread_id);
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
