/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2006. All Rights Reserved.

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

