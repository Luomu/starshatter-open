/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2005. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         SurfacePropertiesDialog.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Surface Properties Dialog interface file
*/

#if !defined(AFX_SURFACEPROPERTIESDIALOG_H__8121A894_106E_4A17_9CE1_ADDD88F6A0CD__INCLUDED_)
#define AFX_SURFACEPROPERTIESDIALOG_H__8121A894_106E_4A17_9CE1_ADDD88F6A0CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// +--------------------------------------------------------------------+

class  Bitmap;
class  MagicDoc;
class  MagicView;
class  Model;

// +--------------------------------------------------------------------+
// SurfacePropertiesDialog dialog
// +--------------------------------------------------------------------+

class SurfacePropertiesDialog : public CDialog
{
// Construction
public:
	SurfacePropertiesDialog(MagicView* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SurfacePropertiesDialog)
	enum { IDD = IDD_SURFACE_PROPS };
	CString	mSurfaceName;
	CString	mNumPolys;
	CString	mNumVerts;
	CString	mSurfaceHeight;
	CString	mSurfaceLength;
	CString	mSurfaceRadius;
	CString	mSurfaceWidth;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SurfacePropertiesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   MagicDoc*   doc;

	// Generated message map functions
	//{{AFX_MSG(SurfacePropertiesDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
