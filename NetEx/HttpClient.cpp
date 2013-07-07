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