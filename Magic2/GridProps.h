/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         GridProps.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Grid Properties Dialog interface file
*/

#if !defined(AFX_GRIDPROPS_H__79C78890_ABB9_4789_BFFC_29617CAC606E__INCLUDED_)
#define AFX_GRIDPROPS_H__79C78890_ABB9_4789_BFFC_29617CAC606E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

// +--------------------------------------------------------------------+
// GridProps dialog
// +--------------------------------------------------------------------+

class Grid;

class GridProps : public CDialog
{
// Construction
public:
	GridProps(Grid* g, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(GridProps)
	enum { IDD = IDD_GRIDPROPS };
	BOOL	mGridShow;
	BOOL	mGridSnap;
	CString	mReferencePlan;
	CString	mReferenceFront;
	CString	mReferenceSide;
	int		mGridSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GridProps)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   Grid* grid;

	// Generated message map functions
	//{{AFX_MSG(GridProps)
	afx_msg void OnFilePlan();
	afx_msg void OnFileFront();
	afx_msg void OnFileSide();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
