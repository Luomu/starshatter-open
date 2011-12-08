/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NetFileServlet.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    HTTP Servlet for File Transfer
*/


#include "MemDebug.h"
#include "NetFileServlet.h"
#include "NetAdminServer.h"
#include "NetLayer.h"

#include "DataLoader.h"

// +-------------------------------------------------------------------+

bool
NetFileServlet::DoGet(HttpRequest& request, HttpResponse& response)
{
   if (!CheckUser(request, response))
      return true;

   Text content;
   Text path = request.GetParam("path");
   Text name = request.GetParam("name");

   if (name.length()) {
      BYTE*       buffer = 0;
      DataLoader* loader = DataLoader::GetLoader();

      if (loader) {
         loader->SetDataPath(path);
         int len = loader->LoadBuffer(name, buffer);

         if (len) {
            content = Text((const char*) buffer, len);
         }

         loader->ReleaseBuffer(buffer);
         loader->SetDataPath(0);
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

// +-------------------------------------------------------------------+

bool
NetWebServlet::DoGet(HttpRequest& request, HttpResponse& response)
{
   Text content;
   Text name = request.URI();
   bool found = false;

   if (name.length() > 4) {
      char filename[256];
      strcpy(filename, name.data() + 1);  // skip leading '/'

      FILE* f = ::fopen(filename, "rb");

      if (f) {
         ::fseek(f, 0, SEEK_END);
         int len = ftell(f);
         ::fseek(f, 0, SEEK_SET);

         BYTE* buf = new(__FILE__,__LINE__) BYTE[len];

         ::fread(buf, len, 1, f);
         ::fclose(f);

         content = Text((const char*) buf, len);
         delete [] buf;

         found = true;
         ::Print("weblog: 200 OK %s %d bytes\n", name.data(), len);
      }
      else {
         ::Print("weblog: 404 Not Found %s\n", name.data());
      }
   }

   if (found) {
      Text content_type = "text/plain";

      if (name.contains(".gif"))
         content_type = "image/gif";
      else if (name.contains(".jpg"))
         content_type = "image/jpeg";
      else if (name.contains(".htm"))
         content_type = "text/html";

      response.SetStatus(HttpResponse::SC_OK);
      response.AddHeader("MIME-Version",  "1.0");
      response.AddHeader("Content-Type",  content_type);
      response.SetContent(content);
   }
   else {
      response.SetStatus(HttpResponse::SC_NOT_FOUND);
   }

   return true;
}
