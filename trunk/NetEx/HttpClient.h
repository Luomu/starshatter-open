/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         HttpClient.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    HTTP/1.1 client class
*/


#ifndef HttpClient_h
#define HttpClient_h

#include "NetClient.h"
#include "HttpServer.h"

// +-------------------------------------------------------------------+

class HttpClient : public NetClient
{
public:
   static const char* TYPENAME() { return "HttpClient"; }

   HttpClient(const NetAddr& server_addr);
   virtual ~HttpClient();

   int operator == (const HttpClient& c)   const { return this == &c; }

   HttpResponse*     DoRequest(HttpRequest& request);

protected:
   void              CombineCookies(List<HttpParam>& dst, List<HttpParam>& src);

   List<HttpParam>   cookies;
};


#endif HttpClient_h