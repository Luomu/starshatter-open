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
    FILE:         NetAdminServer.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    HTTP Servlet Engine for Multiplayer Admin
*/


#include "MemDebug.h"
#include "NetAdminServer.h"
#include "NetLobbyServer.h"
#include "NetServerConfig.h"
#include "NetClientConfig.h"
#include "NetAdminChat.h"
#include "NetUser.h"
#include "NetChat.h"

#include "StarServer.h"
#include "HttpServlet.h"
#include "NetLayer.h"

#include "DataLoader.h"
#include "FormatUtil.h"
#include "MachineInfo.h"

extern const char* versionInfo;

// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

class NetAdminLogin : public NetAdminServlet
{
public:
    NetAdminLogin()           { }
    virtual ~NetAdminLogin()  { }

    virtual bool DoGet(HttpRequest& request, HttpResponse& response) {
        NetServerConfig* config = NetServerConfig::GetInstance();

        Text admin_name = "system";
        Text admin_pass = "manager";

        if (config) {
            admin_name = config->GetAdminName();
            admin_pass = config->GetAdminPass();
        }

        Text name  = request.GetParam("user");
        Text pass  = request.GetParam("pass");

        Sleep(500);

        if (CheckUser(request, response)) {
            response.SetStatus(HttpResponse::SC_TEMPORARY_REDIRECT);
            response.SetHeader("MIME-Version",  "1.0");
            response.SetHeader("Content-Type",  "text/html");
            response.SetHeader("Cache-Control", "no-cache");
            response.SetHeader("Expires",       "-1");
            response.SetHeader("Location",      "/home");

            response.SetContent(GetHead("Login") +
            "<body><br>You are already logged in.<br>" +
            GetBodyClose());
        }

        else if (name == admin_name && pass == admin_pass) {
            user = new(__FILE__,__LINE__) NetUser(name);
            user->SetAddress(request.GetClientAddr());

            if (session)
            user->SetSessionID(session->GetID());

            admin->AddUser(user);

            response.SetStatus(HttpResponse::SC_TEMPORARY_REDIRECT);
            response.SetHeader("MIME-Version",  "1.0");
            response.SetHeader("Content-Type",  "text/html");
            response.SetHeader("Cache-Control", "no-cache");
            response.SetHeader("Expires",       "-1");
            response.SetHeader("Location",      "/home");

            response.SetContent(GetHead("Login") +
            "<body><br>You have successfully logged in.<br>" +
            GetBodyClose());
        }

        else {
            response.SetStatus(HttpResponse::SC_OK);
            response.SetHeader("MIME-Version",  "1.0");
            response.SetHeader("Content-Type",  "text/html");
            response.SetHeader("Cache-Control", "no-cache");
            response.SetHeader("Expires",       "-1");

            response.SetContent(GetHead("Login") +
            GetTitleBar(0, "onLoad=\"self.focus();document.loginForm.user.focus();\"") +
            GetContent()     +
            GetBodyClose());
        }

        return true;
    }

    virtual Text GetContent() { Text content =
        "  <table border=\"0\" cellspacing=\"0\" cellpadding=\"4\" align=\"left\" width =\"100%\">\n\
    <tr>\n\
    <td width=\"100\">&nbsp;&nbsp;</td>\n\
    <td valign=\"top\" align=\"left\" width=\"400\"><br><br>\n\
        <span class=\"subhead\">Welcome to the Starshatter Server!</span><br><br>\n\
        <span class=\"std\">Login to access the server <b>";

        NetServerConfig* config = NetServerConfig::GetInstance();
        if (config)
        content += config->Name();
        else
        content += "server";

        content += "</b></span><br>\n\
        <form name=\"loginForm\" method=\"get\" action=\"/login\">\n\
        <table border=\"0\" cellspacing=\"0\" cellpadding=\"4\" width=\"100\">\n\
            <tr>\n\
            <td align=\"left\" valign=\"middle\" width=\"80\"><span class=\"std\">Username:</span></td>\n\
            <td align=\"left\" valign=\"middle\"><input type=\"text\" name=\"user\" size=\"25\"></td>\n\
            </tr>\n\
            <tr>\n\
            <td align=\"left\" valign=\"middle\" width=\"80\"><span class=\"std\">Password:</span></td>\n\
            <td align=\"left\" valign=\"middle\"><input type=\"password\" name=\"pass\" size=\"25\"></td>\n\
            </tr>\n\
            <tr>\n\
            <td>&nbsp;</td>\n\
            <td align=\"right\"><input type=\"submit\" value=\"Login\"></td>\n\
            </tr>\n\
        </table>\n\
        </form>\n\
    <td>&nbsp;</td>\n\
    </tr>\n\
</table>\n";

        return content;
    }
};

// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

class NetAdminServerMgr : public NetAdminServlet
{
public:
    NetAdminServerMgr()           { }
    virtual ~NetAdminServerMgr()  { }

    virtual bool DoGet(HttpRequest& request, HttpResponse& response) {
        if (CheckUser(request, response)) {
            Text action = request.GetParam("action");
            action.setSensitive(false);

            bool completed = false;

            if (action == "restart") {
                StarServer* svr = StarServer::GetInstance();

                if (svr) {
                    svr->Shutdown(true);
                    completed = true;

                    response.SetStatus(HttpResponse::SC_OK);
                    response.SetHeader("MIME-Version",  "1.0");
                    response.SetHeader("Content-Type",  "text/html");
                    response.SetHeader("Cache-Control", "no-cache");
                    response.SetHeader("Expires",       "-1");

                    response.SetContent(GetHead("Restart") +
                    GetTitleBar() +
                    "<div class=\"content\"><b>The Starshatter Server will restart in three (3) seconds.</b><br></div>" +
                    GetBodyClose());
                }
            }

            else if (action == "shutdown") {
                StarServer* svr = StarServer::GetInstance();

                if (svr) {
                    svr->Shutdown(false);
                    completed = true;

                    response.SetStatus(HttpResponse::SC_OK);
                    response.SetHeader("MIME-Version",  "1.0");
                    response.SetHeader("Content-Type",  "text/html");
                    response.SetHeader("Cache-Control", "no-cache");
                    response.SetHeader("Expires",       "-1");

                    response.SetContent(GetHead("Restart") +
                    GetTitleBar() +
                    "<div class=\"content\"><b>The Starshatter Server will shutdown in three (3) seconds.</b><br></div>" +
                    GetBodyClose());
                }
            }

            if (!completed) {
                response.SetStatus(HttpResponse::SC_TEMPORARY_REDIRECT);
                response.SetHeader("MIME-Version",  "1.0");
                response.SetHeader("Content-Type",  "text/html");
                response.SetHeader("Cache-Control", "no-cache");
                response.SetHeader("Expires",       "-1");
                response.SetHeader("Location",      "/home");

                response.SetContent(GetHead("Login") +
                "<body><br>Unknown Action.<br>" +
                GetBodyClose());
            }
        }

        return true;
    }
};

// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

class NetAdminFile : public NetAdminServlet
{
public:
    NetAdminFile()           { }
    virtual ~NetAdminFile()  { }

    virtual bool DoGet(HttpRequest& request, HttpResponse& response) {
        if (!CheckUser(request, response))
        return true;

        Text content;
        Text path = request.GetParam("path");
        Text name = request.GetParam("name");

        if (name.length()) {
            BYTE*       buffer = 0;
            DataLoader* loader = DataLoader::GetLoader();

            if (loader) {
                bool use_file_system = loader->IsFileSystemEnabled();

                loader->UseFileSystem(true);
                loader->SetDataPath(path);
                int len = loader->LoadBuffer(name, buffer);

                if (len) {
                    content = Text((const char*) buffer, len);
                }

                loader->ReleaseBuffer(buffer);
                loader->SetDataPath(0);
                loader->UseFileSystem(use_file_system);
            }
        }

        response.SetStatus(HttpResponse::SC_OK);
        response.AddHeader("MIME-Version",  "1.0");
        response.AddHeader("Cache-Control", "no-cache");
        response.AddHeader("Expires",       "-1");
        response.AddHeader("Content-Type",  "text/plain");
        response.SetContent(content);

        return true;
    }
};

// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

class NetAdminUserList : public NetAdminServlet
{
public:
    NetAdminUserList()           { }
    virtual ~NetAdminUserList()  { }

    virtual bool DoGet(HttpRequest& request, HttpResponse& response) {
        if (CheckUser(request, response)) {
            response.SetStatus(HttpResponse::SC_OK);
            response.SetHeader("MIME-Version",  "1.0");
            response.SetHeader("Content-Type",  "text/html");
            response.SetHeader("Cache-Control", "no-cache");
            response.SetHeader("Expires",       "-1");

            response.SetContent(GetHead("User List") +
            GetTitleBar() +
            GetContent() +
            GetBodyClose());
        }

        return true;
    }

    virtual Text GetContent() {
        Text content =
        "<script LANGUAGE=\"JavaScript\">\n\
<!--\n\
function doConfirm() {\n\
return confirm(\"Are you sure you want to ban this player?\");\n\
}\n\
// -->\n\
</script>\n\
<div class=\"content\">\n\
<table border=\"0\"  width=\"95%\">\n\
    <tr class=\"heading\">\n\
    <td nowrap valign=\"middle\" align=\"left\">\n\
        <span class=\"heading\">&nbsp;User List</span>\n\
    </td>\n\
    </tr>\n\
</table>\n\n";

        content +=
        "  <table border=\"0\"  width=\"95%\" class=\"std\">\n\
    <tr>\n\
    <td nowrap width=\"1%\">&nbsp;</td>\n\
    <td nowrap width=\"20%\" valign=\"middle\" align=\"left\"><b>Name</b></td>\n\
    <td nowrap width=\"10%\" valign=\"middle\" align=\"left\"><b>Address</b></td>\n\
    <td nowrap width=\"10%\" valign=\"middle\" align=\"center\"><b>Is Host</b></td>\n\
    <td nowrap width=\"20%\" valign=\"middle\" align=\"left\"><b>Squadron</b></td>\n\
    <td nowrap width=\"20%\" valign=\"middle\" align=\"center\"><b>Stats</b></td>\n\
    <td nowrap width=\"19%\" valign=\"middle\" align=\"center\"><b>Ban</b></td>\n\
    <td></td>\n\
</tr>\n";

        NetLobbyServer* lobby = NetLobbyServer::GetInstance();

        if (lobby) {
            ListIter<NetUser> u_iter = lobby->GetUsers();
            while (++u_iter) {
                NetUser* u = u_iter.value();
                NetAddr  a = u->GetAddress();

                char addr_dotted[32];
                char addr_hex[16];
                char user_stats[16];

                sprintf_s(addr_dotted, "%d.%d.%d.%d", a.B1(), a.B2(), a.B3(), a.B4());
                sprintf_s(addr_hex,    "%08x",        a.IPAddr());
                sprintf_s(user_stats,  "%d / %d / %d", u->Missions(), u->Kills(), u->Losses());

                content += "<tr>\n<td nowrap width=\"1%\">&nbsp;</td>\n\
            <td nowrap valign=\"middle\" align=\"left\">";
                content += u->Name();
                content += "</td><td nowrap valign=\"middle\" align=\"left\">";
                content += addr_dotted;
                content += "</td><td nowrap valign=\"middle\" align=\"center\">";
                content += u->IsHost() ? "*" : "&nbsp;";
                content += "</td><td nowrap valign=\"middle\" align=\"left\">";
                content += u->Squadron();
                content += "</td><td nowrap valign=\"middle\" align=\"center\">";
                content += user_stats;
                content += "</td><td nowrap valign=\"middle\" align=\"center\">";
                content += "<a onclick=\"return doConfirm()\" href=\"/ban?name=";
                content += HttpRequest::EncodeParam(u->Name());
                content += "&addr=";
                content += addr_hex;
                content += "\">BAN</a></td></tr>\n";
            }
        }

        content += "  </table>\n\n";

        content += "</div>\n\n";
        content += GetCopyright();
        return content;
    }
};

// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

class NetAdminBanUser : public NetAdminServlet
{
public:
    NetAdminBanUser()           { }
    virtual ~NetAdminBanUser()  { }

    virtual bool DoGet(HttpRequest& request, HttpResponse& response) {
        if (CheckUser(request, response)) {
            Text name = request.GetParam("name");
            bool completed = false;

            NetLobbyServer* lobby = NetLobbyServer::GetInstance();

            if (lobby) {
                ListIter<NetUser> u_iter = lobby->GetUsers();
                while (++u_iter && !completed) {
                    NetUser* u = u_iter.value();

                    if (u->Name() == name) {
                        NetLobbyServer* nls = NetLobbyServer::GetInstance();

                        if (nls) {
                            nls->BanUser(u);
                            completed = true;
                        }
                    }
                }
            }

            response.SetStatus(HttpResponse::SC_TEMPORARY_REDIRECT);
            response.SetHeader("MIME-Version",  "1.0");
            response.SetHeader("Content-Type",  "text/html");
            response.SetHeader("Cache-Control", "no-cache");
            response.SetHeader("Expires",       "-1");
            response.SetHeader("Location",      "/users");

            response.SetContent(GetHead("User List") +
            GetTitleBar() +
            "<div class=\"content\">User Banned.<br></div>" +
            GetBodyClose());
        }

        return true;
    }
};

// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

static NetAdminServer*  net_Admin_server = 0;

NetAdminServer*
NetAdminServer::GetInstance(WORD port)
{
    if (!net_Admin_server && port > 0)
    net_Admin_server = new(__FILE__,__LINE__) NetAdminServer(port);

    return net_Admin_server;
}

NetAdminServer::NetAdminServer(WORD port)
: HttpServletExec(port)
{
    http_server_name = Text("Starshatter NetAdminServer ") + versionInfo;
}

NetAdminServer::~NetAdminServer()
{
    if (net_Admin_server == this)
    net_Admin_server = 0;
}

// +--------------------------------------------------------------------+

HttpServlet*
NetAdminServer::GetServlet(HttpRequest& request)
{
    Text path = request.URI();
    path.setSensitive(false);

    if (path.indexOf("/login") == 0)
    return new(__FILE__,__LINE__) NetAdminLogin;

    if (path.indexOf("/chat") == 0)
    return new(__FILE__,__LINE__) NetAdminChat;

    if (path.indexOf("/server") == 0)
    return new(__FILE__,__LINE__) NetAdminServerMgr;

    if (path.indexOf("/file") == 0)
    return new(__FILE__,__LINE__) NetAdminFile;

    if (path.indexOf("/user") == 0)
    return new(__FILE__,__LINE__) NetAdminUserList;

    if (path.indexOf("/ban") == 0)
    return new(__FILE__,__LINE__) NetAdminBanUser;

    return new(__FILE__,__LINE__) NetAdminServlet;
}

// +-------------------------------------------------------------------+

void
NetAdminServer::AddChat(NetUser* user, const char* msg)
{
    if (user && msg && *msg) {
        NetLobbyServer* lobby = NetLobbyServer::GetInstance();

        if (lobby)
        lobby->AddChat(user, msg);
    }
}

ListIter<NetChatEntry>
NetAdminServer::GetChat()
{
    NetLobbyServer* lobby = NetLobbyServer::GetInstance();

    if (lobby)
    return lobby->GetChat();

    static List<NetChatEntry> idle_chatter;
    return idle_chatter;
}

// +-------------------------------------------------------------------+

void
NetAdminServer::AddUser(NetUser* user)
{
    if (user && !admin_users.contains(user))
    admin_users.append(user);
}

void
NetAdminServer::DelUser(NetUser* user)
{
    if (user) {
        admin_users.remove(user);
        delete user;
    }
}

int
NetAdminServer::NumUsers()
{
    return admin_users.size();
}


List<NetUser>&
NetAdminServer::GetUsers()
{
    return admin_users;
}

bool
NetAdminServer::HasHost()
{
    bool result = false;

    NetLobbyServer* lobby = NetLobbyServer::GetInstance();

    if (lobby)
    result = lobby->HasHost();

    return result;
}

NetUser*
NetAdminServer::FindUserBySession(Text id)
{
    ListIter<NetUser> iter = admin_users;
    while (++iter) {
        NetUser* u = iter.value();
        if (u->GetSessionID() == id)
        return u;
    }

    return 0;
}

void
NetAdminServer::DoSyncedCheck()
{
    ListIter<NetUser> iter = admin_users;
    while (++iter) {
        NetUser* u = iter.value();

        bool found = false;

        ListIter<HttpSession> s_iter = sessions;
        while (++s_iter && !found) {
            HttpSession* s = s_iter.value();

            if (s->GetID() == u->GetSessionID())
            found = true;
        }

        if (!found)
        delete iter.removeItem();
    }
}


// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

NetAdminServlet::NetAdminServlet()
{
    admin = NetAdminServer::GetInstance();
    user  = 0;
}

// +-------------------------------------------------------------------+

bool
NetAdminServlet::DoGet(HttpRequest& request, HttpResponse& response)
{
    if (CheckUser(request, response)) {

        if (request.URI() == "/home")
        response.SetStatus(HttpResponse::SC_OK);
        else
        response.SetStatus(HttpResponse::SC_TEMPORARY_REDIRECT);

        response.SetHeader("MIME-Version",  "1.0");
        response.SetHeader("Content-Type",  "text/html");
        response.SetHeader("Cache-Control", "no-cache");
        response.SetHeader("Expires",       "-1");
        response.SetHeader("Location",      "/home");

        response.SetContent(GetHead() +
        GetTitleBar(GetStatLine()) +
        GetContent() +
        GetBodyClose());
    }

    return true;
}

// +-------------------------------------------------------------------+

bool
NetAdminServlet::CheckUser(HttpRequest& request, HttpResponse& response)
{
    if (!user) {
        if (session)
        user = admin->FindUserBySession(session->GetID());

        if (!user) {
            response.SetStatus(HttpResponse::SC_TEMPORARY_REDIRECT);
            response.SetHeader("MIME-Version",  "1.0");
            response.SetHeader("Content-Type",  "text/plain");
            response.SetHeader("Cache-Control", "no-cache");
            response.SetHeader("Expires",       "-1");
            response.SetHeader("Location",      "/login");
            response.SetContent("You are not logged in.");
        }
    }

    return user != 0;
}

// +-------------------------------------------------------------------+

Text
NetAdminServlet::GetCSS()
{
    return

    "body      { font-family:arial,helvetica,sans-serif; color:black; background-color:white }\n\
a:link    { text-decoration:none; font-weight:normal; font-size:10pt; color:black }\n\
a:visited { text-decoration:none; font-weight:normal; font-size:10pt; color:black }\n\
a:hover   { text-decoration:underline; font-weight:normal; font-size:10pt; color:black }\n\
.std      { font-size:10pt }\n\
.tiny     { font-size:8pt }\n\
.heading  { font-size:14pt; font-weight:bold; background-color:#99BBEE }\n\
.subhead  { font-size:11pt; font-weight:bold }\n\
.status   { font-size:10pt; color:white }\n\
.content  { padding-right: 4pt; padding-left: 4pt; padding-bottom: 4pt; padding-top: 4pt; margin: 4pt; }\n\
.copy     { font-size:8pt; }\n\
.top-bar     { color: white;  background-color: #336699 }\n\
.top-line    { color: yellow; background-color: black }\n\
.topbarbig   { line-height:24px; color:white; font-size:18px; font-weight:bold; }\n\
.topbarsmall { line-height:18px; color:white; font-size:14px; }\n";
}

Text
NetAdminServlet::GetHead(const char* title)
{
    Text head = "<html>\n<head>\n<title>Starshatter Server";

    if (title && *title) {
        head += " - ";
        head += title;
    }

    head += "</title>\n<style type=\"text/css\" media=\"screen\">\n";
    head += GetCSS();
    head += "</style>\n</head>\n";

    return head;
}

Text
NetAdminServlet::GetBody()
{
    return GetTitleBar(GetStatLine()) +
    GetContent()  +
    GetBodyClose();
}

Text
NetAdminServlet::GetTitleBar(const char* statline, const char* onload)
{
    Text bar = "<body ";

    if (onload && *onload)
    bar += onload;

    bar += " leftmargin=\"0\" topmargin=\"0\" marginwidth=\"0\" marginheight=\"0\">\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" width=\"100%\" class=\"top-bar\">\n\
    <tr height=\"50\">\n\
    <td>&nbsp;</td>\n\
    <td valign=\"middle\" align=\"left\">\n\
        <span class=\"topbarsmall\">Administration Console</span><br>\n";

    if (statline) {
        bar += "<a href=\"/home\">";
    }

    bar += "<span class=\"topbarbig\">Starshatter Server ";
    bar += versionInfo;
    bar += "</span>";

    if (statline) {
        bar += "</a>";
    }

    bar += "\n\
    </td>\n\
    </tr>\n\
    <tr class=\"top-line\">\n\
    <td colspan=\"2\">";

    if (statline && *statline)
    bar += statline;
    else
    bar += "&nbsp;";

    bar += "</td>\n\
    </tr>\n\
</table>\n\n";

    return bar;
}

Text
NetAdminServlet::GetStatLine()
{
    NetServerConfig* config = NetServerConfig::GetInstance();

    Text line =
    "      <table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" border=\"0\">\n\
        <tr>\n\
        <td nowrap width=\"33%\" class=\"top-line\" align=\"left\">\n\
            <span class=\"status\">&nbsp;&nbsp;Connected to <b>";

    char buffer[256];
    sprintf_s(buffer, "%s:%d", config->Name().data(), config->GetAdminPort());
    line += buffer;

    line += "</b></span>\n\
        </td>\n\
        <td nowrap width=\"34%\" class=\"top-line\" align=\"center\">\n\
            <span class=\"status\">Server Mode: <b>";

    NetLobbyServer* lobby = NetLobbyServer::GetInstance();
    if (lobby) {
        switch (lobby->GetStatus()) {
        default:
        case NetServerInfo::OFFLINE:     line += "Offline";      break;
        case NetServerInfo::LOBBY:       line += "Lobby";        break;
        case NetServerInfo::BRIEFING:    line += "Briefing";     break;
        case NetServerInfo::ACTIVE:      line += "Active";       break;
        case NetServerInfo::DEBRIEFING:  line += "Debriefing";   break;
        case NetServerInfo::PERSISTENT:  line += "PERSISTENT";   break;
        }
    }
    else {
        line += "Unknown";
    }

    line += "</b></span>\n\
        </td>\n\
        <td nowrap width=\"33%\" class=\"top-line\" align=\"right\">\n\
            <span class=\"status\">";

    line += FormatTimeString();
    
    line += "&nbsp;&nbsp;</span>\n\
        </td>\n\
        </tr>\n\
    </table>\n";

    return line;
}

Text
NetAdminServlet::GetContent()
{
    Text content =
    "<script LANGUAGE=\"JavaScript\">\n\
<!--\n\
function doConfirm() {\n\
return confirm(\"Are you sure you want to do this?\");\n\
}\n\
// -->\n\
</script>\n\
<div class=\"content\">\n\
<table border=\"0\"  width=\"95%\">\n\
    <tr class=\"heading\">\n\
    <td nowrap valign=\"middle\" align=\"left\">\n\
        <span class=\"heading\">&nbsp;Game Admin Functions</span>\n\
    </td>\n\
    </tr>\n\
</table>\n\n\
<table border=\"0\" width=\"95%\">\n\
    <tr>\n\
    <td nowrap width=\"1%\">&nbsp;</td>\n\
    <td nowrap width=\"33%\" valign=\"middle\" align=\"left\">\n\
        <a href=\"/chat\">Lobby Chat</a>\n\
    </td>\n\
    <td nowrap width=\"33%\" valign=\"middle\" align=\"left\">\n\
        <a href=\"/home\">Mission List</a>\n\
    </td>\n\
    <td></td>\n\
    </tr>\n\
    <tr>\n\
    <td nowrap width=\"1%\">&nbsp;</td>\n\
    <td nowrap width=\"33%\" valign=\"middle\" align=\"left\">\n\
        <a href=\"/file?name=errlog.txt\">View Error Log</a>\n\
    </td>\n\
    <td nowrap width=\"33%\" valign=\"middle\" align=\"left\">\n\
        <a href=\"/users\">Player List</a>\n\
    </td>\n\
    <td></td>\n\
    </tr>\n\
    <tr>\n\
    <td nowrap width=\"1%\">&nbsp;</td>\n\
    <td nowrap width=\"33%\" valign=\"middle\" align=\"left\">\n\
        <a href=\"/file?name=serverlog.txt\">View Server Log</a>\n\
    </td>\n\
    <td nowrap width=\"33%\" valign=\"middle\" align=\"left\">\n\
        <a href=\"/home\">Ban List</a>\n\
    </td>\n\
    <td></td>\n\
    </tr>\n\
    <tr>\n\
    <td nowrap width=\"1%\">&nbsp;</td>\n\
    <td nowrap width=\"33%\" valign=\"middle\" align=\"left\"></td>\n\
    <td nowrap width=\"33%\" valign=\"middle\" align=\"left\"></td>\n\
    <td></td>\n\
</tr>\n\
</table>\n\n";

    content +=
    "  <table border=\"0\"  width=\"95%\">\n\
    <tr class=\"heading\">\n\
    <td nowrap valign=\"middle\" align=\"left\">\n\
        <span class=\"heading\">&nbsp;Server Admin Functions</span>\n\
    </td>\n\
    </tr>\n\
</table>\n\n\
<table border=\"0\" width=\"95%\">\n\
    <tr>\n\
    <td nowrap width=\"1%\">&nbsp;</td>\n\
    <td nowrap width=\"33%\" valign=\"middle\" align=\"left\">\n\
        <a onclick=\"return doConfirm()\" href=\"/server?action=restart\">Restart Server</a>\n\
    </td>\n\
    <td nowrap width=\"33%\" valign=\"middle\" align=\"left\">\n\
        <a onclick=\"return doConfirm()\" href=\"/server?action=shutdown\">Shutdown Server</a>\n\
    </td>\n\
    <td></td>\n\
</tr>\n\
</table>\n\n";

    content += "</div>\n\n";
    content += GetCopyright();
    return content;
}

Text
NetAdminServlet::GetBodyClose()
{
    return "\n\n</body>\n</html>\n";
}

Text
NetAdminServlet::GetCopyright()
{
    return "<br><span class=\"copy\">&nbsp;&nbsp;&nbsp;&nbsp;Copyright &copy; 1997-2004 Destroyer Studios.  All rights reserved.</span><br>";
}
