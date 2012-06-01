/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         MachineInfo.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Collect and Display Machine, OS, and Driver Information
*/

#ifndef MachineInfo_h
#define MachineInfo_h

#include "Types.h"

// +--------------------------------------------------------------------+

class MachineInfo
{
public:
	enum { CPU_INVALID, CPU_P5=5, CPU_P6=6, CPU_P7=7, CPU_PLUS };
	enum { OS_INVALID,  OS_WIN95, OS_WIN98, OS_WINNT, OS_WIN2K, OS_WINXP, OS_WINXP64, OS_WINVISTA, OS_WINSEVEN, OS_WINFUTURE };
	enum { DX_NONE,     DX_3=3,   DX_5=5,   DX_6=6,   DX_7=7,   DX_8=8, DX_9=9, DX_PLUS };

	static int     GetCpuClass();
	static int     GetCpuSpeed();
	static int     GetTotalRam();
	static int     GetPlatform();
	static int     GetDirectXVersion();

	static void    DescribeMachine();

	static const char* GetShortDescription();
};

// +--------------------------------------------------------------------+

#endif MachineInfo_h