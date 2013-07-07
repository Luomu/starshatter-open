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

    SUBSYSTEM:    nGenEx.lib
    FILE:         ContentBundle.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Chained collection of localized strings
*/

#include "MemDebug.h"
#include "ContentBundle.h"
#include "DataLoader.h"

void  Print(const char* fmt, ...);

// +--------------------------------------------------------------------+

ContentBundle::ContentBundle(const char* bundle, Locale* locale)
{
    Text file = FindFile(bundle, locale);
    if (file.length() > 0) {
        LoadBundle(file);
    }
}

// +--------------------------------------------------------------------+

ContentBundle::~ContentBundle()
{
}

// +--------------------------------------------------------------------+

Text
ContentBundle::GetText(const char* key) const
{
    return values.find(key, Text(key));
}

// +--------------------------------------------------------------------+

Text
ContentBundle::FindFile(const char* bundle, Locale* locale)
{
    Text        result;
    Text        basename = Text(bundle);
    DataLoader* loader = DataLoader::GetLoader();

    if (loader && bundle) {
        if (locale) {
            result = basename + locale->GetFullCode() + ".txt";

            if (loader->FindFile(result))
            return result;

            result = basename + "_" + locale->GetLanguage() + ".txt";

            if (loader->FindFile(result))
            return result;
        }

        result = basename + ".txt";

        if (loader->FindFile(result))
        return result;
    }

    return Text();
}

// +--------------------------------------------------------------------+

void
ContentBundle::LoadBundle(const char* filename)
{
    DataLoader* loader = DataLoader::GetLoader();
    if (loader && filename && *filename) {
        BYTE* buffer = 0;
        loader->LoadBuffer(filename, buffer, true, true);
        if (buffer && *buffer) {
            char  key[1024];
            char  val[2048];
            char* p = (char*) buffer;
            int   s = 0, ik = 0, iv = 0;

            key[0] = 0;
            val[0] = 0;

            while (*p) {
                if (*p == '=') {
                    s = 1;
                }
                else if (*p == '\n' || *p == '\r') {
                    if (key[0] && val[0])
                    values.insert(Text(key).trim(), Text(val).trim());

                    ZeroMemory(key, 1024);
                    ZeroMemory(val, 2048);
                    s   = 0;
                    ik  = 0;
                    iv  = 0;
                }
                else if (s == 0) {
                    if (!key[0]) {
                        if (*p == '#') {
                            s = -1; // comment
                        }
                        else if (!isspace(*p)) {
                            key[ik++] = *p;
                        }
                    }
                    else {
                        key[ik++] = *p;
                    }
                }
                else if (s == 1) {
                    if (!isspace(*p)) {
                        s = 2;
                        val[iv++] = *p;
                    }
                }
                else if (s == 2) {
                    val[iv++] = *p;
                }

                p++;
            }

            loader->ReleaseBuffer(buffer);
        }
    }
}
