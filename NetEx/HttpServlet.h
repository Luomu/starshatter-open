/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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