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

    SUBSYSTEM:    FoundationEx
    FILE:         Text.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Declaration of the Text class
*/

#ifndef Text_h
#define Text_h

#include <string.h>
#include <windows.h>
#include "ThreadSync.h"

// +-------------------------------------------------------------------+

class TextRep
{
   friend class Text;

public:
   TextRep();
   ~TextRep();

private:
   TextRep(const char* s);
   TextRep(const char* s, int len);
   TextRep(char c, int len);
   TextRep(const TextRep* rep);
   
   void        addref();
   long        deref();

   void        dohash();

   char*       data;
   long        ref;
   int         length;
   unsigned    hash;
   bool        sensitive;
   
   static ThreadSync sync;
   static TextRep    nullrep;
};

// +-------------------------------------------------------------------+

class Text
{
public:
   static const char* TYPENAME() { return "Text"; }

   Text();
   Text(char c);
   Text(const char*  s);
   Text(const char*  s, int len);
   Text(char c, int len);
   Text(const Text&  s);
   ~Text();

   // case sensitivity
   bool  isSensitive()     const;
   void  setSensitive(bool s);

   // comparison
   int compare(const char* s) const;
   int compare(const Text& s) const;
   
   // assignment
   Text& operator=(const char*   s);
   Text& operator=(const Text&   s);
   
   // catenation
   Text& append(char c);
   Text& append(const char*      s);
   Text& append(const Text&      s);
   
   Text  operator+(char          c);
   Text  operator+(const char*   s);
   Text  operator+(const Text&   s);

   Text& operator+=(char         c) { return append(c); }
   Text& operator+=(const char*  s) { return append(s); }
   Text& operator+=(const Text&  s) { return append(s); }

   // indexing
   char     operator[](int index) const;
   char     operator()(int index) const;
   char&    operator[](int index);
   char&    operator()(int index);
   
   Text  operator()(int start, int len) const;

   // access
   int         length() const    { return rep->length; }
   unsigned    hash() const      { return rep->hash;   }

   const char* data() const      { return sym; }
   operator const char* () const { return sym; }
   
   bool        contains(char        c) const;
   bool        contains(const char* s) const;

   bool        containsAnyOf(const char* charSet) const;

   int         indexOf(char         c) const;
   int         indexOf(const char*  s) const;
   
   // mutation
   void        toLower();
   void        toUpper();

   // substring
   Text        substring(int start, int length);
   Text        trim();
   Text        replace(const char* pattern, const char* substitution);

private:
   void        clone();

   const char* sym;
   TextRep*    rep;
};

// +-------------------------------------------------------------------+

inline int Text::compare(const char* s) const
{
   if (rep->sensitive)
      return strcmp(sym, s);
   else
      return _stricmp(sym, s);
}

inline int Text::compare(const Text& s) const
{
   if (rep->sensitive && s.rep->sensitive)
      return strcmp(sym, s.sym);
   else
      return _stricmp(sym, s.sym);
}

// +-------------------------------------------------------------------+

inline int operator==(const Text& l, const Text& r) {
   return (l.length() == r.length()) && (l.compare(r) == 0); }
inline int operator!=(const Text& l, const Text& r) { return l.compare(r) != 0; }
inline int operator< (const Text& l, const Text& r) { return l.compare(r) <  0; }
inline int operator<=(const Text& l, const Text& r) { return l.compare(r) <= 0; }
inline int operator> (const Text& l, const Text& r) { return l.compare(r) >  0; }
inline int operator>=(const Text& l, const Text& r) { return l.compare(r) >= 0; }

inline int operator==(const char* l, const Text& r) { return r.compare(l) == 0; }
inline int operator!=(const char* l, const Text& r) { return r.compare(l) != 0; }
inline int operator< (const char* l, const Text& r) { return r.compare(l) <  0; }
inline int operator<=(const char* l, const Text& r) { return r.compare(l) <= 0; }
inline int operator> (const char* l, const Text& r) { return r.compare(l) >  0; }
inline int operator>=(const char* l, const Text& r) { return r.compare(l) >= 0; }

inline int operator==(      char* l, const Text& r) { return r.compare(l) == 0; }
inline int operator!=(      char* l, const Text& r) { return r.compare(l) != 0; }
inline int operator< (      char* l, const Text& r) { return r.compare(l) <  0; }
inline int operator<=(      char* l, const Text& r) { return r.compare(l) <= 0; }
inline int operator> (      char* l, const Text& r) { return r.compare(l) >  0; }
inline int operator>=(      char* l, const Text& r) { return r.compare(l) >= 0; }

inline int operator==(const Text& l, const char* r) { return l.compare(r) == 0; }
inline int operator!=(const Text& l, const char* r) { return l.compare(r) != 0; }
inline int operator< (const Text& l, const char* r) { return l.compare(r) <  0; }
inline int operator<=(const Text& l, const char* r) { return l.compare(r) <= 0; }
inline int operator> (const Text& l, const char* r) { return l.compare(r) >  0; }
inline int operator>=(const Text& l, const char* r) { return l.compare(r) >= 0; }

inline int operator==(const Text& l,       char* r) { return l.compare(r) == 0; }
inline int operator!=(const Text& l,       char* r) { return l.compare(r) != 0; }
inline int operator< (const Text& l,       char* r) { return l.compare(r) <  0; }
inline int operator<=(const Text& l,       char* r) { return l.compare(r) <= 0; }
inline int operator> (const Text& l,       char* r) { return l.compare(r) >  0; }
inline int operator>=(const Text& l,       char* r) { return l.compare(r) >= 0; }

inline Text operator+(const char* l, const Text& r) { return Text(l) + r; }
inline Text operator+(      char* l, const Text& r) { return Text(l) + r; }

// +-------------------------------------------------------------------+

#endif Text_h
