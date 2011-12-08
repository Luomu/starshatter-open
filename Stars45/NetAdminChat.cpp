/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NetAdminServer.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    HTTP Servlet Engine for Multiplayer Admin
*/


#include "MemDebug.h"
#include "NetAdminChat.h"
#include "NetLobbyServer.h"
#include "NetServerConfig.h"
#include "NetUser.h"
#include "NetChat.h"
#include "NetUtil.h"

#include "HttpServlet.h"
#include "NetLayer.h"
#include "FormatUtil.h"

// +-------------------------------------------------------------------+

NetAdminChat::NetAdminChat()
{ }

// +-------------------------------------------------------------------+

bool
NetAdminChat::DoGet(HttpRequest& request, HttpResponse& response)
{
   if (CheckUser(request, response)) {
      NetLobbyServer* lobby = NetLobbyServer::GetInstance();

      if (lobby) {
         Text msg = request.GetParam("msg");
         Text act = request.GetParam("action");

         if (msg.length()) {
            lobby->AddChat(user, msg);

            if (user)
               NetUtil::SendChat(0xffff, user->Name(), msg);
         }
         
         else if (act.length()) {
            if (act == "clear")
               lobby->ClearChat();

            else if (act == "save")
               lobby->SaveChat();
         }
      }

      response.SetStatus(HttpResponse::SC_OK);
      response.AddHeader("MIME-Version",  "1.0");
      response.AddHeader("Content-Type",  "text/html");
      response.AddHeader("Cache-Control", "no-cache");
      response.AddHeader("Expires",       "-1");

      response.SetContent(GetHead("Chat") +
                          GetTitleBar(GetStatLine(),
                                      "onLoad=\"self.focus();document.chatForm.msg.focus();\"") +
                          GetContent() +
                          GetBodyClose());
   }

   return true;
}

// +-------------------------------------------------------------------+

Text
NetAdminChat::GetContent()
{
   Text content = "<div style=\"overflow-y:scroll; height:240px; padding-right:4pt; padding-left:4pt; padding-bottom:4pt; padding-top:4pt; margin:4pt;\">\n";

   int nchat = 0;
   NetLobbyServer* lobby = NetLobbyServer::GetInstance();
   if (lobby) {
      content += "\n<table width=\"90%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n";

      ListIter<NetChatEntry> iter = lobby->GetChat();
      while (++iter) {
         NetChatEntry* c = iter.value();

         content += "  <tr><td nowrap width=\"130\" class=\"tiny\">";
         content += FormatTimeString(c->GetTime());
         content += "</td><td nowrap width=\"80\" class=\"tiny\">";
         content += c->GetUser();
         content += "</td><td class=\"tiny\">";
         content += c->GetMessage();
         content += "</td></tr>\n";
      }

      content += "</table>\n\n";
   }

   content += "</div>\n<div class=\"content\">\n\
  <form name=\"chatForm\" method=\"post\"action=\"/chat\">\n\
  <table border=\"0\">\n\
    <tr>\n\
     <td valign=\"middle\">&nbsp;&nbsp;<input type=\"text\" name=\"msg\" size=\"80\"></td>\n\
     <td valign=\"middle\">&nbsp;&nbsp;<input type=\"submit\" value=\"Send\"></td>\n\
    </tr>\n\
    <tr>\n\
     <td colspan=\"2\" valign=\"middle\" class=\"std\">&nbsp;&nbsp;<a href=\"/chat\">Refresh</a>\
&nbsp;&nbsp;&#183;&nbsp;&nbsp;<a href=\"/chat?action=save\">Save</a>&nbsp;&nbsp;&#183;&nbsp;&nbsp;<a href=\"/chat?action=clear\">Clear</a></td>\n\
    </tr>\n\
  </table>\n\
  </form>\n\
</div>\n\n";

   content += GetCopyright();
   return content;
}
