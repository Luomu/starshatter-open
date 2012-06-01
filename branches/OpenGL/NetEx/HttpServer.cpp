/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         HttpServer.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Server Pump for HTTP Server
*/


#include "MemDebug.h"
#include "HttpServer.h"
#include "NetLayer.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// +-------------------------------------------------------------------+

HttpServer::HttpServer(WORD port, int poolsize)
   : NetServer(port, poolsize)
{
   http_server_name = "Generic HttpServer 1.0";
}

HttpServer::~HttpServer()
{ }

// +--------------------------------------------------------------------+

Text
HttpServer::ProcessRequest(Text msg, const NetAddr& addr)
{
   HttpRequest    request(msg);
   HttpResponse   response;

   request.SetClientAddr(addr);

   switch (request.Method()) {
   case HttpRequest::HTTP_GET:
      if (DoGet(request, response))
         return response;

   case HttpRequest::HTTP_POST:
      if (DoPost(request, response))
         return response;

   case HttpRequest::HTTP_HEAD:
      if (DoHead(request, response))
         return response;
   }

   return ErrorResponse();
}

// +--------------------------------------------------------------------+

Text
HttpServer::GetServerName()
{
   return http_server_name;
}

void
HttpServer::SetServerName(const char* name)
{
   http_server_name = name;
}

// +--------------------------------------------------------------------+

Text
HttpServer::DefaultResponse()
{
   Text response = "HTTP/1.1 200 OK\nServer: ";
   response += http_server_name;
   response += "\nMIME-Version: 1.0\nContent-Type: text/html\nConnection: close\n\n";

   return response;
}

Text
HttpServer::ErrorResponse()
{
   Text response = "HTTP/1.1 500 Internal Server Error\nServer:";
   response += http_server_name;
   response += "\nMIME-Version: 1.0\nContent-Type: text/html\nConnection: close\n\n";

   response += "<html><head><title>";
   response += http_server_name;
   response += " Error</title></head>\n";
   response += "<body bgcolor=\"black\" text=\"white\">\n<h1>";
   response += http_server_name;
   response += "</h1>\n<p>Veruca... sweetheart... angel...  I'm not a magician!\n";
   response += "</body></html>\n\n";

   return response;
}

// +--------------------------------------------------------------------+

bool
HttpServer::DoGet(HttpRequest& request, HttpResponse& response)
{
   char buffer[1024];
   Text content;

   content  = "<html><head><title>";
   content += http_server_name;
   content += "</title></head>\n";
   content += "<body bgcolor=\"white\" text=\"black\">\n<h1>";
   content += http_server_name;
   content += "</h1>\n";
   content += "<br><h3>Client Address:</h3><p>\n";

   sprintf_s(buffer, "%d.%d.%d.%d:%d<br><br>\n",
            client_addr.B1(),
            client_addr.B2(),
            client_addr.B3(),
            client_addr.B4(),
            client_addr.Port());

   content += buffer;
   content += "<h3>Request Method:</h3><p>\n";

   switch (request.Method()) {
   case HttpRequest::HTTP_GET:
      content += "GET";
      break;

   case HttpRequest::HTTP_POST:
      content += "POST";
      break;

   case HttpRequest::HTTP_HEAD:
      content += "HEAD";
      break;

   default:
      content += "(unsupported?)";
      break;
   }

   content += "<br>\n";
   content += "<br><h3>URI Requested:</h3><p>\n";
   content += request.URI();
   content += "<br>\n";

   if (request.GetQuery().size() > 0) {
      content += "<br><h3>Query Parameters:</h3>\n";

      ListIter<HttpParam> q_iter = request.GetQuery();
      while (++q_iter) {
         HttpParam* q = q_iter.value();
         sprintf_s(buffer, "<b>%s:</b> <i>%s</i><br>\n", q->name.data(), q->value.data());
         content += buffer;
      }
   }

   content += "<br><h3>Request Headers:</h3>\n";
   ListIter<HttpParam> h_iter = request.GetHeaders();
   while (++h_iter) {
      HttpParam* h = h_iter.value();
      sprintf_s(buffer, "<b>%s:</b> <i>%s</i><br>\n", h->name.data(), h->value.data());
      content += buffer;
   }

   content += "</body></html>\n\n";

   response.SetStatus(HttpResponse::SC_OK);
   response.AddHeader("Server",        http_server_name);
   response.AddHeader("MIME-Version",  "1.0");
   response.AddHeader("Content-Type",  "text/html");
   response.SetContent(content);

   return true;
}

// +--------------------------------------------------------------------+

bool
HttpServer::DoPost(HttpRequest& request, HttpResponse& response)
{
   return DoGet(request, response);
}

// +--------------------------------------------------------------------+

bool
HttpServer::DoHead(HttpRequest& request, HttpResponse& response)
{
   if (DoGet(request, response)) {
      int len = response.Content().length();
 
      char buffer[256];
      sprintf_s(buffer, "%d", len);
      response.SetHeader("Content-Length", buffer);
      response.SetContent("");

      return true;
   }

   return false;
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

HttpRequest::HttpRequest(const char* r)
   : method(0)
{
   if (r && *r)
      ParseRequest(r);
}

HttpRequest::~HttpRequest()
{
   query.destroy();
   headers.destroy();
   cookies.destroy();
}

// +--------------------------------------------------------------------+

void
HttpRequest::ParseRequest(Text request)
{
   if (request.length() <= 8)
      return;

   const char* pReq    = 0;
   const char* pURI    = 0;
   const char* pQuery  = 0;

   switch (request[0]) {
   case 'G':   
      if (request.indexOf("GET") == 0)
         method = HTTP_GET;
      break;

   case 'P':
      if (request.indexOf("POST") == 0)
         method = HTTP_POST;
      break;

   case 'H':
      if (request.indexOf("HEAD") == 0)
         method = HTTP_HEAD;
      break;

   default:
      break;
   }

   if (!method) return;

   char buffer[1024];
   int  i = 0;

   // save the request line:
   pReq = request.data();
   while (*pReq && *pReq != '\n')
      buffer[i++] = *pReq++;
   buffer[i] = 0;

   request_line = buffer;
   i = 0;

   // find the URI:
   pURI = request.data();
   while (*pURI && !isspace(*pURI))
      pURI++;

   while (*pURI && isspace(*pURI))
      pURI++;

   // copy the URI and find the query string:
   while (*pURI && *pURI != '?' && !isspace(*pURI)) {
      buffer[i++] = *pURI++;
   }

   buffer[i] = 0;
   uri = buffer;
   pQuery = pURI;

   // parse the query string:
   if (*pQuery == '?') {
      pQuery++;

      while (*pQuery && !isspace(*pQuery)) {
         char name_buf[1024];
         char value_buf[1024];

         i = 0;
         while (*pQuery && *pQuery != '=' && !isspace(*pQuery))
            name_buf[i++] = *pQuery++;
         name_buf[i] = 0;

         if (*pQuery == '=')
            pQuery++;

         i = 0;
         while (*pQuery && *pQuery != '&' && !isspace(*pQuery))
            value_buf[i++] = *pQuery++;
         value_buf[i] = 0;

         if (*pQuery == '&')
            pQuery++;

         HttpParam* param = new(__FILE__,__LINE__) HttpParam(name_buf, DecodeParam(value_buf));
         if (param)
            query.append(param);
      }
   }

   // get the headers:
   const char* p = request.data();
   while (*p && *p != '\n')
      p++;

   if (*p == '\n') p++;

   while (*p && *p != '\r' && *p != '\n') {
      char name_buf[1024];
      char value_buf[1024];

      i = 0;
      while (*p && *p != ':')
         name_buf[i++] = *p++;
      name_buf[i] = 0;

      p++;                       // skip ':'
      while (isspace(*p)) p++;   // skip spaces

      i = 0;
      while (*p && *p != '\r' && *p != '\n') // read to end of header line
         value_buf[i++] = *p++;
      value_buf[i] = 0;

      if (!_stricmp(name_buf, "Cookie")) {
         ParseCookie(value_buf);
      }
      else {
         HttpParam* param = new(__FILE__,__LINE__) HttpParam(name_buf, value_buf);
         if (param)
            headers.append(param);
      }

      while (*p && *p != '\n')
         p++;

      if (*p == '\n') p++;
   }

   if (method == HTTP_POST && *p) {
      while (*p == '\n' || *p == '\r')
         p++;

      content = *p;
      pQuery  = p;

      while (*pQuery && !isspace(*pQuery)) {
         char name_buf[1024];
         char value_buf[1024];

         i = 0;
         while (*pQuery && *pQuery != '=' && !isspace(*pQuery))
            name_buf[i++] = *pQuery++;
         name_buf[i] = 0;

         if (*pQuery == '=')
            pQuery++;

         i = 0;
         while (*pQuery && *pQuery != '&' && !isspace(*pQuery))
            value_buf[i++] = *pQuery++;
         value_buf[i] = 0;

         if (*pQuery == '&')
            pQuery++;

         HttpParam* param = new(__FILE__,__LINE__) HttpParam(name_buf, DecodeParam(value_buf));
         if (param)
            query.append(param);
      }
   }
}

void
HttpRequest::ParseCookie(const char* param)
{
   const char* p = param;

   while (p && *p) {
      while (isspace(*p)) p++;

      // just ignore reserved attributes
      if (*p == '$') {
         while (*p && !isspace(*p) && *p != ';') p++;

         if (*p == ';')
            p++;
      }

      // found a cookie!
      else if (isalpha(*p)) {
         char name[1024];
         char data[1024];

         char* d = name;
         while (*p && *p != '=')
            *d++ = *p++;
         *d = 0;

         if (*p == '=')
            p++;

         if (*p == '"')
            p++;

         d = data;
         while (*p && *p != '"' && *p != ';')
            *d++ = *p++;
         *d = 0;

         if (*p == '"')
            p++;

         if (*p == ';')
            p++;

         HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, data);
         if (param)
            cookies.append(param);
      }

      // this shouldn't happen - abandon the parse
      else {
         return;
      }
   }
}

// +--------------------------------------------------------------------+

Text
HttpRequest::GetParam(const char* name)
{
   ListIter<HttpParam> iter = query;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name)
         return p->value;
   }

   return Text();
}

// +--------------------------------------------------------------------+

Text
HttpRequest::GetHeader(const char* name)
{
   ListIter<HttpParam> iter = headers;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name)
         return p->value;
   }

   return Text();
}

void
HttpRequest::SetHeader(const char* name, const char* value)
{
   ListIter<HttpParam> iter = headers;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name) {
         p->value = value;
         return;
      }
   }

   HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, value);
   if (param)
      headers.append(param);
}

void
HttpRequest::AddHeader(const char* name, const char* value)
{
   HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, value);
   if (param)
      headers.append(param);
}

// +--------------------------------------------------------------------+

Text
HttpRequest::GetCookie(const char* name)
{
   ListIter<HttpParam> iter = cookies;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name)
         return p->value;
   }

   return Text();
}

void
HttpRequest::SetCookie(const char* name, const char* value)
{
   ListIter<HttpParam> iter = cookies;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name) {
         p->value = value;
         return;
      }
   }

   HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, value);
   if (param)
      cookies.append(param);
}

void
HttpRequest::AddCookie(const char* name, const char* value)
{
   HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, value);
   if (param)
      cookies.append(param);
}

// +--------------------------------------------------------------------+

Text
HttpRequest::DecodeParam(const char* value)
{
   if (!value || !*value) return "";

   int   size   = strlen(value);
   char  val    = 0;
   char  code[4];
   char  sbuf[256];
   char* lbuf = 0;

   char* dst    = sbuf;
   char* p      = sbuf;

   if (size > 255) {
      lbuf   = new(__FILE__,__LINE__) char[size+1];
      dst    = lbuf;
      p      = lbuf;
   }

   if (p) {
      while (*value) {
         switch (*value) {
         default:    *p++ = *value; break;
         case '+':   *p++ = ' ';    break;

         case '%':
            value++;
            code[0] = *value++;
            code[1] = *value;
            code[2] = 0;

            val = (char) strtol(code, 0, 16);
            *p++ = val;
            break;
         }

         value++;
      }

      *p = 0;
   }

   Text  result = dst;

   if (lbuf)
      delete [] lbuf;

   return result;
}

// +--------------------------------------------------------------------+

Text
HttpRequest::EncodeParam(const char* value)
{
   if (!value || !*value) return "";

   int   size   = strlen(value);
   char  hex1   = 0;
   char  hex2   = 0;

   char  sbuf[1024];
   char* lbuf = 0;

   char* dst    = sbuf;
   char* p      = sbuf;

   if (size > 255) {
      lbuf   = new(__FILE__,__LINE__) char[4*size+1];
      dst    = lbuf;
      p      = lbuf;
   }

   if (p) {
      while (*value) {
         switch (*value) {
         default:    *p++ = *value; break;
         case ' ':   *p++ = '+';    break;

         case '?':   *p++ = '%'; *p++ = '3'; *p++ = 'F'; break;
         case '&':   *p++ = '%'; *p++ = '2'; *p++ = '6'; break;
         case ':':   *p++ = '%'; *p++ = '3'; *p++ = 'A'; break;
         case '/':   *p++ = '%'; *p++ = '2'; *p++ = 'F'; break;
         case '\\':  *p++ = '%'; *p++ = '5'; *p++ = 'C'; break;
         case '%':   *p++ = '%'; *p++ = '2'; *p++ = '5'; break;
         case '|':   *p++ = '%'; *p++ = '7'; *p++ = 'C'; break;
         case '<':   *p++ = '%'; *p++ = '3'; *p++ = 'C'; break;
         case '>':   *p++ = '%'; *p++ = '3'; *p++ = 'E'; break;
         case '[':   *p++ = '%'; *p++ = '5'; *p++ = 'B'; break;
         case ']':   *p++ = '%'; *p++ = '5'; *p++ = 'D'; break;
         case '{':   *p++ = '%'; *p++ = '7'; *p++ = 'B'; break;
         case '}':   *p++ = '%'; *p++ = '7'; *p++ = 'D'; break;
         case '"':   *p++ = '%'; *p++ = '2'; *p++ = '2'; break;
         case '^':   *p++ = '%'; *p++ = '5'; *p++ = 'E'; break;
         case '`':   *p++ = '%'; *p++ = '6'; *p++ = '0'; break;
         case '\n':  break;
         case '\r':  break;
         case '\t':  break;
         }

         value++;
      }

      *p = 0;
   }

   Text  result = dst;

   if (lbuf)
      delete [] lbuf;

   return result;
}

// +--------------------------------------------------------------------+

HttpRequest::operator Text()
{
   Text response = request_line.data();
   response += "\n";

   for (int i = 0; i < headers.size(); i++) {
      HttpParam* h = headers[i];
      response += h->name;
      response += ": ";
      response += h->value;
      response += "\n";
   }

   for (int i = 0; i < cookies.size(); i++) {
      HttpParam* c = cookies[i];
      response += "Cookie: ";
      response += c->name;
      response += "=\"";
      response += c->value;
      response += "\"\n";
   }

   response += "Connection: close\n\n";
   response += content;

   return response;
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

HttpResponse::HttpResponse(int stat, const char* data)
   : status(stat), content(data)
{ }

HttpResponse::HttpResponse(const char* r)
   : status(0), content(r)
{
   if (r && *r)
      ParseResponse(r);
}

HttpResponse::~HttpResponse()
{
   headers.destroy();
   cookies.destroy();
}

// +--------------------------------------------------------------------+

HttpResponse::operator Text()
{
   Text response;

   switch (status) {
   case SC_CONTINUE             : response = "HTTP/1.1 100 Continue\n";                break;
   case SC_SWITCHING_PROTOCOLS  : response = "HTTP/1.1 101 Switching Protocols\n";     break;

   case SC_OK                   : response = "HTTP/1.1 200 OK\n";                      break;
   case SC_CREATED              : response = "HTTP/1.1 201 Created\n";                 break;
   case SC_ACCEPTED             : response = "HTTP/1.1 202 Accepted\n";                break;
   case SC_NON_AUTHORITATIVE    : response = "HTTP/1.1 203 Non Authoritative\n";       break;
   case SC_NO_CONTENT           : response = "HTTP/1.1 204 No Content\n";              break;
   case SC_RESET_CONTENT        : response = "HTTP/1.1 205 Reset Content\n";           break;
   case SC_PARTIAL_CONTENT      : response = "HTTP/1.1 206 Partial Content\n";         break;

   case SC_MULTIPLE_CHOICES     : response = "HTTP/1.1 300 Multiple Choices\n";        break;
   case SC_MOVED_PERMANENTLY    : response = "HTTP/1.1 301 Moved Permanently\n";       break;
   case SC_FOUND                : response = "HTTP/1.1 302 Found\n";                   break;
   case SC_SEE_OTHER            : response = "HTTP/1.1 303 See Other\n";               break;
   case SC_NOT_MODIFIED         : response = "HTTP/1.1 304 Not Modified\n";            break;
   case SC_USE_PROXY            : response = "HTTP/1.1 305 Use Proxy\n";               break;
   case SC_TEMPORARY_REDIRECT   : response = "HTTP/1.1 307 Temporary Redirect\n";      break;

   case SC_BAD_REQUEST          : response = "HTTP/1.1 400 Bad Request\n";             break;
   case SC_UNAUTHORIZED         : response = "HTTP/1.1 401 Unauthorized\n";            break;
   case SC_PAYMENT_REQUIRED     : response = "HTTP/1.1 402 Payment Required\n";        break;
   case SC_FORBIDDEN            : response = "HTTP/1.1 403 Forbidden\n";               break;
   case SC_NOT_FOUND            : response = "HTTP/1.1 404 Not Found\n";               break;
   case SC_METHOD_NOT_ALLOWED   : response = "HTTP/1.1 405 Method Not Allowed\n";      break;
   case SC_NOT_ACCEPTABLE       : response = "HTTP/1.1 406 Not Acceptable\n";          break;
   case SC_PROXY_AUTH_REQ       : response = "HTTP/1.1 407 Proxy Authorization Req\n"; break;
   case SC_REQUEST_TIME_OUT     : response = "HTTP/1.1 408 Request Timeout\n";         break;
   case SC_CONFLICT             : response = "HTTP/1.1 409 Conflict\n";                break;
   case SC_GONE                 : response = "HTTP/1.1 410 Gone\n";                    break;
   case SC_LENGTH_REQUIRED      : response = "HTTP/1.1 411 Length Required\n";         break;

   default:
   case SC_SERVER_ERROR         : response = "HTTP/1.1 500 Internal Server Error\n";   break;
   case SC_NOT_IMPLEMENTED      : response = "HTTP/1.1 501 Not Implemented\n";         break;
   case SC_BAD_GATEWAY          : response = "HTTP/1.1 502 Bad Gateway\n";             break;
   case SC_SERVICE_UNAVAILABLE  : response = "HTTP/1.1 503 Service Unavailable\n";     break;
   case SC_GATEWAY_TIMEOUT      : response = "HTTP/1.1 504 Gateway Timeout\n";         break;
   case SC_VERSION_NOT_SUPPORTED: response = "HTTP/1.1 505 HTTP Version Not Supported\n"; break;
   }

   SetHeader("Connection", "close");

   char buffer[256];

   if (content.length()) {
      sprintf_s(buffer, "%d", content.length());
      SetHeader("Content-Length", buffer);
   }

   for (int i = 0; i < cookies.size(); i++) {
      HttpParam* cookie = cookies.at(i);
      sprintf_s(buffer, "%s=\"%s\"; Version=\"1\"", cookie->name.data(), cookie->value.data());

      AddHeader("Set-Cookie", buffer);
   }

   for (int i = 0; i < headers.size(); i++) {
      const HttpParam* p = headers.at(i);
      sprintf_s(buffer, "%s: %s\n", p->name.data(), p->value.data());
      response += buffer;
   }

   response += "\n";
   response += content;

   return response;
}

// +--------------------------------------------------------------------+

Text
HttpResponse::GetHeader(const char* name)
{
   ListIter<HttpParam> iter = headers;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name)
         return p->value;
   }

   return Text();
}

void
HttpResponse::SetHeader(const char* name, const char* value)
{
   ListIter<HttpParam> iter = headers;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name) {
         p->value = value;
         return;
      }
   }

   HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, value);
   if (param)
      headers.append(param);
}

void
HttpResponse::AddHeader(const char* name, const char* value)
{
   HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, value);
   if (param)
      headers.append(param);
}

// +--------------------------------------------------------------------+

Text
HttpResponse::GetCookie(const char* name)
{
   ListIter<HttpParam> iter = cookies;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name)
         return p->value;
   }

   return Text();
}

void
HttpResponse::SetCookie(const char* name, const char* value)
{
   ListIter<HttpParam> iter = cookies;
   while (++iter) {
      HttpParam* p = iter.value();

      if (p->name == name) {
         p->value = value;
         return;
      }
   }

   HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, value);
   if (param)
      cookies.append(param);
}

void
HttpResponse::AddCookie(const char* name, const char* value)
{
   HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, value);
   if (param)
      cookies.append(param);
}

// +--------------------------------------------------------------------+

void
HttpResponse::SendRedirect(const char* url)
{
   status = SC_TEMPORARY_REDIRECT;
   SetHeader("Location", url);
}

// +--------------------------------------------------------------------+

void
HttpResponse::ParseResponse(Text response)
{
   if (response.length() <= 12 || response.indexOf("HTTP/1.") != 0)
      return;

   const char* pStatus = response.data() + 9;

   sscanf_s(pStatus, "%d", &status);
   if (!status) return;

   int  i = 0;

   // get the headers:
   const char* p = response.data();
   while (*p && *p != '\n')
      p++;

   if (*p == '\n') p++;

   while (*p && *p != '\r' && *p != '\n') {
      char name_buf[1024];
      char value_buf[1024];

      i = 0;
      while (*p && *p != ':')
         name_buf[i++] = *p++;
      name_buf[i] = 0;

      p++;                       // skip ':'
      while (isspace(*p)) p++;   // skip spaces

      i = 0;
      while (*p && *p != '\r' && *p != '\n') // read to end of header line
         value_buf[i++] = *p++;
      value_buf[i] = 0;

      if (!_stricmp(name_buf, "Set-Cookie")) {
         ParseCookie(value_buf);
      }
      else {
         HttpParam* param = new(__FILE__,__LINE__) HttpParam(name_buf, value_buf);
         if (param)
            headers.append(param);
      }

      while (*p && *p != '\n')
         p++;

      if (*p == '\n') p++;
   }

   if (*p == '\n') p++;
   content = p;
}

void
HttpResponse::ParseCookie(const char* param)
{
   const char* p = param;

   while (p && *p) {
      while (isspace(*p)) p++;

      // just ignore reserved attributes
      if (*p == '$') {
         while (*p && !isspace(*p) && *p != ';') p++;

         if (*p == ';')
            p++;
      }

      // found a cookie!
      else if (isalpha(*p)) {
         char name[1024];
         char data[1024];

         char* d = name;
         while (*p && *p != '=')
            *d++ = *p++;
         *d = 0;

         if (*p == '=')
            p++;

         if (*p == '"')
            p++;

         d = data;
         while (*p && *p != '"' && *p != ';')
            *d++ = *p++;
         *d = 0;

         if (*p == '"')
            p++;

         if (*p == ';')
            p++;

         // ignore the version attribute
         if (_stricmp(name, "version")) {
            HttpParam* param = new(__FILE__,__LINE__) HttpParam(name, data);
            if (param)
               cookies.append(param);
         }
      }

      // this shouldn't happen - abandon the parse
      else {
         return;
      }
   }
}

