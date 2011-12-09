/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2006. All Rights Reserved.

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
