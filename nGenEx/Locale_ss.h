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
    FILE:         Locale.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Description of locale by ISO language, country, and variant
*/

#ifndef Locale_h
#define Locale_h

#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Locale
{
public:
    static const char* TYPENAME() { return "Locale"; }

    Locale(const char* language, const char* country=0, const char* variant=0);
    ~Locale();

    int operator == (const Locale& that) const;

    // Operations:
    static const List<Locale>& GetAllLocales();
    static Locale*             ParseLocale(const char* str);

    // Property accessors:
    const char*       GetLanguage()  const { return language;   }
    const char*       GetCountry()   const { return country;    }
    const char*       GetVariant()   const { return variant;    }
    const Text        GetFullCode()  const;
    const Text        GetDisplayName() const;


protected:
    static Locale*    CreateLocale(const char* language, const char* country=0, const char* variant=0);
    char              language[8];
    char              country[8];
    char              variant[8];
};

#endif Locale_h

