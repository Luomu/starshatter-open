// SurfacePropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Magic.h"
#include "MagicDoc.h"
#include "MagicView.h"
#include "Solid.h"
#include "SurfacePropertiesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SurfacePropertiesDialog dialog


SurfacePropertiesDialog::SurfacePropertiesDialog(MagicView* pParent /*=NULL*/)
	: CDialog(SurfacePropertiesDialog::IDD, pParent), doc(0)
{
	//{{AFX_DATA_INIT(SurfacePropertiesDialog)
	mSurfaceName = _T("");
	mNumPolys = _T("");
	mNumVerts = _T("");
	mSurfaceHeight = _T("");
	mSurfaceLength = _T("");
	mSurfaceRadius = _T("");
	mSurfaceWidth = _T("");
	//}}AFX_DATA_INIT

   if (pParent) {
      doc = pParent->GetDocument();

      if (doc && doc->GetSolid()) {
         Model*   model    = doc->GetSolid()->GetModel();
         Surface* surface  = model->GetSurfaces().first();
         Point    plus;
         Point    minus;

         surface->GetVertexSet()->CalcExtents(plus, minus);

         char buffer[256];
         strcpy(buffer, surface->Name());
         mSurfaceName = buffer;

         sprintf(buffer, "%d", surface->NumPolys());
         mNumPolys = buffer;

         sprintf(buffer, "%d", surface->NumVerts());
         mNumVerts = buffer;

         sprintf(buffer, "%.1f", surface->Radius());
         mSurfaceRadius = buffer;

         sprintf(buffer, "%.1f  (%.1f - %.1f)", plus.z-minus.z, minus.z, plus.z);
         mSurfaceLength = buffer;

         sprintf(buffer, "%.1f  (%.1f - %.1f)", plus.x-minus.x, minus.x, plus.x);
         mSurfaceWidth = buffer;

         sprintf(buffer, "%.1f  (%.1f - %.1f)", plus.y-minus.y, minus.y, plus.y);
         mSurfaceHeight = buffer;
      }
   }
}


void SurfacePropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SurfacePropertiesDialog)
	DDX_CBString(pDX, IDC_SURFACE_NAME, mSurfaceName);
	DDX_Text(pDX, IDC_SURFACE_NPOLYS, mNumPolys);
	DDX_Text(pDX, IDC_SURFACE_NVERTS, mNumVerts);
	DDX_Text(pDX, IDC_SURFACE_HEIGHT, mSurfaceHeight);
	DDX_Text(pDX, IDC_SURFACE_LENGTH, mSurfaceLength);
	DDX_Text(pDX, IDC_SURFACE_RADIUS, mSurfaceRadius);
	DDX_Text(pDX, IDC_SURFACE_WIDTH, mSurfaceWidth);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SurfacePropertiesDialog, CDialog)
	//{{AFX_MSG_MAP(SurfacePropertiesDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SurfacePropertiesDialog message handlers
