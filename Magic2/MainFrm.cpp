// MainFrm.cpp : implementation of the MainFrame class
//

#include "stdafx.h"
#include "Magic.h"

#include "MainFrm.h"
#include "MagicView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MainFrame

#include "Bitmap.h"

IMPLEMENT_DYNCREATE(MainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(MainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(MainFrame)
	ON_WM_CREATE()
	ON_WM_ACTIVATEAPP()
	ON_COMMAND(ID_VIEW_RENDER, OnRender)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // xy indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// MainFrame construction/destruction

MainFrame* MainFrame::statframe;


MainFrame::MainFrame()
{
   statframe = this;
	
}

MainFrame::~MainFrame()
{
   Bitmap::ClearCache();
}

BOOL MainFrame::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
   if (message == WM_ENTERSIZEMOVE) {
      MagicView* magic_view = MagicView::GetInstance();
      if (magic_view)
         magic_view->OnEnterSizeMove();
   }

   else if (message == WM_EXITSIZEMOVE) {
      MagicView* magic_view = MagicView::GetInstance();
      if (magic_view)
         magic_view->OnExitSizeMove();
   }

   return CFrameWnd::OnWndMsg(message, wParam, lParam, pResult);
}

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

void MainFrame::StatusXY(const char* xy)
{
   statframe->m_wndStatusBar.SetPaneText(1, xy, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// MainFrame diagnostics

#ifdef _DEBUG
void MainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void MainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// MainFrame message handlers


void MainFrame::OnActivateApp(BOOL bActive, DWORD dwThreadID) 
{
    ((Magic*)AfxGetApp())->SetAppActivated(bActive ? true : false);
}

void MainFrame::OnRender() 
{
   MagicView* magic_view = MagicView::GetInstance();
   if (magic_view)
      magic_view->SendMessage(WM_COMMAND, ID_VIEW_RENDER);
}
