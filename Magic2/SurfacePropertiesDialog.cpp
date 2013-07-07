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
 */

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
         strcpy_s(buffer, surface->Name());
         mSurfaceName = buffer;

         sprintf_s(buffer, "%d", surface->NumPolys());
         mNumPolys = buffer;

         sprintf_s(buffer, "%d", surface->NumVerts());
         mNumVerts = buffer;

         sprintf_s(buffer, "%.1f", surface->Radius());
         mSurfaceRadius = buffer;

         sprintf_s(buffer, "%.1f  (%.1f - %.1f)", plus.z-minus.z, minus.z, plus.z);
         mSurfaceLength = buffer;

         sprintf_s(buffer, "%.1f  (%.1f - %.1f)", plus.x-minus.x, minus.x, plus.x);
         mSurfaceWidth = buffer;

         sprintf_s(buffer, "%.1f  (%.1f - %.1f)", plus.y-minus.y, minus.y, plus.y);
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
