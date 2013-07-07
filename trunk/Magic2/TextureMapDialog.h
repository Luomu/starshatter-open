/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
    CComboBox    mMapping;
    CComboBox    mMaterialList;
    CStatic    mMaterialThumb;
    int        mMaterialIndex;
    BOOL    mFlip;
    BOOL    mMirror;
    BOOL    mRotate;
    double    mScaleV;
    int        mAxis;
    double    mScaleU;
    int        mMapType;
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
