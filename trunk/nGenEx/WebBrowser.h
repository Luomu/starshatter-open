/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2006. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         WebBrowser.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Helper class to find and launch user's default web browser
*/

#ifndef WebBrowser_h
#define WebBrowser_h

#include "Types.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class WebBrowser
{
public:
	static const char* TYPENAME() { return "WebBrowser"; }

	WebBrowser();
	virtual ~WebBrowser();

	virtual void OpenURL(const char* url);

protected:
	void FindDefaultBrowser();
	void FindOpenCommand();

	Text browser;
	Text command;
};

// +--------------------------------------------------------------------+

#endif WebBrowser_h


