/*  Project nGen
    John DiCamillo
    Copyright © 1997-2002. All Rights Reserved.

    SUBSYSTEM:    nGen.lib
    FILE:         Types.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Basic Type Definitions
*/

#ifndef Types_h
#define Types_h

// +--------------------------------------------------------------------+

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define STRICT 1

// Works with Windows 2000 and later and Windows 98 or later
#undef _WIN32_IE
#undef WINVER
#undef _WIN32_WINDOWS
#undef _WIN32_WINNT
#define WINVER         0x0500 
#define _WIN32_WINDOWS 0x0410 
#define _WIN32_WINNT   0x0500 

//#if !defined(HMONITOR_DECLARED)
//    #define HMONITOR_DECLARED
//    DECLARE_HANDLE(HMONITOR);
//#endif

#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <math.h>      
#include <limits.h>      
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.  
// This makes D3D objects work well in the debugger watch window, but slows down 
// performance slightly.
#if defined(DEBUG) | defined(_DEBUG)
#define D3D_DEBUG_INFO
#endif

// Direct3D includes
#include <d3d9.h>
#include <d3dx9.h>

// DirectSound includes
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>

// +--------------------------------------------------------------------+

#endif Types_h

