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

#include "stdafx.h"
#include "Magic.h"
#include "MagicDoc.h"
#include "MagicView.h"
#include "MaterialDialog.h"
#include "Selection.h"
#include "Selector.h"
#include "Thumbnail.h"

#include "Bitmap.h"
#include "Polygon.h"
#include "Solid.h"
#include "Video.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

inline int BlendModeToSelection(int mode)
{
   switch (mode) {
   case Material::MTL_SOLID:        return 0;
   case Material::MTL_TRANSLUCENT:  return 1;
   case Material::MTL_ADDITIVE:     return 2;
   }

   return 0;
}

inline int SelectionToBlendMode(int sel)
{
   switch (sel) {
   case 0:  return Material::MTL_SOLID;
   case 1:  return Material::MTL_TRANSLUCENT;
   case 2:  return Material::MTL_ADDITIVE;
   }

   return Material::MTL_SOLID;
}

// +--------------------------------------------------------------------+
// MaterialDialog dialog
// +--------------------------------------------------------------------+

static Material emergency_material;

MaterialDialog::MaterialDialog(MagicView* pParent /*=NULL*/)
	: CDialog(MaterialDialog::IDD, pParent), solid(0), material(0), doc(0)
{
	//{{AFX_DATA_INIT(MaterialDialog)
	mAmbientValue = 0.0;
	mBrillianceValue = 0.0;
	mBumpValue = 0.0;
	mDiffuseValue = 0.0;
	mEmissiveValue = 0.0;
	mMaterialName = _T("");
	mPowerValue = 0.0;
	mSpecularValue = 0.0;
	mSpecularTexture = _T("");
	mDiffuseTexture = _T("");
	mBumpTexture = _T("");
	mEmissiveTexture = _T("");
	mMaterialShader = _T("");
	//}}AFX_DATA_INIT

   doc = pParent->GetDocument();

   if (doc && doc->GetSolid()) {
      solid = doc->GetSolid();

      Selection* seln = doc->GetSelection();

      if (seln->GetPolys().size() > 0) {
         material = seln->GetPolys().first()->material;
      }
   }
}

MaterialDialog::~MaterialDialog()
{
}


void MaterialDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MaterialDialog)
	DDX_Control(pDX, IDC_SHADOW, mShadowList);
	DDX_Control(pDX, IDC_BLEND_MODE, mBlendModeList);
	DDX_Control(pDX, IDC_MATERIAL_LIST, mMaterialList);
	DDX_Control(pDX, IDC_MATERIAL_PREVIEW, mMaterialThumb);
	DDX_Control(pDX, IDC_SPECULAR_COLOR, mSpecularColor);
	DDX_Control(pDX, IDC_EMISSIVE_COLOR, mEmissiveColor);
	DDX_Control(pDX, IDC_DIFFUSE_COLOR, mDiffuseColor);
	DDX_Control(pDX, IDC_AMBIENT_COLOR, mAmbientColor);
	DDX_Text(pDX, IDC_AMBIENT_VALUE, mAmbientValue);
	DDX_Text(pDX, IDC_BRILLIANCE_VALUE, mBrillianceValue);
	DDX_Text(pDX, IDC_BUMP_VALUE, mBumpValue);
	DDX_Text(pDX, IDC_DIFFUSE_VALUE, mDiffuseValue);
	DDX_Text(pDX, IDC_EMISSIVE_VALUE, mEmissiveValue);
	DDX_Text(pDX, IDC_MATERIAL_NAME, mMaterialName);
	DDX_Text(pDX, IDC_POWER_VALUE, mPowerValue);
	DDX_Text(pDX, IDC_SPECULAR_VALUE, mSpecularValue);
	DDX_Text(pDX, IDC_SPECULAR_TEXTURE, mSpecularTexture);
	DDX_Text(pDX, IDC_DIFFUSE_TEXTURE, mDiffuseTexture);
	DDX_Text(pDX, IDC_BUMP_TEXTURE, mBumpTexture);
	DDX_Text(pDX, IDC_EMISSIVE_TEXTURE, mEmissiveTexture);
	DDX_Text(pDX, IDC_MATERIAL_SHADER, mMaterialShader);
	//}}AFX_DATA_MAP

   if (!pDX->m_bSaveAndValidate) {
      if (solid && solid->GetModel()) {
         mMaterialList.ResetContent();

         Model* model = solid->GetModel();
         for (int i = 0; i < model->NumMaterials(); i++) {
            Material* m = model->GetMaterials()[i];
            mMaterialList.AddString(m->name);

            if (m == material)
               mMaterialList.SetCurSel(i);
         }
      }

      if (mBlendModeList.GetSafeHwnd())
         mBlendModeList.SetCurSel(BlendModeToSelection(material->blend));

      if (mShadowList.GetSafeHwnd())
         mShadowList.SetCurSel(material->shadow);
   }
}


BEGIN_MESSAGE_MAP(MaterialDialog, CDialog)
	//{{AFX_MSG_MAP(MaterialDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_AMBIENT_COLOR, OnAmbientColor)
	ON_BN_CLICKED(IDC_DIFFUSE_COLOR, OnDiffuseColor)
	ON_BN_CLICKED(IDC_EMISSIVE_COLOR, OnEmissiveColor)
	ON_BN_CLICKED(IDC_SPECULAR_COLOR, OnSpecularColor)
	ON_WM_DRAWITEM()
	ON_EN_CHANGE(IDC_AMBIENT_VALUE, OnChangeMaterialValue)
	ON_BN_CLICKED(IDC_FILE_DIFFUSE, OnFileDiffuse)
	ON_BN_CLICKED(IDC_FILE_SPECULAR, OnFileSpecular)
	ON_BN_CLICKED(IDC_FILE_EMISSIVE, OnFileEmissive)
	ON_BN_CLICKED(IDC_FILE_BUMP, OnFileBump)
	ON_EN_CHANGE(IDC_DIFFUSE_TEXTURE, OnChangeDiffuseTexture)
	ON_EN_CHANGE(IDC_SPECULAR_TEXTURE, OnChangeSpecularTexture)
	ON_EN_CHANGE(IDC_EMISSIVE_TEXTURE, OnChangeEmissiveTexture)
	ON_EN_CHANGE(IDC_BUMP_TEXTURE, OnChangeBumpTexture)
	ON_EN_CHANGE(IDC_MATERIAL_NAME, OnChangeMaterialName)
	ON_LBN_SELCHANGE(IDC_MATERIAL_LIST, OnSelectMaterial)
	ON_BN_CLICKED(IDC_SELECT_POLYS, OnSelectPolys)
	ON_BN_CLICKED(IDC_NEW_MATERIAL, OnNewMaterial)
	ON_BN_CLICKED(IDC_DEL_MATERIAL, OnDelMaterial)
	ON_CBN_SELCHANGE(IDC_BLEND_MODE, OnSelectBlendMode)
	ON_CBN_SELCHANGE(IDC_SHADOW, OnSelectShadow)
	ON_EN_CHANGE(IDC_BRILLIANCE_VALUE, OnChangeMaterialValue)
	ON_EN_CHANGE(IDC_BUMP_VALUE, OnChangeMaterialValue)
	ON_EN_CHANGE(IDC_DIFFUSE_VALUE, OnChangeMaterialValue)
	ON_EN_CHANGE(IDC_EMISSIVE_VALUE, OnChangeMaterialValue)
	ON_EN_CHANGE(IDC_POWER_VALUE, OnChangeMaterialValue)
	ON_EN_CHANGE(IDC_SPECULAR_VALUE, OnChangeMaterialValue)
	ON_EN_CHANGE(IDC_MATERIAL_SHADER, OnChangeMaterialShader)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// +--------------------------------------------------------------------+

void MaterialDialog::UpdateMaterial()
{
   if (material) {
      UpdateData();

      strcpy_s(material->name,   mMaterialName);
      strcpy_s(material->shader, mMaterialShader);

      material->ambient_value    = mAmbientValue;
      material->diffuse_value    = mDiffuseValue;
      material->specular_value   = mSpecularValue;
      material->emissive_value   = mEmissiveValue;
      material->power            = mPowerValue;
      material->brilliance       = mBrillianceValue;
      material->bump             = mBumpValue;
      material->blend            = SelectionToBlendMode(mBlendModeList.GetCurSel());
      material->shadow           = mShadowList.GetCurSel() ? true : false;

      material->Ka = ColorValue(material->ambient_color)  * material->ambient_value;
      material->Kd = ColorValue(material->diffuse_color)  * material->diffuse_value;
      material->Ks = ColorValue(material->specular_color) * material->specular_value;
      material->Ke = ColorValue(material->emissive_color) * material->emissive_value;

      material->CreateThumbnail();

      InvalidateRect(NULL, FALSE);
   }
}

// +--------------------------------------------------------------------+
// MaterialDialog message handlers
// +--------------------------------------------------------------------+

BOOL MaterialDialog::OnInitDialog() 
{
   if (solid && solid->GetModel() && !material) {
      Model* model = solid->GetModel();

      if (model->NumMaterials() > 0)
         material = model->GetMaterials().first();
   }

   if (!material)
	   material = &emergency_material;

   mMaterialName     = material->name;
   mMaterialShader   = material->shader;
   mAmbientValue     = material->ambient_value;
   mDiffuseValue     = material->diffuse_value;
   mSpecularValue    = material->specular_value;
   mEmissiveValue    = material->emissive_value;
   mPowerValue       = material->power;
   mBrillianceValue  = material->brilliance;
   mBumpValue        = material->bump;

   mDiffuseTexture   = material->tex_diffuse ?
                       material->tex_diffuse->GetFilename() : "";

   mSpecularTexture  = material->tex_specular ?
                       material->tex_specular->GetFilename() : "";

   mEmissiveTexture  = material->tex_emissive ?
                       material->tex_emissive->GetFilename() : "";

   mBumpTexture      = material->tex_bumpmap ?
                       material->tex_bumpmap->GetFilename() : "";

	CDialog::OnInitDialog();

   UpdateMaterial();
	
   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MaterialDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

   if (material && material->thumbnail) {
      ThumbPreview(mMaterialThumb.GetSafeHwnd(), material->thumbnail);
   }
}

void MaterialDialog::OnAmbientColor() 
{
   if (material) {
      Color          c     = material->ambient_color;
      COLORREF       crgb  = RGB(c.Red(), c.Green(), c.Blue());
      CColorDialog   chooser(crgb);

      if (chooser.DoModal() == IDOK) {
         crgb = chooser.GetColor();
         material->ambient_color = Color(GetRValue(crgb), GetGValue(crgb), GetBValue(crgb));
         UpdateMaterial();
      }
   }
}

void MaterialDialog::OnDiffuseColor() 
{
   if (material) {
      Color          c     = material->diffuse_color;
      COLORREF       crgb  = RGB(c.Red(), c.Green(), c.Blue());
      CColorDialog   chooser(crgb);

      if (chooser.DoModal() == IDOK) {
         crgb = chooser.GetColor();
         material->diffuse_color = Color(GetRValue(crgb), GetGValue(crgb), GetBValue(crgb));
         UpdateMaterial();
      }
   }
}

void MaterialDialog::OnSpecularColor() 
{
   if (material) {
      Color          c     = material->specular_color;
      COLORREF       crgb  = RGB(c.Red(), c.Green(), c.Blue());
      CColorDialog   chooser(crgb);

      if (chooser.DoModal() == IDOK) {
         crgb = chooser.GetColor();
         material->specular_color = Color(GetRValue(crgb), GetGValue(crgb), GetBValue(crgb));
         UpdateMaterial();
      }
   }
}

void MaterialDialog::OnEmissiveColor() 
{
   if (material) {
      Color          c     = material->emissive_color;
      COLORREF       crgb  = RGB(c.Red(), c.Green(), c.Blue());
      CColorDialog   chooser(crgb);

      if (chooser.DoModal() == IDOK) {
         crgb = chooser.GetColor();
         material->emissive_color = Color(GetRValue(crgb), GetGValue(crgb), GetBValue(crgb));
         UpdateMaterial();
      }
   }
}

void MaterialDialog::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
   CWnd* wnd = GetDlgItem(nIDCtl);
   Color c   = Color::LightGray;

   if (material) {
      if (wnd->GetSafeHwnd() == mAmbientColor.GetSafeHwnd())
         c = material->ambient_color;

      else if (wnd->GetSafeHwnd() == mDiffuseColor.GetSafeHwnd())
         c = material->diffuse_color;

      else if (wnd->GetSafeHwnd() == mSpecularColor.GetSafeHwnd())
         c = material->specular_color;

      else if (wnd->GetSafeHwnd() == mEmissiveColor.GetSafeHwnd())
         c = material->emissive_color;
   }

   CBrush brush(RGB(c.Red(), c.Green(), c.Blue()));

   ::FillRect(lpDrawItemStruct->hDC,
             &lpDrawItemStruct->rcItem,
              brush);

	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

// +--------------------------------------------------------------------+

void MaterialDialog::OnChangeMaterialValue() 
{
   UpdateMaterial();
}

// +--------------------------------------------------------------------+

void MaterialDialog::OnFileTexture(int type)
{
   if (!material)
      return;

   char filename[512];
   filename[0] = '\0';
   CFileDialog ofd(TRUE, "pcx");
   
   ofd.m_ofn.lpstrFilter = "All Image Files\0*.bmp; *.dds; *.jpg; *.pcx; *.png; *.tga\0Bitmap Files (*.bmp)\0*.bmp\0JPEG Files (*.jpg)\0*.pcx\0PCX Files (*.pcx)\0*.pcx\0PNG Files (*.png)\0*.png\0Truevision Targa Files (*.tga)\0*.png\0All Files\0*.*\0\0";
   ofd.m_ofn.lpstrFile   = filename;
   ofd.m_ofn.nMaxFile    = sizeof(filename);
   
   if (ofd.DoModal() != IDOK)
      return;

   char tex_name[512];
   sprintf_s(tex_name, "%s", ofd.GetFileName().GetBuffer(0));

   ChangeFileTexture(tex_name, type);
}

void MaterialDialog::ChangeFileTexture(char* fname, int type)
{
   Bitmap* bmp = 0;

   LoadTexture(fname, bmp);

   if (bmp) {
      switch (type) {
      case 0:
         material->tex_diffuse = bmp;
         mDiffuseTexture = fname;
         break;

      case 1:
         material->tex_specular = bmp;
         mSpecularTexture = fname;
         break;

      case 2:
         material->tex_emissive = bmp;
         mEmissiveTexture = fname;
         break;

      case 3:
         material->tex_bumpmap = bmp;
          mBumpTexture = fname;
         break;
      }

      UpdateData(FALSE);
      UpdateMaterial();
   }
}

void MaterialDialog::OnFileDiffuse() 
{
   OnFileTexture(0);
}

void MaterialDialog::OnFileSpecular() 
{
   OnFileTexture(1);
}

void MaterialDialog::OnFileEmissive() 
{
   OnFileTexture(2);
}

void MaterialDialog::OnFileBump() 
{
   OnFileTexture(3);
}

void MaterialDialog::OnChangeDiffuseTexture() 
{
   if (material) {
      UpdateData();

      if (mDiffuseTexture.GetLength() < 1) {
         material->tex_diffuse = 0;
         UpdateMaterial();
         return;
      }

      char* filename = mDiffuseTexture.LockBuffer();
      ChangeFileTexture(filename, 0);
      mDiffuseTexture.UnlockBuffer();
   }
}

void MaterialDialog::OnChangeSpecularTexture() 
{
   if (material) {
      UpdateData();

      if (mSpecularTexture.GetLength() < 1) {
         material->tex_specular = 0;
         UpdateMaterial();
         return;
      }

      char* filename = mSpecularTexture.LockBuffer();
      ChangeFileTexture(filename, 1);
      mSpecularTexture.UnlockBuffer();
   }
}

void MaterialDialog::OnChangeEmissiveTexture() 
{
   if (material) {
      UpdateData();

      if (mEmissiveTexture.GetLength() < 1) {
         material->tex_emissive = 0;
         UpdateMaterial();
         return;
      }

      char* filename = mEmissiveTexture.LockBuffer();
      ChangeFileTexture(filename, 2);
      mEmissiveTexture.UnlockBuffer();
   }
}

void MaterialDialog::OnChangeBumpTexture() 
{
   if (material) {
      UpdateData();

      if (mBumpTexture.GetLength() < 1) {
         material->tex_bumpmap = 0;
         UpdateMaterial();
         return;
      }

      char* filename = mBumpTexture.LockBuffer();
      ChangeFileTexture(filename, 3);
      mBumpTexture.UnlockBuffer();
   }
}

// +--------------------------------------------------------------------+

void MaterialDialog::OnChangeMaterialName() 
{
   if (material) {
      UpdateData();
      strcpy_s(material->name, mMaterialName);
   }
}

void MaterialDialog::OnChangeMaterialShader() 
{
   if (material) {
      UpdateData();
      strcpy_s(material->shader, mMaterialShader);
   }
}

void MaterialDialog::OnSelectMaterial() 
{
   int         selected = mMaterialList.GetCurSel();
   Material*   mtl      = 0;

   if (solid && solid->GetModel()) {
      Model* model = solid->GetModel();

      if (model->NumMaterials() > 0 && selected < model->NumMaterials())
         mtl = model->GetMaterials()[selected];
   }

   if (!mtl)
	   mtl = &emergency_material;

   if (material != mtl) {
      material = mtl;

      mMaterialName     = material->name;
      mMaterialShader   = material->shader;
      mAmbientValue     = material->ambient_value;
      mDiffuseValue     = material->diffuse_value;
      mSpecularValue    = material->specular_value;
      mEmissiveValue    = material->emissive_value;
      mPowerValue       = material->power;
      mBrillianceValue  = material->brilliance;
      mBumpValue        = material->bump;

      mDiffuseTexture   = material->tex_diffuse ?
                          material->tex_diffuse->GetFilename() : "";

      mSpecularTexture  = material->tex_specular ?
                          material->tex_specular->GetFilename() : "";

      mEmissiveTexture  = material->tex_emissive ?
                          material->tex_emissive->GetFilename() : "";

      mBumpTexture      = material->tex_bumpmap ?
                          material->tex_bumpmap->GetFilename() : "";

      if (mBlendModeList.GetSafeHwnd())
         mBlendModeList.SetCurSel(BlendModeToSelection(material->blend));

      if (mShadowList.GetSafeHwnd())
         mShadowList.SetCurSel(material->shadow);

      UpdateData(FALSE);
      UpdateMaterial();
   }
}

// +--------------------------------------------------------------------+

void MaterialDialog::OnNewMaterial() 
{
   if (solid && material && material != &emergency_material) {
      Model*      model = solid->GetModel();
      Material*   mtl   = new Material;

      if (model && mtl) {
         mtl->Ka = Color::DarkGray;
         mtl->Kd = Color::LightGray;
         mtl->Ks = ColorValue(0.1f,0.1f,0.1f);
         mtl->power = 10.0f;

         mtl->ambient_value  = 0.2f;
         mtl->ambient_color  = Color::DarkGray;
         mtl->diffuse_value  = 0.8f;
         mtl->diffuse_color  = Color::LightGray;
         mtl->specular_value = 0.5f;
         mtl->specular_color = Color::White;
         strcpy_s(mtl->name, "(new)");

         model->GetMaterials().append(mtl);

         material = 0;
         mMaterialList.AddString(mtl->name);
         mMaterialList.SetCurSel(model->NumMaterials()-1);

         mBlendModeList.SetCurSel(0);
         mShadowList.SetCurSel(1);

         OnSelectMaterial();
      }
   }
}

void MaterialDialog::OnDelMaterial() 
{
   if (solid && material && material != &emergency_material) {
      Model* model = solid->GetModel();

      // do not delete the last material:
      if (model->NumMaterials() > 1 && model->GetMaterials().contains(material)) {
         Material* mtl = model->GetMaterials().first();

         if (mtl == material)
            mtl = model->GetMaterials()[1];

         // reassign the material for any polys and segments
         // that are using the one we are about to delete:
         ListIter<Surface> iter = model->GetSurfaces();

         while (++iter) {
            Surface* s = iter.value();

            for (int i = 0; i < s->NumPolys(); i++) {
               Poly* p = s->GetPolys() + i;

               if (p->material == material) {
                  p->material = mtl;
               }
            }

            ListIter<Segment> seg_iter = s->GetSegments();
            while (++seg_iter) {
               Segment* segment = seg_iter.value();
               if (segment->material == material)
                  segment->material = mtl;
            }
         }

         // now delete the material:
         model->GetMaterials().remove(material);
         delete material;

         material = 0;
         mMaterialList.SetCurSel(0);
         OnSelectMaterial();
      }
   }
}

void MaterialDialog::OnSelectPolys() 
{
   if (solid && doc && doc->GetSelector()) {
      Selector* selector = doc->GetSelector();

      if (!material || material == &emergency_material)
         selector->SelectMaterial(0);
      else
         selector->SelectMaterial(material);
   }
}

void MaterialDialog::OnSelectBlendMode() 
{
   if (material)
      material->blend = SelectionToBlendMode(mBlendModeList.GetCurSel());
}

void MaterialDialog::OnSelectShadow() 
{
   if (material)
      material->shadow = mShadowList.GetCurSel() ? true : false;
}

void MaterialDialog::OnOK() 
{
   Video* video = Video::GetInstance();
   if (video)
      video->InvalidateCache();

	CDialog::OnOK();
}
