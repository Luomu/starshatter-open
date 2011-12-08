/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         HttpServlet.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Server Pump for HTTP Server
*/


#include "MemDebug.h"
#include "HttpServlet.h"
#include "NetLayer.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// +-------------------------------------------------------------------+

HttpServlet::HttpServlet()
   : session(0)
{ }

HttpServlet::~HttpServlet()
{ }

// +--------------------------------------------------------------------+

bool
HttpServlet::Service(HttpRequest& request, HttpResponse& response)
{
   bool result = false;

   switch (request.Method()) {
   case HttpRequest::HTTP_GET:
      result = DoGet(request, response);
      break;

   case HttpRequest::HTTP_POST:
      result = DoPost(request, response);
      break;

   case HttpRequest::HTTP_HEAD:
      result = DoHead(request, response);
      break;

   default:
      break;
   }

   return result;
}

// +--------------------------------------------------------------------+

bool
HttpServlet::DoGet(HttpRequest& request, HttpResponse& response)
{
   return false;
}

bool
HttpServlet::DoPost(HttpRequest& request, HttpResponse& response)
{
   return DoGet(request, response);
}

bool
HttpServlet::DoHead(HttpRequest& request, HttpResponse& response)
{
   if (DoGet(request, response)) {
      int len = response.Content().length();
 
      char buffer[256];
      sprintf(buffer, "%d", len);
      response.SetHeader("Content-Length", buffer);
      response.SetContent("");

      return true;
   }

   return false;
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

HttpSession::HttpSession()
{
   id          = GenerateUniqueID();
   access_time = NetLayer::GetUTC();
}

HttpSession::~HttpSession()
{
   attributes.destroy();
}

// +--------------------------------------------------------------------+

Text
HttpSession::GetAttribute(const char* name)
{
   ListIter<HttpParam> iter = attributes;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name)
         return p->value;
   }

   return Text();
}

void
HttpSession::SetAttribute(const char* name, const char* value)
{
   ListIter<HttpParam> iter = attributes;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name) {
         p->value = value;
         return;
      }
   }

   HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, value);
   if (param)
      attributes.append(param);
}

void
HttpSession::DelAttribute(const char* name)
{
   ListIter<HttpParam> iter = attributes;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name) {
         delete iter.removeItem();
         return;
      }
   }
}

// +--------------------------------------------------------------------+

int
HttpSession::GetIntAttribute(const char* name)
{
   ListIter<HttpParam> iter = attributes;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name) {
         int result = ::atoi(p->value.data());
         return result;
      }
   }

   return 0;
}

void
HttpSession::SetIntAttribute(const char* name, int value)
{
   char buf[32];
   sprintf(buf, "%d", value);
 
   ListIter<HttpParam> iter = attributes;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name) {
         p->value = buf;
         return;
      }
   }

   HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, buf);
   if (param)
      attributes.append(param);
}

void
HttpSession::DelIntAttribute(const char* name)
{
   DelAttribute(name);
}

// +--------------------------------------------------------------------+

Text
HttpSession::GenerateUniqueID()
{
   char unique[17];

   for (int i = 0; i < 16; i++) {
      char c = rand() % 25 + 'a';
      unique[i] = c;
   }

   unique[16] = 0;
   return unique;
}

// +--------------------------------------------------------------------+

void
HttpSession::Access()
{
   access_time = NetLayer::GetUTC();
}


