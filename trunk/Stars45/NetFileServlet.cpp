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
        strcpy_s(filename, name.data() + 1);  // skip leading '/'

        FILE* f;
        ::fopen_s(&f, filename, "rb");
        
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
