/*  Project FoundationEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    FoundationEx
    FILE:         text.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Implementation of the Text class
*/

#include "MemDebug.h"
#include "Text.h"
#include "stdio.h"
#include <ctype.h>

// +-------------------------------------------------------------------+
// SPECIAL TEXT REP FOR NULL STRINGS
// This is used only by the default constructor for the Text object,
// to prevent extra rep allocation when constructing empty strings.

TextRep        TextRep::nullrep;

TextRep::TextRep() 
   : ref(1234567), data(0), length(0), hash(0), sensitive(true)
{
#ifdef MEM_DEBUG
   data = new(__FILE__,__LINE__) char[4];
#else
   data = new char[4];
#endif

   if (data)
      ZeroMemory(data, 4);
}

// +-------------------------------------------------------------------+

ThreadSync TextRep::sync;


TextRep::TextRep(const char* s)
   : ref(1), length(0), sensitive(true)
{
   if (s) length = ::strlen(s);

#ifdef MEM_DEBUG
   data = new(__FILE__,__LINE__) char[length+1];
#else
   data = new char[length+1];
#endif

   if (data) {
      if (s) ::strcpy(data, s);
      else   data[length] = '\0';

      dohash();
   }
}

TextRep::TextRep(const char* s, int len)
   : ref(1), length(len), sensitive(true)
{
   if (length < 0) length = 0;

#ifdef MEM_DEBUG
   data = new(__FILE__,__LINE__) char[length+1];
#else
   data = new char[length+1];
#endif

   if (data) {
      ::CopyMemory(data, s, length);
      data[length] = '\0';
      dohash();
   }
}

TextRep::TextRep(char c, int len)
   : ref(1), length(len), sensitive(true)
{
   if (length < 0) length = 0;

#ifdef MEM_DEBUG
   data = new(__FILE__,__LINE__) char[length+1];
#else
   data = new char[length+1];
#endif

   if (data) {
      ::FillMemory(data, length, c);
      data[length] = '\0';
      dohash();
   }
}

TextRep::TextRep(const TextRep* rep)
   : ref(1)
{
   length    = rep->length;

#ifdef MEM_DEBUG
   data = new(__FILE__,__LINE__) char[length+1];
#else
   data = new char[length+1];
#endif

   hash      = rep->hash;
   sensitive = rep->sensitive;

   if (data)
      ::strcpy(data, rep->data);
}

TextRep::~TextRep()
{
   delete[] data;
}

void
TextRep::addref()
{
   sync.acquire();
   ref++;
   sync.release();
}

long
TextRep::deref()
{
   sync.acquire();
   long r = --ref;
   sync.release();
   return r;
}

inline static void mash(unsigned& hash, unsigned chars)
{
  hash = (chars ^ ((hash << 5) | (hash >> (8*sizeof(unsigned) - 5))));
}

void
TextRep::dohash()
{
   unsigned hv       = (unsigned)length; // Mix in the string length.
   unsigned i        = length*sizeof(char)/sizeof(unsigned);
   const unsigned* p = (const unsigned*)data;
   
   while (i--)
      mash(hv, *p++);			// XOR in the characters.

   // XOR in any remaining characters:
   i                 = length*sizeof(char)%sizeof(unsigned);
   if (i) {
      unsigned h = 0;
      const char* c = (const char*)p;
      while (i--) 
         h = ((h << 8*sizeof(char)) | *c++);
      mash(hv, h);
   }

   hash = hv;
}

// +-------------------------------------------------------------------+

Text::Text()
{
   rep = &TextRep::nullrep;
   rep->addref();
   sym = rep->data;
}

Text::Text(char c)
{
   char buf[2]; buf[0] = c; buf[1] = '\0';

#ifdef MEM_DEBUG
   rep = new(__FILE__,__LINE__) TextRep(buf);
#else
   rep = new TextRep(buf);
#endif

   if (!rep) {
      rep = &TextRep::nullrep;
      rep->addref();
   }

   sym = rep->data;
}

Text::Text(const char* s)
{
#ifdef MEM_DEBUG
   rep = new(__FILE__,__LINE__) TextRep(s);
#else
   rep = new TextRep(s);
#endif

   if (!rep) {
      rep = &TextRep::nullrep;
      rep->addref();
   }

   sym = rep->data;
}

Text::Text(const char* s, int len)
{
#ifdef MEM_DEBUG
   rep = new(__FILE__,__LINE__) TextRep(s, len);
#else
   rep = new TextRep(s, len);
#endif

   if (!rep) {
      rep = &TextRep::nullrep;
      rep->addref();
   }

   sym = rep->data;
}

Text::Text(char c, int len)
{
#ifdef MEM_DEBUG
   rep = new(__FILE__,__LINE__) TextRep(c, len);
#else
   rep = new TextRep(c, len);
#endif

   if (!rep) {
      rep = &TextRep::nullrep;
      rep->addref();
   }

   sym = rep->data;
}

Text::Text(const Text& s)
{
   rep = s.rep;
   rep->addref();
   sym = rep->data;
}

Text::~Text()
{
   if (rep->deref() == 0) delete rep;

   rep = &TextRep::nullrep;
   sym = rep->data;
}

Text&
Text::operator=(const char* s)
{
   if (rep->deref() == 0) delete rep;
#ifdef MEM_DEBUG
   rep = new(__FILE__,__LINE__) TextRep(s);
#else
   rep = new TextRep(s);
#endif

   if (!rep)
      rep = &TextRep::nullrep;
   sym = rep->data;
   return *this;
}

Text&
Text::operator=(const Text& s)
{
   s.rep->addref();
   if (rep->deref() == 0) delete rep;
   rep = s.rep;
   sym = rep->data;
   return *this;
}

Text
Text::operator+(char c)
{
#ifdef MEM_DEBUG
   char* buf = new(__FILE__,__LINE__) char[rep->length + 2];
#else
   char* buf = new char[rep->length + 2];
#endif

   if (buf) {
      ::strcpy(buf, sym);
      buf[rep->length] = c;
      buf[rep->length+1] = '\0';
      Text retval(buf);
      delete [] buf;
      return retval;
   }

   else {
      return *this;
   }
}

Text
Text::operator+(const char* s)
{
#ifdef MEM_DEBUG
   char* buf = new(__FILE__,__LINE__) char[::strlen(s) + rep->length + 1];
#else
   char* buf = new char[::strlen(s) + rep->length + 1];
#endif

   if (buf) {
      ::strcpy(buf, sym);
      ::strcat(buf, s);
      Text retval(buf);
      delete [] buf;
      return retval;
   }

   else {
      return *this;
   }
}

Text
Text::operator+(const Text& s)
{
#ifdef MEM_DEBUG
   char* buf = new(__FILE__,__LINE__) char[s.rep->length + rep->length + 1];
#else
   char* buf = new char[s.rep->length + rep->length + 1];
#endif

   if (buf) {
      ::strcpy(buf, sym);
      ::strcat(buf, s.sym);
      Text retval(buf);
      delete [] buf;
      return retval;
   }

   else {
      return *this;
   }
}

bool
Text::isSensitive() const
{
   return rep->sensitive;
}

void
Text::setSensitive(bool s)
{
   rep->sensitive = s;
}

Text&
Text::append(char c)
{
#ifdef MEM_DEBUG
   char* buf = new(__FILE__,__LINE__) char[rep->length + 2];
#else
   char* buf = new char[rep->length + 2];
#endif

   if (buf) {
      ::strcpy(buf, sym);
      buf[rep->length] = c;
      buf[rep->length+1] = '\0';
      if (rep->deref() == 0) delete rep;

#ifdef MEM_DEBUG
      rep = new(__FILE__,__LINE__) TextRep(buf);
#else
      rep = new TextRep(buf);
#endif

      if (!rep)
         rep = &TextRep::nullrep;

      sym = rep->data;
      delete [] buf;
   }

   return *this;
}

Text&
Text::append(const char* s)
{
#ifdef MEM_DEBUG
   char* buf = new(__FILE__,__LINE__) char[::strlen(s) + rep->length + 1];
#else
   char* buf = new char[::strlen(s) + rep->length + 1];
#endif

   if (buf) {
      ::strcpy(buf, sym);
      ::strcat(buf, s);
      if (rep->deref() == 0) delete rep;

#ifdef MEM_DEBUG
      rep = new(__FILE__,__LINE__) TextRep(buf);
#else
      rep = new TextRep(buf);
#endif

      if (!rep)
         rep = &TextRep::nullrep;

      sym = rep->data;
      delete [] buf;
   }

   return *this;
}

Text&
Text::append(const Text& s)
{
#ifdef MEM_DEBUG
   char* buf = new(__FILE__,__LINE__) char[s.rep->length + rep->length + 1];
#else
   char* buf = new char[s.rep->length + rep->length + 1];
#endif

   if (buf) {
      int lenA = rep->length;
      int lenB = s.rep->length;

      CopyMemory(buf,        sym,   lenA);
      CopyMemory(buf + lenA, s.sym, lenB);
      buf[lenA + lenB] = 0;

      if (rep->deref() == 0) delete rep;

#ifdef MEM_DEBUG
      rep = new(__FILE__,__LINE__) TextRep(buf, lenA + lenB);
#else
      rep = new TextRep(buf);
#endif

      if (!rep)
         rep = &TextRep::nullrep;

      sym = rep->data;
      delete [] buf;
   }

   return *this;
}

void
Text::clone()
{
   if (rep->ref > 1) {
      rep->deref();

#ifdef MEM_DEBUG
      TextRep* t = new(__FILE__,__LINE__) TextRep(rep);
#else
      TextRep* t = new TextRep(rep);
#endif

      rep = t;

      if (!rep)
         rep = &TextRep::nullrep;

      sym = rep->data;
   }
}

char
Text::operator[](int index) const
{
   if (index < (int) rep->length)
      return sym[index];
   else
      throw "BOUNDS ERROR";

   return '\0';
}

char
Text::operator()(int index) const
{
   return sym[index];
}

char&
Text::operator[](int index)
{
   if (index < (int) rep->length) {
      clone();
      return (char&) sym[index];
   }
   else
      throw "BOUNDS ERROR";

   return (char&) sym[0];
}

char&
Text::operator()(int index)
{
   clone();
   return (char&) sym[index];
}

Text
Text::operator()(int start, int len) const
{
   if (start > rep->length || len <= 0)
      return Text();

   if (start + len > rep->length)
      len = rep->length - start;

#ifdef MEM_DEBUG   
   char* buf = new(__FILE__,__LINE__) char[len+1];
#else
   char* buf = new char[len+1];
#endif

   if (buf) {
      ::strncpy(buf, sym+start, len);
      buf[len] = '\0';

      Text retval(buf);
      delete [] buf;
      return retval;
   }

   return Text();
}

bool
Text::contains(char c) const
{
   if (rep->length > 0) {
      if (!rep->sensitive) {
         char alt = c;
         if (islower(alt))      alt = toupper(alt);
         else if (isupper(alt)) alt = tolower(alt);

         if (strchr(rep->data, alt) != 0)
            return true;
      }

      if (strchr(rep->data, c) != 0)
         return true;
   }

   return false;
}

bool
Text::contains(const char* pattern) const
{
   if (rep->length > 0 && pattern && *pattern) {
      if (rep->sensitive) {
         if (strstr(rep->data, pattern) != 0)
            return true;
      }
      else {
         Text smash1(*this);
         smash1.toLower();
         Text smash2(pattern);
         smash2.toLower();

         if (strstr(smash1.data(), smash2.data()) != 0)
            return true;
      }
   }

   return false;
}

bool
Text::containsAnyOf(const char* charSet) const
{
   if (rep->length > 0 && charSet && *charSet) {
      if (rep->sensitive) {
         if (strpbrk(rep->data, charSet) != 0)
            return true;
      }
      else {
         Text smash1(*this);
         smash1.toLower();
         Text smash2(charSet);
         smash2.toLower();

         if (strpbrk(smash1.data(), smash2.data()) != 0)
            return true;
      }
   }

   return false;
}

int
Text::indexOf(char c) const
{
   if (rep->length > 0) {
      if (!rep->sensitive) {
         char alt = c;
         if (islower(alt))      alt = toupper(alt);
         else if (isupper(alt)) alt = tolower(alt);

         const char* p = strchr(rep->data, alt);

         if (p)
            return (p - rep->data);
      }

      const char* p = strchr(rep->data, c);

      if (p)
         return (p - rep->data);
   }

   return -1;
}

int
Text::indexOf(const char* pattern) const
{
   if (rep->length > 0 && pattern && *pattern) {
      if (rep->sensitive) {
         const char* p = strstr(rep->data, pattern);
         if (p) return (p - rep->data);
      }
      else {
         Text smash1(*this);
         smash1.toLower();
         Text smash2(pattern);
         smash2.toLower();

         const char* p = strstr(smash1.data(), smash2.data());
         if (p) return (p - smash1.data());
      }
   }

   return -1;
}

void
Text::toLower()
{
   clone();
   size_t n = rep->length;
   char* p = (char*) sym;
   while (n--) {
      *p = tolower((unsigned char)*p);
      p++;
   }
   
   rep->dohash();
}

void
Text::toUpper()
{
   clone();
   size_t n = rep->length;
   char* p = (char*) sym;
   while ( n-- ) {
      *p = toupper((unsigned char)*p);
      p++;
   }
   
   rep->dohash();
}

Text
Text::substring(int start, int length)
{
   Text result;

   if (start >= 0 && start < (int) rep->length && length > 0) {
      if (start + length > (int) rep->length)
         length = (int) rep->length - start;

      const char* s = sym + start;

#ifdef MEM_DEBUG
      result.rep = new(__FILE__,__LINE__) TextRep(s, length);
#else
      result.rep = new TextRep(s, length);
#endif

      if (!result.rep)
         result.rep = &TextRep::nullrep;

      result.sym = result.rep->data;
   }

   return result;
}

Text
Text::trim()
{
   Text result;

   if (rep->length) {
      const char* p = sym;
      const char* q = sym + rep->length-1;

      while (p && *p && isspace(*p))   p++;
      while (q>p && *q && isspace(*q)) q--;

      result = substring(p-sym, q-p+1);
   }

   return result;
}

Text
Text::replace(const char* pattern, const char* substitution)
{
   Text result;

   if (rep->length && pattern && *pattern) {
      int index = 0;
      int skip  = strlen(pattern);
      do {
         const char* p = strstr(rep->data + index, pattern);
         if (p) {
            int len = (p - rep->data + index);
            result.append(substring(index, len));
            result.append(substitution);
            index += len + skip;
         }
         else if (index < rep->length) {
            result.append(substring(index, rep->length-index));
            index = -1;
         }
      }
      while (index >= 0 && index < rep->length);
   }

   return result;
}
