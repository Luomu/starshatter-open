/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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