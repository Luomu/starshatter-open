// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__E37C2030_8F5E_4A38_92DD_72F0EAE0DA81__INCLUDED_)
#define AFX_STDAFX_H__E37C2030_8F5E_4A38_92DD_72F0EAE0DA81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

#if _MSC_VER <= 1600
#define _WIN32_WINNT 0x0502 // XP is the lowest supported OS
#else
#define _WIN32_WINNT 0x0600 // VS2012 does not support XP, 0x0600 is Vista
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>        // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>            // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.  
// This makes D3D objects work well in the debugger watch window, but slows down 
// performance slightly.
#if defined(DEBUG) | defined(_DEBUG)
#define D3D_DEBUG_INFO
#endif

// Direct3D includes
#include <d3d9.h>
#include <d3dx9.h>
#include <mmsystem.h>
#include <mmreg.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E37C2030_8F5E_4A38_92DD_72F0EAE0DA81__INCLUDED_)
