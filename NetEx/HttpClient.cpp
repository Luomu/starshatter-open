/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         HttpClient.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Server Pump for HTTP Server
*/


#include "MemDebug.h"
#include "HttpClient.h"
#include "NetHost.h"
#include "NetLayer.h"
#include <mmsystem.h>

// +-------------------------------------------------------------------+

HttpClient::HttpClient(const NetAddr& server_addr)
   : NetClient(server_addr)
{
}

HttpClient::~HttpClient()
{
   cookies.destroy();
}

HttpResponse*
HttpClient::DoRequest(HttpRequest& request)
{
   // add existing cookies to request before sending:
   CombineCookies(request.GetCookies(), cookies);

   Text req = request.operator Text();
   Text msg = SendRecv(req);
   HttpResponse*  response = new(__FILE__,__LINE__) HttpResponse(msg);

   if (response) {
      // save cookies returned in response:
      CombineCookies(cookies, response->GetCookies());
   }

   return response;
}

void
HttpClient::CombineCookies(List<HttpParam>& dst, List<HttpParam>& src)
{
   for (int i = 0; i < src.size(); i++) {
      HttpParam* s = src[i];
      HttpParam* d = dst.find(s);

      if (d) {
         d->value = s->value;
      }
      else {
         HttpParam* cookie = new(__FILE__,__LINE__) HttpParam(s->name, s->value);
         if (cookie)
            dst.append(cookie);
      }
   }
}