/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         TextureMapDialog.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Texture Mapping Dialog interface file
*/

#if !defined(AFX_TEXTUREMAPDIALOG_H__F8EDA550_FF19_4E91_9A9C_597FC357C563__INCLUDED_)
#define AFX_TEXTUREMAPDIALOG_H__F8EDA550_FF19_4E91_9A9C_597FC357C563__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

// +--------------------------------------------------------------------+

class  Bitmap;
class  MagicDoc;
class  MagicView;
struct Material;
class  Model;

// +--------------------------------------------------------------------+
// TextureMapDialog dialog
// +--------------------------------------------------------------------+

class TextureMapDialog : public CDialog
{
// Construction
public:
	TextureMapDialog(MagicView* pParent = NULL);
   virtual ~TextureMapDialog();

// Dialog Data
	//{{AFX_DATA(TextureMapDialog)
	enum { IDD = IDD_MODIFY_TEXTURE };
	CComboBox	mMapping;
	CComboBox	mMaterialList;
	CStatic	mMaterialThumb;
	int		mMaterialIndex;
	BOOL	mFlip;
	BOOL	mMirror;
	BOOL	mRotate;
	double	mScaleV;
	int		mAxis;
	double	mScaleU;
	int		mMapType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TextureMapDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   MagicDoc*   doc;
   Material*   material;
   Model*      model;
   Bitmap*     blank;

	// Generated message map functions
	//{{AFX_MSG(TextureMapDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSelectMaterial();
	afx_msg void OnAlign();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
