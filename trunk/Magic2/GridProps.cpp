/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         GridProps.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Grid Properties Dialog implementation file
*/


#include "stdafx.h"
#include "Magic.h"
#include "GridProps.h"
#include "Grid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// +--------------------------------------------------------------------+
// GridProps dialog
// +--------------------------------------------------------------------+

GridProps::GridProps(Grid* g, CWnd* pParent /*=NULL*/)
	: CDialog(GridProps::IDD, pParent), grid(g)
{
	//{{AFX_DATA_INIT(GridProps)
	mGridShow         = grid->IsShow();
	mGridSnap         = grid->IsSnap();
	mReferencePlan    = grid->GetReferencePlan();
	mReferenceFront   = grid->GetReferenceFront();
	mReferenceSide    = grid->GetReferenceSide();
	mGridSize         = grid->GetSize();
	//}}AFX_DATA_INIT
}

static const char* C(CString& str)
{
   static char buf[512];
   int i;
   for (i = 0; i < str.GetLength(); i++)
      buf[i] = (char) str.GetAt(i);
	buf[i] = 0;

   return buf;
}

void GridProps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GridProps)
	DDX_Check(pDX, IDC_GRID_SHOW, mGridShow);
	DDX_Check(pDX, IDC_GRID_SNAP, mGridSnap);
	DDX_Text(pDX, IDC_REFERENCE_PLAN, mReferencePlan);
	DDX_Text(pDX, IDC_REFERENCE_FRONT, mReferenceFront);
	DDX_Text(pDX, IDC_REFERENCE_SIDE, mReferenceSide);
	DDX_Text(pDX, IDC_GRID_SIZE, mGridSize);
	DDV_MinMaxInt(pDX, mGridSize, 1, 64);
	//}}AFX_DATA_MAP

   // if saving, write the values back to the grid
   if (pDX->m_bSaveAndValidate) {
      grid->SetSnap(mGridSnap ? true : false);
      grid->SetShow(mGridShow ? true : false);
      grid->SetSize(mGridSize);

      grid->SetReferencePlan(C(mReferencePlan));
      grid->SetReferenceFront(C(mReferenceFront));
      grid->SetReferenceSide(C(mReferenceSide));
   }
}


BEGIN_MESSAGE_MAP(GridProps, CDialog)
	//{{AFX_MSG_MAP(GridProps)
	ON_BN_CLICKED(IDC_FILE_PLAN, OnFilePlan)
	ON_BN_CLICKED(IDC_FILE_FRONT, OnFileFront)
	ON_BN_CLICKED(IDC_FILE_SIDE, OnFileSide)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// +--------------------------------------------------------------------+

static void OnImageFile(CString& strImageFile)
{
   char filename[512];
   filename[0] = '\0';
   CFileDialog ofd(TRUE, "pcx");
   
   ofd.m_ofn.lpstrFilter = "PCX Files\0*.pcx\0All Files\0*.*\0\0";
   ofd.m_ofn.lpstrFile   = filename;
   ofd.m_ofn.nMaxFile    = sizeof(filename);
   
   if (ofd.DoModal() != IDOK)
      return;

   char tex_name[512];
   sprintf(tex_name, "%s", ofd.GetFileName().GetBuffer(0));

   strImageFile = tex_name;
}

void GridProps::OnFilePlan() 
{
   OnImageFile(mReferencePlan);
   UpdateData(FALSE);
}

void GridProps::OnFileFront() 
{
   OnImageFile(mReferenceFront);
   UpdateData(FALSE);
}

void GridProps::OnFileSide() 
{
   OnImageFile(mReferenceSide);
   UpdateData(FALSE);
}
