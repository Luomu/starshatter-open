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
     FILE:         HttpServlet.h
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Network Server Pump for HTTP Server
*/


#ifndef HttpServlet_h
#define HttpServlet_h

#include "HttpServer.h"

// +-------------------------------------------------------------------+

class HttpServlet;
class HttpSession;

// +-------------------------------------------------------------------+

class HttpServlet
{
public:
    static const char* TYPENAME() { return "HttpServlet"; }

    HttpServlet();
    virtual ~HttpServlet();

    virtual bool      Service(HttpRequest& request, HttpResponse& response);

    virtual bool      DoGet(HttpRequest& request,  HttpResponse& response);
    virtual bool      DoPost(HttpRequest& request, HttpResponse& response);
    virtual bool      DoHead(HttpRequest& request, HttpResponse& response);

    virtual HttpSession* GetSession()               { return session; }
    virtual void         SetSession(HttpSession* s) { session = s; }

protected:
    HttpSession*      session;
};

// +-------------------------------------------------------------------+

class HttpSession
{
public:
    static const char* TYPENAME() { return "HttpSession"; }

    HttpSession();
    virtual ~HttpSession();

    int operator == (const HttpSession& s) const { return id == s.id; }

    Text              GenerateUniqueID();

    Text              GetID()              const { return id;         }
    void              SetID(const char* i)       { id = i;            }
    int               GetLastAccess()      const { return access_time;}
    void              Access();

    List<HttpParam>&  GetAttributes()            { return attributes; }

    Text              GetAttribute(const char* name);
    void              SetAttribute(const char* name, const char* value);
    void              DelAttribute(const char* name);

    int               GetIntAttribute(const char* name);
    void              SetIntAttribute(const char* name, int value);
    void              DelIntAttribute(const char* name);

protected:
    Text              id;
    int               access_time;
    List<HttpParam>   attributes;
};


#endif HttpServlet_h