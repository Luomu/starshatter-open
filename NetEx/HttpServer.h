/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         HttpServer.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Server Pump for HTTP Server
*/


#ifndef HttpServer_h
#define HttpServer_h

#include "NetServer.h"

// +-------------------------------------------------------------------+

class HttpParam;
class HttpRequest;
class HttpResponse;

// +-------------------------------------------------------------------+

class HttpServer : public NetServer
{
public:
   static const char* TYPENAME() { return "HttpServer"; }

   HttpServer(WORD port, int poolsize=1);
   virtual ~HttpServer();

   int operator == (const HttpServer& l) const { return addr == l.addr; }

   virtual Text      ProcessRequest(Text request, const NetAddr& addr);
   virtual Text      DefaultResponse();
   virtual Text      ErrorResponse();

   virtual bool      DoGet(HttpRequest& request,  HttpResponse& response);
   virtual bool      DoPost(HttpRequest& request, HttpResponse& response);
   virtual bool      DoHead(HttpRequest& request, HttpResponse& response);

   virtual Text      GetServerName();
   virtual void      SetServerName(const char* name);

protected:
   Text              http_server_name;
};

// +-------------------------------------------------------------------+

class HttpParam
{
public:
   static const char* TYPENAME() { return "HttpParam"; }

   HttpParam(const char* n, const char* v) : name(n), value(v) { }

   int operator == (const HttpParam& p) const { return name == p.name; }

   Text  name;
   Text  value;
};

// +-------------------------------------------------------------------+

class HttpRequest
{
public:
   static const char* TYPENAME() { return "HttpRequest"; }

   enum METHOD {
      HTTP_OPTIONS,
      HTTP_GET,
      HTTP_HEAD,
      HTTP_POST,
      HTTP_PUT,
      HTTP_DELETE,
      HTTP_TRACE,
      HTTP_CONNECT
   };

   HttpRequest(const char* request=0);
   ~HttpRequest();

   operator Text();

   void              ParseRequest(Text request);
   void              ParseCookie(const char* param);

   int               Method()       const { return method;        }
   Text              URI()          const { return uri;           }
   Text              Content()      const { return content;       }
   Text              RequestLine()  const { return request_line;  }

   List<HttpParam>&  GetQuery()           { return query;         }
   List<HttpParam>&  GetHeaders()         { return headers;       }
   List<HttpParam>&  GetCookies()         { return cookies;       }

   NetAddr           GetClientAddr()            const { return client_addr; }
   void              SetClientAddr(const NetAddr& a)  { client_addr = a;    }

   Text              GetParam(const char* name);

   Text              GetHeader(const char* name);
   void              SetHeader(const char* name, const char* value);
   void              AddHeader(const char* name, const char* value);
   Text              GetCookie(const char* name);
   void              SetCookie(const char* name, const char* value);
   void              AddCookie(const char* name, const char* value);

   Text              DecodeParam(const char* value);
   static Text       EncodeParam(const char* value);

private:
   int               method;
   Text              uri;
   Text              content;
   Text              request_line;
   NetAddr           client_addr;

   List<HttpParam>   query;
   List<HttpParam>   headers;
   List<HttpParam>   cookies;
};

// +-------------------------------------------------------------------+

class HttpResponse
{
public:
   static const char* TYPENAME() { return "HttpResponse"; }

   enum STATUS {
      SC_CONTINUE             = 100,
      SC_SWITCHING_PROTOCOLS  = 101,

      SC_OK                   = 200,
      SC_CREATED              = 201,
      SC_ACCEPTED             = 202,
      SC_NON_AUTHORITATIVE    = 203,
      SC_NO_CONTENT           = 204,
      SC_RESET_CONTENT        = 205,
      SC_PARTIAL_CONTENT      = 206,

      SC_MULTIPLE_CHOICES     = 300,
      SC_MOVED_PERMANENTLY    = 301,
      SC_FOUND                = 302,
      SC_SEE_OTHER            = 303,
      SC_NOT_MODIFIED         = 304,
      SC_USE_PROXY            = 305,
      SC_TEMPORARY_REDIRECT   = 307,

      SC_BAD_REQUEST          = 400,
      SC_UNAUTHORIZED         = 401,
      SC_PAYMENT_REQUIRED     = 402,
      SC_FORBIDDEN            = 403,
      SC_NOT_FOUND            = 404,
      SC_METHOD_NOT_ALLOWED   = 405,
      SC_NOT_ACCEPTABLE       = 406,
      SC_PROXY_AUTH_REQ       = 407,
      SC_REQUEST_TIME_OUT     = 408,
      SC_CONFLICT             = 409,
      SC_GONE                 = 410,
      SC_LENGTH_REQUIRED      = 411,

      SC_SERVER_ERROR         = 500,
      SC_NOT_IMPLEMENTED      = 501,
      SC_BAD_GATEWAY          = 502,
      SC_SERVICE_UNAVAILABLE  = 503,
      SC_GATEWAY_TIMEOUT      = 504,
      SC_VERSION_NOT_SUPPORTED= 505
   };


   HttpResponse(int status=500, const char* content=0);
   HttpResponse(const char* response);
   ~HttpResponse();

   operator Text();

   void              ParseResponse(Text request);
   void              ParseCookie(const char* param);

   int               Status()       const { return status;  }
   void              SetStatus(int s)     { status = s;     }

   Text              Content()      const { return content; }
   void              SetContent(Text t)   { content = t;    }
   void              AddContent(Text t)   { content += t;   }

   List<HttpParam>&  GetHeaders()         { return headers; }
   List<HttpParam>&  GetCookies()         { return cookies; }

   Text              GetHeader(const char* name);
   void              SetHeader(const char* name, const char* value);
   void              AddHeader(const char* name, const char* value);
   Text              GetCookie(const char* name);
   void              SetCookie(const char* name, const char* value);
   void              AddCookie(const char* name, const char* value);

   void              SendRedirect(const char* url);

private:
   int               status;
   Text              content;

   List<HttpParam>   headers;
   List<HttpParam>   cookies;
};


#endif HttpServer_h