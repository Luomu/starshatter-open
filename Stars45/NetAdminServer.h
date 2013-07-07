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

    SUBSYSTEM:    Stars.exe
    FILE:         NetAdminServer.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    HTTP Servlet Engine for Multiplayer Admin
*/


#ifndef NetAdminServer_h
#define NetAdminServer_h

#include "HttpServletExec.h"
#include "HttpServlet.h"

// +-------------------------------------------------------------------+

class Mission;
class MissionElement;
class NetChatEntry;
class NetUser;

// +-------------------------------------------------------------------+

class NetAdminServer : public HttpServletExec
{
public:
    virtual ~NetAdminServer();

    int operator == (const NetAdminServer& s) const { return this == &s; }

    virtual HttpServlet*    GetServlet(HttpRequest& request);

    virtual void            AddUser(NetUser* user);
    virtual void            DelUser(NetUser* user);
    virtual int             NumUsers();
    virtual bool            HasHost();
    virtual List<NetUser>&  GetUsers();

    virtual NetUser*        FindUserBySession(Text id);

    virtual void            AddChat(NetUser* user, const char* msg);
    ListIter<NetChatEntry>  GetChat();
    DWORD                   GetStartTime() const { return start_time; }

    virtual void            GameOn()    { }
    virtual void            GameOff()   { }

    // singleton locator:
    static NetAdminServer* GetInstance(WORD port=0);

protected:
    NetAdminServer(WORD port);
    virtual void            DoSyncedCheck();

    DWORD                   start_time;
    List<NetUser>           admin_users;
};

// +-------------------------------------------------------------------+

class NetAdminServlet : public HttpServlet
{
public:
    NetAdminServlet();
    virtual ~NetAdminServlet()  { }

    virtual bool      DoGet(HttpRequest& request, HttpResponse& response);
    virtual bool      CheckUser(HttpRequest& request, HttpResponse& response);

    virtual Text      GetCSS();
    virtual Text      GetHead(const char* title=0);
    virtual Text      GetBody();
    virtual Text      GetTitleBar(const char* statline=0, const char* onload=0);
    virtual Text      GetStatLine();
    virtual Text      GetCopyright();
    virtual Text      GetContent();
    virtual Text      GetBodyClose();

protected:
    NetAdminServer*   admin;
    NetUser*          user;
};

#endif NetAdminServer_h