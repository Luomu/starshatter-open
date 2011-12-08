/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2006. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Locale.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    3D Locale (Polygon) Object
*/

#include "MemDebug.h"
#include "Locale.h"

void  Print(const char* fmt, ...);

// +--------------------------------------------------------------------+

static List<Locale>   locales;

// +--------------------------------------------------------------------+

Locale::Locale(const char* l, const char* c, const char* v)
{
   ZeroMemory(this, sizeof(Locale));
   if (l && *l) {
      strncpy_s(language, l, 6);
      char* p = language;
      while (*p) {
         *p = tolower(*p);
         p++;
      }
   }

   if (c && *c) {
      strncpy_s(country,  c, 6);
      char* p = country;
      while (*p) {
         *p = toupper(*p);
         p++;
      }
   }

   if (v && *v) {
      strncpy_s(variant,  v, 6);
      char* p = variant;
      while (*p) {
         *p = tolower(*p);
         p++;
      }
   }

   locales.append(this);
}

// +--------------------------------------------------------------------+

Locale::~Locale()
{
   locales.remove(this);
}

// +--------------------------------------------------------------------+

int
Locale::operator == (const Locale& that) const
{
   if (this == &that) return 1;

   return !_stricmp(language, that.language) &&
          !_stricmp(country,  that.country)  &&
          !_stricmp(variant,  that.variant);
}

// +--------------------------------------------------------------------+

Locale*
Locale::ParseLocale(const char* str)
{
   if (str && *str) {
      int   i = 0;
      char  s1[4];
      char  s2[4];
      char  s3[4];

      while (*str && *str != '_' && i < 3) {
         s1[i] = *str++;
         i++;
      }
      s1[i] = 0;
      i = 0;

      if (*str == '_')
         str++;

      while (*str && *str != '_' && i < 3) {
         s2[i] = *str++;
         i++;
      }
      s2[i] = 0;
      i = 0;

      if (*str == '_')
         str++;

      while (*str && *str != '_' && i < 3) {
         s3[i] = *str++;
         i++;
      }
      s3[i] = 0;
      i = 0;

      return CreateLocale(s1, s2, s3);
   }

   return 0;
}

// +--------------------------------------------------------------------+

Locale*
Locale::CreateLocale(const char* l, const char* c, const char* v)
{
   ListIter<Locale> iter = locales;
   while (++iter) {
      Locale* loc = iter.value();
      if (!_stricmp(l, loc->GetLanguage())) {
         if (c && *c) {
            if (!_stricmp(c, loc->GetCountry())) {
               if (v && *v) {
                  if (!_stricmp(v, loc->GetVariant())) {
                     return loc;
                  }
               }
               else {
                  return loc;
               }
            }
         }
         else {
            return loc;
         }
      }
   }

   if (l[0]) {
      if (c[0]) {
         if (v[0]) {
            return new(__FILE__,__LINE__) Locale(l, c, v);
         }
         return new(__FILE__,__LINE__) Locale(l, c);
      }
      return new(__FILE__,__LINE__) Locale(l);
   }

   return 0;
}

// +--------------------------------------------------------------------+

const List<Locale>& 
Locale::GetAllLocales()
{
   return locales;
}

// +--------------------------------------------------------------------+

const Text
Locale::GetFullCode() const
{
   Text result = language;
   if (*country) {
      result.append("_");
      result.append(country);

      if (*variant) {
         result.append("_");
         result.append(variant);
      }
   }
   return result;
}

// +--------------------------------------------------------------------+

static const char* languages[] = {
   "en", "English",
   "fr", "French",
   "de", "German",
   "it", "Italian",
   "pt", "Portuguese",
   "ru", "Russian",
   "es", "Spanish"
};

static const char* countries[] = {
   "US", "USA",
   "CA", "Canada",
   "FR", "France",
   "DE", "Germany",
   "IT", "Italy",
   "PT", "Portugal",
   "RU", "Russia",
   "ES", "Spain",
   "UK", "United Kingdom"
};

const Text
Locale::GetDisplayName() const
{
   Text result;
   if (*language) {
      for (int i = 0; i < 14; i += 2) {
         if (!_stricmp(language, languages[i])) {
            result = languages[i+1];
            break;
         }
      }

      if (*country) {
         for (int i = 0; i < 18; i += 2) {
            if (!_stricmp(country, countries[i])) {
               result.append(" - ");
               result.append(countries[i+1]);
               break;
            }
         }
      }

   }
   return result;
}

