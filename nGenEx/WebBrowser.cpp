/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2006. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         WebBrowser.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Helper class to find and launch user's default web browser
*/

#include "MemDebug.h"
#include "WebBrowser.h"

// +--------------------------------------------------------------------+

WebBrowser::WebBrowser()
{
	FindDefaultBrowser();
	FindOpenCommand();
}

WebBrowser::~WebBrowser()
{
}

// +--------------------------------------------------------------------+


void
WebBrowser::OpenURL(const char* url)
{
	if (url) {
		char cmdline[256];

		if (command.contains("%1")) {
			strcpy_s(cmdline, command.replace("%1", url).data());
		}
		else {
			strcpy_s(cmdline, Text(command + " " + url).data());
		}

		STARTUPINFO s;
		ZeroMemory(&s, sizeof(s));
		s.cb = sizeof(s);

		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));

		if (CreateProcess(NULL, cmdline, 0, 0, 0, 0, 0, 0, &s, &pi)) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else {
			::Print("Unable to launch web browser for url '%s'\n", url);
		}
	}
}

// +--------------------------------------------------------------------+

void 
WebBrowser::FindDefaultBrowser()
{
	HKEY  hkey;
	char  value[256] = "";
	DWORD dwSize;

	ZeroMemory(value, 256);

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT,
				".html",
				0,
				KEY_READ,
				&hkey) == ERROR_SUCCESS) {

		dwSize = 256;
		RegQueryValueEx(hkey,
		"",
		NULL,
		NULL,
		(LPBYTE) value,
		&dwSize);

		RegCloseKey(hkey);

		if (dwSize > 0) {
			::Print("Default Web Browser: %s\n", value);
			browser = value;
		}
	}
}

// +--------------------------------------------------------------------+

void
WebBrowser::FindOpenCommand()
{
	HKEY  hkey;
	char  value[256] = "";
	DWORD dwSize;

	ZeroMemory(value, 256);

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT,
				browser + "\\shell\\open\\command",
				0,
				KEY_READ,
				&hkey) == ERROR_SUCCESS) {

		dwSize = 256;
		RegQueryValueEx(hkey,
		"",
		NULL,
		NULL,
		(LPBYTE) value,
		&dwSize);

		RegCloseKey(hkey);

		if (dwSize > 0) {
			::Print("Browser Shell Open Command: %s\n", value);
			command = value;
		}
	}
}

// +--------------------------------------------------------------------+


