/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         FontMgr.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Font Resource Manager class implementation
*/

#include "MemDebug.h"
#include "FontMgr.h"

// +--------------------------------------------------------------------+

List<FontItem> FontMgr::fonts;

// +--------------------------------------------------------------------+

void
FontMgr::Close()
{
	fonts.destroy();
}

// +--------------------------------------------------------------------+

void
FontMgr::Register(const char* name, Font* font)
{
	FontItem* item = new(__FILE__,__LINE__) FontItem;

	if (item) {
		item->name = name;
		item->size = 0;
		item->font = font;

		fonts.append(item);
	}
}

// +--------------------------------------------------------------------+

Font*
FontMgr::Find(const char* name)
{
	ListIter<FontItem> item = fonts;
	while (++item) {
		if (item->name == name)
		return item->font;
	}

	return 0;
}
