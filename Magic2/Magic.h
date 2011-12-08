/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         Magic.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Interface of the main application class
*/


#if !defined(AFX_MAGIC_H__E61FB5C3_42B6_42D1_BEFA_17F0B038E33B__INCLUDED_)
#define AFX_MAGIC_H__E61FB5C3_42B6_42D1_BEFA_17F0B038E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


class Magic : public CWinApp
{
public:
	Magic();
	virtual ~Magic();

    bool AppActivated()          const { return app_active; }
    void SetAppActivated(bool a)       { app_active = a;    }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Magic)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(Magic)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   bool  app_active;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAGIC_H__E61FB5C3_42B6_42D1_BEFA_17F0B038E33B__INCLUDED_)
