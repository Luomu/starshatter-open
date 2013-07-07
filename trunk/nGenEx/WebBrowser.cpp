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
    FILE:         WebBrowser.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Helper class to find and launch user's default web browser
*/

#include "MemDebug.h"
#include "WebBrowser.h"

// +--------------------------------------------------------------------+

WebBrowser::WebBrowser()
{
    FindDefaultBrowser();
    FindOpenCommand();
}

WebBrowser::~WebBrowser()
{
}

// +--------------------------------------------------------------------+


void
WebBrowser::OpenURL(const char* url)
{
    if (url) {
        char cmdline[256];

        if (command.contains("%1")) {
            strcpy_s(cmdline, command.replace("%1", url).data());
        }
        else {
            strcpy_s(cmdline, Text(command + " " + url).data());
        }

        STARTUPINFO s;
        ZeroMemory(&s, sizeof(s));
        s.cb = sizeof(s);

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        if (CreateProcess(NULL, cmdline, 0, 0, 0, 0, 0, 0, &s, &pi)) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            ::Print("Unable to launch web browser for url '%s'\n", url);
        }
    }
}

// +--------------------------------------------------------------------+

void 
WebBrowser::FindDefaultBrowser()
{
    HKEY  hkey;
    char  value[256] = "";
    DWORD dwSize;

    ZeroMemory(value, 256);

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT,
                ".html",
                0,
                KEY_READ,
                &hkey) == ERROR_SUCCESS) {

        dwSize = 256;
        RegQueryValueEx(hkey,
        "",
        NULL,
        NULL,
        (LPBYTE) value,
        &dwSize);

        RegCloseKey(hkey);

        if (dwSize > 0) {
            ::Print("Default Web Browser: %s\n", value);
            browser = value;
        }
    }
}

// +--------------------------------------------------------------------+

void
WebBrowser::FindOpenCommand()
{
    HKEY  hkey;
    char  value[256] = "";
    DWORD dwSize;

    ZeroMemory(value, 256);

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT,
                browser + "\\shell\\open\\command",
                0,
                KEY_READ,
                &hkey) == ERROR_SUCCESS) {

        dwSize = 256;
        RegQueryValueEx(hkey,
        "",
        NULL,
        NULL,
        (LPBYTE) value,
        &dwSize);

        RegCloseKey(hkey);

        if (dwSize > 0) {
            ::Print("Browser Shell Open Command: %s\n", value);
            command = value;
        }
    }
}

// +--------------------------------------------------------------------+


