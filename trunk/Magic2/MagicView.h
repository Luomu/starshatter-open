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
    FILE:         MagicView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Interface of the MagicView class
*/


#if !defined(AFX_MAGICVIEW_H__387B567A_8235_41B8_A993_E41567E680D7__INCLUDED_)
#define AFX_MAGICVIEW_H__387B567A_8235_41B8_A993_E41567E680D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// +--------------------------------------------------------------------+

class MagicDoc;
class ActiveWindow;
class Grid;
class Light;
class Scene;
class Screen;
class Video;
class VideoSettings;
class ModelView;
class UVMapView;

// +--------------------------------------------------------------------+

class MagicView : public CView
{
protected: // create from serialization only
    MagicView();
    DECLARE_DYNCREATE(MagicView)

// Attributes
public:
    MagicDoc* GetDocument();

   static MagicView* GetInstance();

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(MagicView)
    public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();
    protected:
    virtual void OnDraw(CDC* pDC);
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~MagicView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

   virtual void   OnEnterSizeMove();
   virtual void   OnExitSizeMove();
   virtual void   ResizeVideo();
   virtual CPoint LPtoWP(const CPoint& p);

protected:
   Video*         video;
   VideoSettings* video_settings;
   Screen*        screen;
   RECT           client_rect;
   DWORD          window_style;
   bool           is_minimized;
   bool           is_maximized;
   bool           is_sizing;

   Grid*          grid;
   Scene*         scene;
   Light*         main_light;
   Light*         back_light;
   ActiveWindow*  main_win;
   ActiveWindow*  view_win[4];
   ModelView*     model_view[4];
   ActiveWindow*  uvmap_win;
   UVMapView*     uvmap_view;
   int            view_mode;
   int            view_focus;
   bool           drag_left;
   bool           drag_right;
   CPoint         drag_start;

   bool           view_bumpmaps;
   bool           view_shadows;
   bool           animate_light;

   enum VIEW_MODE {
      VIEW_TOP          = 0,
      VIEW_PERSPECTIVE  = 1,
      VIEW_SIDE         = 2, 
      VIEW_FRONT        = 3,
      VIEW_ALL          = 4,
      VIEW_UV_MAP       = 5
   };

   virtual void   SetupModelViews();
   virtual void   SetFocusModelView(int f);
   virtual int    GetWinIndexByPoint(int x, int y);
   ModelView*     GetModelViewByIndex(int index);

   bool           IsUVEdit() const { return (view_mode == VIEW_UV_MAP) && (uvmap_view != 0); }
   void           CloseUVEditor();

// Generated message map functions
protected:
    //{{AFX_MSG(MagicView)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnRender();
    afx_msg void OnPaint();
    afx_msg void OnViewAll();
    afx_msg void OnUpdateViewAll(CCmdUI* pCmdUI);
    afx_msg void OnViewFront();
    afx_msg void OnUpdateViewFront(CCmdUI* pCmdUI);
    afx_msg void OnViewPerspective();
    afx_msg void OnUpdateViewPerspective(CCmdUI* pCmdUI);
    afx_msg void OnViewSide();
    afx_msg void OnUpdateViewSide(CCmdUI* pCmdUI);
    afx_msg void OnViewTop();
    afx_msg void OnUpdateViewTop(CCmdUI* pCmdUI);
    afx_msg void OnTextureMap();
    afx_msg void OnModifyMaterial();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnViewZoomNormal();
    afx_msg void OnViewZoomIn();
    afx_msg void OnViewZoomOut();
    afx_msg void OnViewModeWireframe();
    afx_msg void OnViewModeSolid();
    afx_msg void OnViewModeTextured();
    afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnGridProperties();
    afx_msg void OnGridShow();
    afx_msg void OnGridSnap();
    afx_msg void OnUpdateGridSnap(CCmdUI* pCmdUI);
    afx_msg void OnViewBackColor();
    afx_msg void OnFileImport();
    afx_msg void OnFileExport();
    afx_msg void OnSelectAll();
    afx_msg void OnSelectNone();
    afx_msg void OnUpdateTextureMap(CCmdUI* pCmdUI);
    afx_msg void OnUpdateModifyMaterial(CCmdUI* pCmdUI);
    afx_msg void OnSelectInverse();
    afx_msg void OnModifyUVMap();
    afx_msg void OnUpdateModifyUVMap(CCmdUI* pCmdUI);
    afx_msg void OnViewShadows();
    afx_msg void OnUpdateViewShadows(CCmdUI* pCmdUI);
    afx_msg void OnViewAnimatelight();
    afx_msg void OnUpdateViewAnimatelight(CCmdUI* pCmdUI);
    afx_msg void OnViewBumpmaps();
    afx_msg void OnUpdateViewBumpmaps(CCmdUI* pCmdUI);
    afx_msg void OnViewVertexshader();
    afx_msg void OnUpdateViewVertexshader(CCmdUI* pCmdUI);
    afx_msg void OnViewPixelshader();
    afx_msg void OnUpdateViewPixelshader(CCmdUI* pCmdUI);
    afx_msg void OnViewVisibleshadows();
    afx_msg void OnUpdateViewVisibleshadows(CCmdUI* pCmdUI);
    afx_msg void OnEditUndo();
    afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
    afx_msg void OnEditRedo();
    afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
    afx_msg void OnSurfaceProperties();
    afx_msg void OnUpdateSurfaceProperties(CCmdUI* pCmdUI);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MagicView.cpp
inline MagicDoc* MagicView::GetDocument()
   { return (MagicDoc*)m_pDocument; }
#endif

// +--------------------------------------------------------------------+

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAGICVIEW_H__387B567A_8235_41B8_A993_E41567E680D7__INCLUDED_)
