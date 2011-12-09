/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2006. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         ContentBundle.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Chained collection of localized strings
*/

#ifndef ContentBundle_h
#define ContentBundle_h

#include "Types.h"
#include "Dictionary.h"
#include "Text.h"
#include "Locale.h"

// +--------------------------------------------------------------------+

class ContentBundle
{
public:
	static const char* TYPENAME() { return "ContentBundle"; }

	ContentBundle(const char* bundle, Locale* locale);
	virtual ~ContentBundle();

	int operator == (const ContentBundle& that)  const { return this == &that; }

	const Text&       GetName()                  const { return name; }
	Text              GetText(const char* key)   const;
	bool              IsLoaded()                 const { return !values.isEmpty(); }

protected:
	void LoadBundle(const char* filename);
	Text FindFile(const char* bundle, Locale* locale);

	Text              name;
	Dictionary<Text>  values;
};

#endif ContentBundle_h

