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
    FILE:         FormatUtil.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Text formatting utilities
*/

#ifndef FormatUtil_h
#define FormatUtil_h

#include "Types.h"
#include "Geometry.h"
#include "Text.h"

// +--------------------------------------------------------------------+

void FormatNumber(char* txt, double n);
void FormatNumberExp(char* txt, double n);
void FormatTime(char* txt, double seconds);
void FormatTimeOfDay(char* txt, double seconds);
void FormatDayTime(char* txt, double seconds, bool short_format=false);
void FormatDay(char* txt, double seconds);
void FormatPoint(char* txt, const Point& p);
Text FormatTimeString(int utc=0);

const char* SafeString(const char* s);
const char* SafeQuotes(const char* s);

// scan msg and replace all occurrences of tgt with val
// return new result, leave msg unmodified
Text FormatTextReplace(const char* msg, const char* tgt, const char* val);

// scan msg and replace all C-style \x escape sequences
// with their single-character values, leave orig unmodified
Text FormatTextEscape(const char* msg);

// +--------------------------------------------------------------------+

#endif FormatUtil_h

