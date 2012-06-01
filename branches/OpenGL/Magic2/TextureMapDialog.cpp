/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         TextureMapDialog.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Texture Mapping Dialog implementation file
*/


#include "stdafx.h"
#include "Magic.h"
#include "MagicDoc.h"
#include "MagicView.h"
#include "Selection.h"
#include "TextureMapDialog.h"
#include "Thumbnail.h"

#include "Bitmap.h"
#include "Solid.h"
#include "Polygon.h"
#include "Pcx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// +--------------------------------------------------------------------+
// TextureMapDialog dialog
// +--------------------------------------------------------------------+

TextureMapDialog::TextureMapDialog(MagicView* pParent)
	: CDialog(TextureMapDialog::IDD, pParent), doc(0), material(0), model(0), blank(0)
{
	//{{AFX_DATA_INIT(TextureMapDialog)
	mMaterialIndex = -1;
	mFlip = FALSE;
	mMirror = FALSE;
	mRotate = FALSE;
	mScaleV = 0.0;
	mAxis = -1;
	mScaleU = 0.0;
	mMapType = -1;
	//}}AFX_DATA_INIT

   Color gray = Color::LightGray;
   blank = new Bitmap(1,1,&gray);
   blank->ScaleTo(128,128);

   if (pParent) {
      doc = pParent->GetDocument();

      if (doc && doc->GetSolid()) {
         model = doc->GetSolid()->GetModel();
      }

      if (doc && doc->GetSelection()) {
         Selection* seln = doc->GetSelection();

         if (seln->GetPolys().size() > 0) {
            material = seln->GetPolys().first()->material;
            mMaterialIndex = model->GetMaterials().index(material) + 1;
         }
      }
   }
}

TextureMapDialog::~TextureMapDialog()
{
   delete blank;
}

void TextureMapDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TextureMapDialog)
	DDX_Control(pDX, IDC_MAPPING, mMapping);
	DDX_Control(pDX, IDC_MATERIAL, mMaterialList);
	DDX_Control(pDX, IDC_TEXTURE_PREVIEW, mMaterialThumb);
	DDX_CBIndex(pDX, IDC_MATERIAL, mMaterialIndex);
	DDX_Check(pDX, IDC_ALIGN_FLIP, mFlip);
	DDX_Check(pDX, IDC_ALIGN_MIRROR, mMirror);
	DDX_Check(pDX, IDC_ALIGN_ROTATE, mRotate);
	DDX_Text(pDX, IDC_SCALE_V, mScaleV);
	DDX_Radio(pDX, IDC_ALIGN_X, mAxis);
	DDX_Text(pDX, IDC_SCALE_U, mScaleU);
	DDX_CBIndex(pDX, IDC_MAPPING, mMapType);
	//}}AFX_DATA_MAP

   if (pDX->m_bSaveAndValidate) {
      mMaterialIndex = mMaterialList.GetCurSel()-1;
   }
}


BEGIN_MESSAGE_MAP(TextureMapDialog, CDialog)
	//{{AFX_MSG_MAP(TextureMapDialog)
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_MATERIAL, OnSelectMaterial)
	ON_BN_CLICKED(IDC_ALIGN_X, OnAlign)
	ON_BN_CLICKED(IDC_ALIGN_Y, OnAlign)
	ON_BN_CLICKED(IDC_ALIGN_Z, OnAlign)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// +--------------------------------------------------------------------+
// TextureMapDialog message handlers
// +--------------------------------------------------------------------+

BOOL TextureMapDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

   mMaterialList.AddString("<none>");

   if (model && model->NumMaterials()) {
      ListIter<Material> iter = model->GetMaterials();
      while (++iter) {
         Material* mtl = iter.value();
         mMaterialList.AddString(mtl->name);
      }
   }

   mMaterialList.SetCurSel(mMaterialIndex);
   mMapping.SetCurSel(0);

   if (material) {
      material->CreateThumbnail();
      ThumbPreview(mMaterialThumb.GetSafeHwnd(), material->thumbnail);
   }

	return TRUE;
}

// +--------------------------------------------------------------------+

void TextureMapDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

   if (material && material->thumbnail) {
      ThumbPreview(mMaterialThumb.GetSafeHwnd(), material->thumbnail);
   }
   else {
      ThumbPreview(mMaterialThumb.GetSafeHwnd(), blank);
   }
}

void TextureMapDialog::OnSelectMaterial() 
{
   mMaterialIndex = mMaterialList.GetCurSel()-1;
   material = 0;

   if (model && mMaterialIndex >= 0 && mMaterialIndex < model->NumMaterials()) {
      material = model->GetMaterials()[mMaterialIndex];
   }

   if (material) {
      material->CreateThumbnail();
      ThumbPreview(mMaterialThumb.GetSafeHwnd(), material->thumbnail);
   }
   else {
      ThumbPreview(mMaterialThumb.GetSafeHwnd(), blank);
   }
}

void TextureMapDialog::OnAlign() 
{
   if (mMapping.GetCurSel() == 0) {
      mMapping.SetCurSel(1);
      UpdateData(TRUE);

      mScaleU = 1;
      mScaleV = 1;
      mMaterialIndex++;
      UpdateData(FALSE);
   }
}
