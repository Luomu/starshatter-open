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
    FILE:         FormatUtil.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+

void FormatNumber(char* txt, double n)
{
    double a = fabs(n);

    if (a < 1e3)
    sprintf(txt, "%d", (int) (n));

    else if (a < 1e6)
    sprintf(txt, "%.1f K", (n/1e3));

    else if (a < 1e9)
    sprintf(txt, "%.1f M", (n/1e6));

    else if (a < 1e12)
    sprintf(txt, "%.1f G", (n/1e9));

    else if (a < 1e15)
    sprintf(txt, "%.1f T", (n/1e12));

    else   
    sprintf(txt, "%.1e", n);
}

// +--------------------------------------------------------------------+

void FormatNumberExp(char* txt, double n)
{
    double a = fabs(n);

    if (a < 100e3)
    sprintf(txt, "%d", (int) (n));

    else
    sprintf(txt, "%.1e", n);
}

// +--------------------------------------------------------------------+

const int MINUTE  = 60;
const int HOUR    = 60 * MINUTE;
const int DAY     = 24 * HOUR;

void FormatTime(char* txt, double time)
{
    int t = (int) time;

    int h = (t                  / HOUR);
    int m = ((t - h*HOUR)       / MINUTE);
    int s = (t - h*HOUR - m*MINUTE);

    if (h > 0)
    sprintf(txt, "%02d:%02d:%02d", h,m,s);
    else
    sprintf(txt, "%02d:%02d", m,s);
}

// +--------------------------------------------------------------------+

void FormatTimeOfDay(char* txt, double time)
{
    int t = (int) time;

    if (t >= DAY) {
        int d = t / DAY;
        t -= d * DAY;
    }

    int h = (t                  / HOUR);
    int m = ((t - h*HOUR)       / MINUTE);
    int s = (t - h*HOUR - m*MINUTE);

    sprintf(txt, "%02d:%02d:%02d", h,m,s);
}

// +--------------------------------------------------------------------+

void FormatDayTime(char* txt, double time, bool short_format)
{
    int t = (int) time;
    int d = 1, h = 0, m = 0, s = 0;

    if (t >= DAY) {
        d =  t / DAY;
        t -= d * DAY;
        d++;
    }

    if (t >= HOUR) {
        h =  t / HOUR;
        t -= h * HOUR;
    }

    if (t >= MINUTE) {
        m =  t / MINUTE;
        t -= m * MINUTE;
    }

    s = t;

    if (short_format)
    sprintf(txt, "%02d/%02d:%02d:%02d", d, h, m, s);
    else
    sprintf(txt, "Day %02d %02d:%02d:%02d", d, h, m, s);
}

// +--------------------------------------------------------------------+

void FormatDay(char* txt, double time)
{
    int t = (int) time;
    int d = 1, h = 0, m = 0, s = 0;

    if (t >= DAY) {
        d =  t / DAY;
        t -= d * DAY;
        d++;
    }

    sprintf(txt, "Day %02d", d);
}

// +--------------------------------------------------------------------+

void FormatPoint(char* txt, const Point& p)
{
    char x[16];
    char y[16];
    char z[16];

    FormatNumber(x, p.x);
    FormatNumber(y, p.y);
    FormatNumber(z, p.z);

    sprintf(txt, "(%s, %s, %s)", x, y, z);
}

// +--------------------------------------------------------------------+

Text FormatTimeString(int utc)
{
    static const char* month[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    static const char* meridian[2] = { "AM", "PM" };

    if (utc < 1)
    utc = (int) time(0);

    time_t aclock = utc;                // Get time in seconds
    struct tm *t  = localtime(&aclock); // Convert time to struct tm form

    char buffer[256];
    sprintf(buffer, "%d %s %d, %2d:%02d:%02d %s",
    t->tm_mday, month[t->tm_mon], 1900 + t->tm_year,
    t->tm_hour > 12 ? t->tm_hour-12 : t->tm_hour,
    t->tm_min, t->tm_sec, meridian[t->tm_hour > 12]);

    return buffer;
}

// +--------------------------------------------------------------------+

static char safe_str[2048];

const char* SafeString(const char* s)
{
    ZeroMemory(safe_str, sizeof(safe_str));

    if (s && *s) {
        int len = strlen(s);
        int n   = 0;

        for (int i = 0; i < len; i++) {
            char c = s[i];

            if (c == '\n') {
                safe_str[n++] = '\\';
                safe_str[n++] = 'n';
            }

            else if (c == '\t') {
                safe_str[n++] = '\\';
                safe_str[n++] = 't';
            }

            else if (c == '"') {
                safe_str[n++] = '\'';
            }

            else if (c == '\\' && i < len-1) {
                safe_str[n++] = s[i++];
                safe_str[n++] = s[i++];
            }

            else if (c < 32 || c > 126) {
                // non printing characters
            }

            else {
                safe_str[n++] = c;
            }

            if (n > 2040)
            break;
        }
    }

    return safe_str;
}

// +--------------------------------------------------------------------+

const char* SafeQuotes(const char* msg)
{
    int dst = 0;

    if (msg) {
        while (*msg && dst < 254) {
            if (*msg == '"') {
                safe_str[dst++] = '\'';
                msg++;
            }
            else if (isspace(*msg)) {
                safe_str[dst++] = ' ';
                msg++;
            }
            else {
                safe_str[dst++] = *msg++;
            }
        }
    }

    safe_str[dst] = 0;
    return safe_str;
}

// +--------------------------------------------------------------------+

Text FormatTextReplace(const char* msg, const char* tgt, const char* val)
{
    if (!msg || !tgt || !val)
    return "";

    if (!strchr(msg, *tgt))
    return msg;

    Text        result;
    char*       buffer = new char[strlen(msg) + 1];
    const char* p      = msg;
    char*       q      = buffer;
    int         tgtlen = strlen(tgt);

    while (*p) {
        if (!strncmp(p, tgt, tgtlen)) {
            p += tgtlen;
            *q = 0;
            q = buffer;

            result += buffer;
            result += val;
        }

        else {
            *q++ = *p++;
        }
    }

    if (q != buffer) {
        *q = 0;
        result += buffer;
    }

    delete [] buffer;
    return result;
}

// +--------------------------------------------------------------------+

Text FormatTextEscape(const char* msg)
{
    if (!msg)
    return "";

    if (!strchr(msg, '\\'))
    return msg;

    Text        result;
    char*       buffer = new char[strlen(msg) + 1];
    const char* p      = msg;
    char*       q      = buffer;

    while (*p) {
        if (*p == '\\') {
            p++;

            if (*p == 'n') {
                *q++ = '\n';
                p++;
            }

            else if (*p == 't') {
                *q++ = '\t';
                p++;
            }

            else {
                *q++ = *p++;
            }
        }

        else {
            *q++ = *p++;
        }
    }

    *q = 0;
    result = buffer;
    delete [] buffer;
    return result;
}
