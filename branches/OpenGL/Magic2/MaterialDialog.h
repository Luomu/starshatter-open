/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         MaterialDialog.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Material Editor Dialog interface file
*/

#if !defined(AFX_MATERIALDIALOG_H__DD978D83_EB03_479B_92A4_D1FCC333BECA__INCLUDED_)
#define AFX_MATERIALDIALOG_H__DD978D83_EB03_479B_92A4_D1FCC333BECA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

// +--------------------------------------------------------------------+

class  MagicDoc;
class  MagicView;
struct Material;

// +--------------------------------------------------------------------+
// MaterialDialog dialog
// +--------------------------------------------------------------------+

class MaterialDialog : public CDialog
{
// Construction
public:
	MaterialDialog(MagicView* pParent = NULL);
   virtual ~MaterialDialog();

// Dialog Data
	//{{AFX_DATA(MaterialDialog)
	enum { IDD = IDD_MODIFY_MATERIAL };
	CComboBox	mShadowList;
	CComboBox	mBlendModeList;
	CListBox	mMaterialList;
	CStatic	mMaterialThumb;
	CStatic	mSpecularColor;
	CStatic	mEmissiveColor;
	CStatic	mDiffuseColor;
	CStatic	mAmbientColor;
	float 	mAmbientValue;
	float 	mBrillianceValue;
	float 	mBumpValue;
	float 	mDiffuseValue;
	float 	mEmissiveValue;
	CString	mMaterialName;
	float 	mPowerValue;
	float 	mSpecularValue;
	CString	mSpecularTexture;
	CString	mDiffuseTexture;
	CString	mBumpTexture;
	CString	mEmissiveTexture;
	CString	mMaterialShader;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MaterialDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   void        UpdateMaterial();
   void        OnFileTexture(int type);
   void        ChangeFileTexture(char* fname, int type);

   MagicDoc*   doc;
   Solid*      solid;
   Material*   material;

	// Generated message map functions
	//{{AFX_MSG(MaterialDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnAmbientColor();
	afx_msg void OnDiffuseColor();
	afx_msg void OnEmissiveColor();
	afx_msg void OnSpecularColor();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnChangeMaterialValue();
	afx_msg void OnFileDiffuse();
	afx_msg void OnFileSpecular();
	afx_msg void OnFileEmissive();
	afx_msg void OnFileBump();
	afx_msg void OnChangeDiffuseTexture();
	afx_msg void OnChangeSpecularTexture();
	afx_msg void OnChangeEmissiveTexture();
	afx_msg void OnChangeBumpTexture();
	afx_msg void OnChangeMaterialName();
	afx_msg void OnSelectMaterial();
	afx_msg void OnSelectPolys();
	afx_msg void OnNewMaterial();
	afx_msg void OnDelMaterial();
	afx_msg void OnSelectBlendMode();
	afx_msg void OnSelectShadow();
	virtual void OnOK();
	afx_msg void OnChangeMaterialShader();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
