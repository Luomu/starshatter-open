/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Authorization.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Authorization Sprite animation class
*/

#include "MemDebug.h"
#include "Authorization.h"

#include "Game.h"
#include "Text.h"

// +--------------------------------------------------------------------+

static Text GetCDKeyFromRegistry()
{
	Text  cdkey;
	BYTE  cdbuf[64];
	DWORD cdlen = 0;
	HKEY  hkey  = 0;

	ZeroMemory(cdbuf, sizeof(cdbuf));

	RegOpenKeyEx(HKEY_LOCAL_MACHINE,
	"SOFTWARE\\Matrix Games\\Starshatter",
	0,
	KEY_QUERY_VALUE,
	&hkey);

	if (hkey) {
		cdlen  = 64;

		LONG result =
		RegQueryValueEx(hkey,
		"authorized",
		NULL,
		NULL,
		cdbuf,
		&cdlen);

		if (result == ERROR_SUCCESS && cdlen > 0)
		cdkey = (const char*) cdbuf;

		RegCloseKey(hkey);
	}

	return cdkey;
}

static Text GetCDKeyFromIniFile()
{
	Text cdkey;
	char cdbuf[256];

	ZeroMemory(cdbuf, sizeof(cdbuf));

	FILE* f = fopen("maga.mg", "r");
	if (f) {
		bool found_section = false;

		while (fgets(cdbuf, sizeof(cdbuf)-1, f)) {
			Text line = Text(cdbuf).trim();
			line.setSensitive(false);

			if (line == "[SerialNumber]")
			found_section = true;

			if (found_section) {
				if (line.indexOf("serial") == 0) {
					// found the proper line in the proper section,
					// now we need to parse the 'name = value' sentence:

					const char* p = line.data();

					// find the equal sign:
					while (p && *p && *p != '=')
					p++;

					// skip the equal sign:
					p++;

					// find the string after the equal sign:
					while (p && *p && isspace(*p))
					p++;

					if (p && *p) {
						// deal with quoted strings:
						int cutoff = sizeof(cdbuf)-1;

						if (*p == '"') {
							char* s = cdbuf;
							p++;

							while (*p && *p != '"' && cutoff-- > 0) {
								*s++ = *p++;
							}

							*s = 0;
						}

						// and unquoted strings:
						else {
							char* s = cdbuf;

							while (*p && cutoff-- > 0) {
								*s++ = *p++;
							}

							*s = 0;
						}

						cdkey = cdbuf;
					}
				}
			}
		}

		fclose(f);
	}

	return cdkey;
}

// +--------------------------------------------------------------------+

static char serial_number[64];
int execRegistrationProgram();

bool
Authorization::IsUserAuthorized()
{
	// XXX DEBUG ONLY!
	// return true;

	int authcode = execRegistrationProgram();
	if (authcode != 1) {
		::Print("Authorization failed, code = %d\n", authcode);
	}

	return (authcode == 1);
}

// +--------------------------------------------------------------------+

const char*
Authorization::GetSerialNumber()
{
	return serial_number;
}

// +--------------------------------------------------------------------+

int execRegistrationProgram()
{
	int  result = 999;
	char cmdline[256];
	strcpy(cmdline, "SS2rez");

	STARTUPINFO s;
	ZeroMemory(&s, sizeof(s));
	s.cb = sizeof(s);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcess("SS2rez.exe", cmdline, 0, 0, 0, 0, 0, 0, &s, &pi)) {
		DWORD  exitcode = STILL_ACTIVE;

		WaitForSingleObject(pi.hProcess, 20000);
		GetExitCodeProcess(pi.hProcess, &exitcode);

		if (exitcode != STILL_ACTIVE)
		result = exitcode;

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		TCHAR message[256];
		DWORD errcode = GetLastError();

		::Print("  WARN: Failed to create authorization process: %08X.\n", errcode);

		if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, errcode, 0, message, 256, 0)) {
			::Print("        ");
			::Print(message);
			::Print("\n");
		}
	}

	return result;
}


