/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         MagicView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Implementation of the MagicView class
*/

#include "stdafx.h"
#include "Magic.h"

#include "MagicDoc.h"
#include "MagicView.h"
#include "MainFrm.h"
#include "MaterialDialog.h"
#include "SurfacePropertiesDialog.h"
#include "TextureMapDialog.h"
#include "Editor.h"
#include "Grid.h"
#include "GridProps.h"
#include "Selection.h"
#include "Selector.h"
#include "UVMapView.h"

#include "ActiveWindow.h"
#include "Color.h"
#include "Layout.h"
#include "Light.h"
#include "Scene.h"
#include "Screen.h"
#include "Shadow.h"
#include "Solid.h"
#include "Video.h"
#include "VideoDX9.h"
#include "VideoSettings.h"

#include "ModelView.h"

DWORD GetRealTime();

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern PALETTEENTRY standard_palette[256];
extern BYTE         inverse_palette[32768];

// +--------------------------------------------------------------------+

IMPLEMENT_DYNCREATE(MagicView, CView)

BEGIN_MESSAGE_MAP(MagicView, CView)
	//{{AFX_MSG_MAP(MagicView)
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_RENDER, OnRender)
	ON_WM_PAINT()
	ON_COMMAND(ID_VIEW_ALL, OnViewAll)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ALL, OnUpdateViewAll)
	ON_COMMAND(ID_VIEW_FRONT, OnViewFront)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FRONT, OnUpdateViewFront)
	ON_COMMAND(ID_VIEW_PERSPECTIVE, OnViewPerspective)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PERSPECTIVE, OnUpdateViewPerspective)
	ON_COMMAND(ID_VIEW_SIDE, OnViewSide)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SIDE, OnUpdateViewSide)
	ON_COMMAND(ID_VIEW_TOP, OnViewTop)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOP, OnUpdateViewTop)
	ON_COMMAND(ID_MODIFY_TEXTURE_MAP, OnTextureMap)
	ON_COMMAND(ID_MODIFY_MATERIAL, OnModifyMaterial)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_VIEW_ZOOM_NORMAL, OnViewZoomNormal)
	ON_COMMAND(ID_VIEW_ZOOM_IN, OnViewZoomIn)
	ON_COMMAND(ID_VIEW_ZOOM_OUT, OnViewZoomOut)
	ON_COMMAND(ID_VIEW_MODE_WIREFRAME, OnViewModeWireframe)
	ON_COMMAND(ID_VIEW_MODE_SOLID, OnViewModeSolid)
	ON_COMMAND(ID_VIEW_MODE_TEXTURED, OnViewModeTextured)
	ON_WM_RBUTTONDBLCLK()
	ON_COMMAND(ID_PROP_GRID, OnGridProperties)
	ON_COMMAND(ID_GRID_SHOW, OnGridShow)
	ON_COMMAND(ID_GRID_SNAP, OnGridSnap)
	ON_UPDATE_COMMAND_UI(ID_GRID_SNAP, OnUpdateGridSnap)
	ON_COMMAND(ID_VIEW_BACK_COLOR, OnViewBackColor)
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnSelectAll)
	ON_COMMAND(ID_EDIT_SELECT_NONE, OnSelectNone)
	ON_UPDATE_COMMAND_UI(ID_MODIFY_TEXTURE_MAP, OnUpdateTextureMap)
	ON_UPDATE_COMMAND_UI(ID_MODIFY_MATERIAL, OnUpdateModifyMaterial)
	ON_COMMAND(ID_EDIT_SELECT_INVERSE, OnSelectInverse)
	ON_COMMAND(ID_MODIFY_UV_MAP, OnModifyUVMap)
	ON_UPDATE_COMMAND_UI(ID_MODIFY_UV_MAP, OnUpdateModifyUVMap)
	ON_COMMAND(ID_VIEW_SHADOWS, OnViewShadows)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHADOWS, OnUpdateViewShadows)
	ON_COMMAND(ID_VIEW_ANIMATELIGHT, OnViewAnimatelight)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ANIMATELIGHT, OnUpdateViewAnimatelight)
	ON_COMMAND(ID_VIEW_BUMPMAPS, OnViewBumpmaps)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BUMPMAPS, OnUpdateViewBumpmaps)
	ON_COMMAND(ID_VIEW_VERTEXSHADER, OnViewVertexshader)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VERTEXSHADER, OnUpdateViewVertexshader)
	ON_COMMAND(ID_VIEW_PIXELSHADER, OnViewPixelshader)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PIXELSHADER, OnUpdateViewPixelshader)
	ON_COMMAND(ID_VIEW_VISIBLESHADOWS, OnViewVisibleshadows)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VISIBLESHADOWS, OnUpdateViewVisibleshadows)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_PROP_SURFACE, OnSurfaceProperties)
	ON_UPDATE_COMMAND_UI(ID_PROP_SURFACE, OnUpdateSurfaceProperties)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// +--------------------------------------------------------------------+

static MagicView* magic_view = 0;

MagicView::MagicView()
   : video(0), video_settings(0), screen(0), scene(0),
     drag_left(false), drag_right(false), grid(0),
     main_light(0), back_light(0), view_shadows(true), view_bumpmaps(true),
     animate_light(false)
{
   window_style   = 0;
   is_minimized   = false;
   is_maximized   = false;
   is_sizing      = false;
   view_mode      = VIEW_ALL;

   main_win       = 0;
   view_win[0]    = 0;
   view_win[1]    = 0;
   view_win[2]    = 0;
   view_win[3]    = 0;
   model_view[0]  = 0;
   model_view[1]  = 0;
   model_view[2]  = 0;
   model_view[3]  = 0;
   uvmap_win      = 0;

   grid           = new Grid;
   magic_view     = this;

   Solid::EnableCollision(false);
}

MagicView::~MagicView()
{
   if (grid) {
      delete grid;
   }

   if (scene) {
      scene->Graphics().clear();
      delete scene;
   }

   if (screen)          delete screen;
   if (video)           delete video;
   if (video_settings)  delete video_settings;

   if (magic_view == this)
      magic_view = 0;
}

MagicView* MagicView::GetInstance()
{
   return magic_view;
}

BOOL MagicView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// +--------------------------------------------------------------------+
//
// MagicView diagnostics

#ifdef _DEBUG
void MagicView::AssertValid() const
{
	CView::AssertValid();
}

void MagicView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

MagicDoc* MagicView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(MagicDoc)));
	return (MagicDoc*)m_pDocument;
}
#endif //_DEBUG


// +--------------------------------------------------------------------+

void MagicView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
   Color::SetPalette(standard_palette, 256, inverse_palette);

   if (!video_settings)
      video_settings = new VideoSettings;

   GetClientRect(&client_rect);

   // Use client area to set video window size
   int w = client_rect.right - client_rect.left;
   int h = client_rect.bottom - client_rect.top;

   video_settings->is_windowed   = true;
   video_settings->window_width  = w;
   video_settings->window_height = h;

   if (!video) {
      video = new VideoDX9(GetSafeHwnd(), video_settings);
      *video_settings = *video->GetVideoSettings();

      if (video) {
         Color::UseVideo(video);
         video->UseXFont("System", 12, false, false);

         screen = new Screen(video);
         if (!screen) {
            ::Print("ERROR: Could not create Screen object.\n");
            return;
         }

         ::Print("  Created screen object (%d x %d).\n", w, h);
   
         if (!screen->SetBackgroundColor(Color::Black))
            ::Print("  WARNING: could not set video background color to Black\n");

         screen->ClearAllFrames(true);

         ::Print("  Established requested video parameters.\n");
         ::Print("  ---------------------------------------\n\n");

         if (!scene) {
            scene = new Scene;

            scene->SetAmbient(Color(60,60,60));

            Point light_pos(3e6, 5e6, 4e6);

            main_light = new Light(1.0f); //1.25f);
            main_light->MoveTo(light_pos);
            main_light->SetType(Light::LIGHT_DIRECTIONAL);
            main_light->SetColor(Color::White);
            main_light->SetShadow(true);

            scene->AddLight(main_light);

            back_light = new Light(0.5f);
            back_light->MoveTo(light_pos * -1);
            back_light->SetType(Light::LIGHT_DIRECTIONAL);
            back_light->SetColor(Color::White);

            scene->AddLight(back_light);

            Selection*  seln     = GetDocument()->GetSelection();
            Selector*   selector = GetDocument()->GetSelector();

            if (seln && selector) {
               scene->Graphics().clear();
               scene->AddGraphic(seln);
               scene->AddGraphic(selector);

               selector->UseModel(0);
            }
         }

         int   mins[2]     = { 0, 0 };
         float weights[2]  = { 1, 1 };

         main_win = new ActiveWindow(screen, 0, 0, w, h, 100, 0);
         main_win->UseLayout(2, 2, mins, mins, weights, weights);
         main_win->SetBackColor(Color::Gray);

         screen->AddWindow(main_win);

         DWORD view_types[] = { 
            ModelView::VIEW_PLAN,
            ModelView::VIEW_PROJECT,
            ModelView::VIEW_SIDE,
            ModelView::VIEW_FRONT
         };

         for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 2; col++) {
               int index = 2*row + col;

               ActiveWindow* win = new ActiveWindow(screen, 
                                                    col*w/2, 
                                                    row*h/2, 
                                                    w/2, 
                                                    h/2, 
                                                    101+index, 
                                                    WIN_BLACK_FRAME, 
                                                    main_win);

               win->SetCells(col, row, 1, 1);
               win->SetCellInsets(Insets(1,1,1,1));
               win->SetBackColor(Color(160,160,160));

               ModelView* mv = new ModelView(win, scene, view_types[index]);

               if (view_types[index] == ModelView::VIEW_PROJECT)
                  mv->SetFillMode(ModelView::FILL_TEXTURE);

               mv->UseGrid(grid);
               win->AddView(mv);

               view_win[index]   = win;
               model_view[index] = mv;
            }
         }

         view_win[0]->SetStyle(WIN_WHITE_FRAME);

         uvmap_win  = new ActiveWindow(screen, 0, 0, w, h, 110, WIN_BLACK_FRAME, main_win);
         uvmap_view = new UVMapView(uvmap_win);
         uvmap_win->AddView(uvmap_view);

         main_win->DoLayout();
      }

      else {
         ::Print("  Could not establish requested video parameters.\n");
         ::Print("  -----------------------------------------------\n\n");
      }
   }
}

// +--------------------------------------------------------------------+

void
MagicView::SetupModelViews()
{
   switch (view_mode) {
   case VIEW_ALL: {
         for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 2; col++) {
               int index = 2*row + col;

               ActiveWindow* win = view_win[index];

               win->Show();
               win->SetCells(col, row, 1, 1);
               win->SetCellInsets(Insets(1,1,1,1));
            }
         }

         uvmap_win->Hide();
         uvmap_win->SetCells(0,0,0,0);
      }
      break;

   case VIEW_TOP:
   case VIEW_SIDE:
   case VIEW_FRONT:
   case VIEW_PERSPECTIVE: {
         view_focus = view_mode;

         for (int i = 0; i < 4; i++) {
            ActiveWindow* win = view_win[i];

            if (i == view_mode) {
               win->Show();
               win->SetCells(0,0,2,2);
               win->SetStyle(WIN_WHITE_FRAME);
            }

            else {
               win->Hide();
               win->SetCells(0,0,0,0);
               win->SetStyle(WIN_BLACK_FRAME);
            }
         }

         uvmap_win->Hide();
         uvmap_win->SetCells(0,0,0,0);
      }
      break;

   case VIEW_UV_MAP: {
         view_focus = view_mode;

         for (int i = 0; i < 4; i++) {
            ActiveWindow* win = view_win[i];
            win->Hide();
            win->SetCells(0,0,0,0);
            win->SetStyle(WIN_BLACK_FRAME);
         }

         uvmap_win->Show();
         uvmap_win->SetCells(0,0,2,2);
      }
      break;

   default:
      break;
   }

   main_win->DoLayout();
}

void
MagicView::SetFocusModelView(int f)
{
   if (view_mode == VIEW_ALL) {
      view_focus = f;

      for (int row = 0; row < 2; row++) {
         for (int col = 0; col < 2; col++) {
            int index = 2*row + col;

            ActiveWindow* win = view_win[index];

            win->Show();
            win->SetCells(col, row, 1, 1);
            win->SetCellInsets(Insets(1,1,1,1));

            if (index == view_focus) {
               win->SetStyle(WIN_WHITE_FRAME);
            }
            else {
               win->SetStyle(WIN_BLACK_FRAME);
            }
         }
      }
   }
   else if (IsUVEdit()) {
      view_focus = view_mode;
   }
   else {
      view_mode  = f;
      view_focus = f;

      for (int i = 0; i < 4; i++) {
         ActiveWindow* win = view_win[i];

         if (i == view_mode) {
            win->Show();
            win->SetCells(0,0,2,2);
            win->SetStyle(WIN_WHITE_FRAME);
         }

         else {
            win->Hide();
            win->SetCells(0,0,0,0);
            win->SetStyle(WIN_BLACK_FRAME);
         }
      }
   }

   main_win->DoLayout();
}

int
MagicView::GetWinIndexByPoint(int x, int y)
{
   if (view_mode == VIEW_ALL) {
      for (int row = 0; row < 2; row++) {
         for (int col = 0; col < 2; col++) {
            int index = 2*row + col;

            ActiveWindow* win = view_win[index];
            if (win->GetRect().Contains(x, y))
               return index;
         }
      }
   }

   return view_mode;
}

ModelView*
MagicView::GetModelViewByIndex(int index)
{
   if (index >= 0 && index < 4) {
      return model_view[index];
   }

   return model_view[0];
}

// +--------------------------------------------------------------------+

void MagicView::OnPaint() 
{
	ValidateRect(0);
   OnRender();
}

void MagicView::OnDraw(CDC* dc) 
{
}

// +--------------------------------------------------------------------+

void MagicView::ResizeVideo()
{
   if (!video || !video_settings)   return;

   HRESULT  hr = S_OK;
   RECT     client_old;

   client_old = client_rect;

   // Update window properties
   GetClientRect(&client_rect);

   if (client_old.right   - client_old.left  !=
       client_rect.right  - client_rect.left ||
       client_old.bottom  - client_old.top   !=
       client_rect.bottom - client_rect.top) {

      // A new window size will require a new backbuffer
      // size, so the 3D structures must be changed accordingly.

      video_settings->is_windowed   = true;
      video_settings->window_width  = client_rect.right - client_rect.left;
      video_settings->window_height = client_rect.bottom - client_rect.top;

      ::Print("ResizeVideo() %d x %d\n", video_settings->window_width, video_settings->window_height);

      if (video) {
         video->Reset(video_settings);
      }
   }

   // save a copy of the device-specific video settings:
   if (video->GetVideoSettings()) {
      *video_settings = *video->GetVideoSettings();
   }

   if (screen)
      screen->Resize(video_settings->window_width, 
                     video_settings->window_height);

   video->InvalidateCache();
   video->SetShadowEnabled(view_shadows);
   video->SetBumpMapEnabled(view_bumpmaps);
}

// +--------------------------------------------------------------------+

void MagicView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
   CView::OnUpdate(pSender, lHint, pHint);

   Solid*      solid    = GetDocument()->GetSolid();
   Selection*  seln     = GetDocument()->GetSelection();
   Selector*   selector = GetDocument()->GetSelector();

   if (solid && scene) {
      scene->Graphics().clear();
      scene->AddGraphic(solid);
      scene->AddGraphic(seln);
      scene->AddGraphic(selector);
   }

   if (selector)
      selector->UseModel(solid->GetModel());
}

CPoint
MagicView::LPtoWP(const CPoint& p)
{
   CPoint      result;
   ModelView*  view     = GetModelViewByIndex(view_focus);
   CPoint      origin   = view->ProjectPoint(Vec3(0,0,0));
   double      scale    = view->GetFieldOfView() / 2;

   result.x = (LONG) (( p.x - origin.x) / scale);
   result.y = (LONG) ((-p.y + origin.y) / scale);

   return result;
}

// +--------------------------------------------------------------------+
//
// MagicView message handlers

void MagicView::OnEnterSizeMove() 
{
   is_sizing = true;
}

void MagicView::OnExitSizeMove()
{
   is_sizing = false;
   ResizeVideo();
}

void MagicView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

   window_style = GetWindowLong(m_hWnd, GWL_STYLE);

   if (nType == SIZE_MINIMIZED) {
      is_minimized = true;
      is_maximized = false;
   }

   else if (nType == SIZE_MAXIMIZED) {
      is_minimized = false;
      is_maximized = true;
      ResizeVideo();
   }

   else if (nType == SIZE_RESTORED) {
      if (is_maximized) {
         is_maximized = false;
         ResizeVideo();
      }

      else if (is_minimized) {
         is_minimized = false;
         ResizeVideo();
      }
      else if (!is_sizing) {
         // if this is not a resize due to dragging...
         ResizeVideo();
      }
      else {
         // If we're neither maximized nor minimized, the window size 
         // is changing by the user dragging the window edges.  In this 
         // case, we don't reset the device yet -- we wait until the 
         // user stops dragging, and a WM_EXITSIZEMOVE message comes.
      }
   }
}

// +--------------------------------------------------------------------+

void MagicView::OnRender() 
{
   if (!screen || !video) 
      return;

   double s = sin(timeGetTime() * 0.001);
   double c = cos(timeGetTime() * 0.001);

   // IF LIGHTS ANIMATED:
   if (animate_light) {
      Point light_pos(3e6*s, 5e6*c, 4e6*s);

      if (main_light) {
         main_light->SetType(Light::LIGHT_POINT);
         main_light->MoveTo(light_pos);
         main_light->SetType(Light::LIGHT_DIRECTIONAL);
      }

      if (back_light) {
         back_light->SetType(Light::LIGHT_POINT);
         back_light->MoveTo(light_pos * -1);
         back_light->SetType(Light::LIGHT_DIRECTIONAL);
      }
   }

   if (screen->Refresh()) {
      video->Present();
   }
   else {
      ::Print("ERROR: Screen refresh failed.\n");
   }
}

void MagicView::CloseUVEditor()
{
   if (IsUVEdit()) {
      MagicDoc*   doc      = GetDocument();
      Editor*     editor   = doc->GetEditor();
      Solid*      solid    = doc->GetSolid();

      if (editor && solid) {
         editor->UseModel(solid->GetModel());
         editor->Resegment();
      }
   }
}

// +--------------------------------------------------------------------+

void MagicView::OnViewAll() 
{
   CloseUVEditor();
   view_mode = VIEW_ALL;
   SetupModelViews();
}

void MagicView::OnUpdateViewAll(CCmdUI* pCmdUI) 
{
   if (pCmdUI)
      pCmdUI->SetCheck(view_mode == VIEW_ALL);
}

void MagicView::OnViewFront() 
{
   CloseUVEditor();
   view_mode = VIEW_FRONT;
   SetupModelViews();
}

void MagicView::OnUpdateViewFront(CCmdUI* pCmdUI) 
{
   if (pCmdUI)
      pCmdUI->SetCheck(view_mode == VIEW_FRONT);
}

void MagicView::OnViewPerspective() 
{
   CloseUVEditor();
   view_mode = VIEW_PERSPECTIVE;
   SetupModelViews();
}

void MagicView::OnUpdateViewPerspective(CCmdUI* pCmdUI) 
{
   if (pCmdUI)
      pCmdUI->SetCheck(view_mode == VIEW_PERSPECTIVE);
}

void MagicView::OnViewSide() 
{
   CloseUVEditor();
   view_mode = VIEW_SIDE;
   SetupModelViews();
}

void MagicView::OnUpdateViewSide(CCmdUI* pCmdUI) 
{
   if (pCmdUI)
      pCmdUI->SetCheck(view_mode == VIEW_SIDE);
}

void MagicView::OnViewTop() 
{
   CloseUVEditor();
   view_mode = VIEW_TOP;
   SetupModelViews();
}

void MagicView::OnUpdateViewTop(CCmdUI* pCmdUI) 
{
   if (pCmdUI)
      pCmdUI->SetCheck(view_mode == VIEW_TOP);
}

// +--------------------------------------------------------------------+

void MagicView::OnFileImport() 
{
   DWORD err = 0;
   char filename[256];
   filename[0] = '\0';
   CFileDialog ofd(TRUE, "mag");

   ofd.m_ofn.lpstrFilter = "All 3D Files\0*.mag; *.obj; *.3ds\0Magic Files (*.mag)\0*.mag\0Wavefront/OBJ Files (*.obj)\0*.obj\0003DS MAX Files (*.3ds)\0*.3ds\0\0";
   ofd.m_ofn.lpstrFile   = filename;
   ofd.m_ofn.nMaxFile    = sizeof(filename);

   if (ofd.DoModal() != IDOK)
      return;

   char mag_name[256];
   sprintf_s(mag_name, "%s", ofd.GetFileName().GetBuffer(0));

   MagicDoc*   pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   if (pDoc->ImportFile(mag_name)) {
      Invalidate();
      pDoc->SetModifiedFlag(TRUE);
      pDoc->UpdateAllViews(this);
   }
}

void MagicView::OnFileExport() 
{
   DWORD err = 0;
   char filename[256];
   filename[0] = '\0';
   CFileDialog ofd(FALSE, "mag");

   ofd.m_ofn.lpstrFilter = "All 3D Files\0*.mag; *.obj; *.3ds\0Magic Files (*.mag)\0*.mag\0Wavefront/OBJ Files (*.obj)\0*.obj\0003DS MAX Files (*.3ds)\0*.3ds\0\0";
   ofd.m_ofn.lpstrFile   = filename;
   ofd.m_ofn.nMaxFile    = sizeof(filename);

   if (ofd.DoModal() != IDOK)
      return;

   char mag_name[256];
   sprintf_s(mag_name, "%s", ofd.GetFileName().GetBuffer(0));

   MagicDoc*   pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   if (pDoc->ExportFile(mag_name)) {
      pDoc->SetModifiedFlag(FALSE);
      pDoc->UpdateAllViews(this);
   }
}

// +--------------------------------------------------------------------+

void MagicView::OnSurfaceProperties() 
{
   SurfacePropertiesDialog dlg(this);
   dlg.DoModal();
}

void MagicView::OnUpdateSurfaceProperties(CCmdUI* pCmdUI) 
{
   Solid*      solid = GetDocument()->GetSolid();
   pCmdUI->Enable(solid && solid->GetModel());
}

// +--------------------------------------------------------------------+

void MagicView::OnTextureMap() 
{
   TextureMapDialog dlg(this);
   if (dlg.DoModal() == IDOK) {
      MagicDoc*   doc      = GetDocument();
      Solid*      solid    = doc->GetSolid();
      Selection*  seln     = doc->GetSelection();
      Selector*   selector = doc->GetSelector();
      Editor*     editor   = doc->GetEditor();
      Material*   mtl      = 0;

      if (dlg.mMaterialIndex >= 0) {
         mtl = solid->GetModel()->GetMaterials()[dlg.mMaterialIndex];
      }

      editor->UseModel(solid->GetModel());
      editor->ApplyMaterial(mtl, seln->GetPolys(),
                      dlg.mMapType, 2-dlg.mAxis, (float) dlg.mScaleU, (float) dlg.mScaleV,
                      dlg.mFlip, dlg.mMirror, dlg.mRotate);

      selector->Reselect();

      Invalidate();
      doc->SetModifiedFlag(TRUE);
      doc->UpdateAllViews(this);
   }
}

void MagicView::OnUpdateTextureMap(CCmdUI* pCmdUI) 
{
   Solid*      solid = GetDocument()->GetSolid();
   Selection*  seln  = GetDocument()->GetSelection();

   pCmdUI->Enable(solid && solid->GetModel() && seln && seln->GetPolys().size() > 0);
}

void MagicView::OnModifyMaterial() 
{
   MaterialDialog dlg(this);
   dlg.DoModal();

   Invalidate();
   GetDocument()->SetModifiedFlag(TRUE);
   GetDocument()->UpdateAllViews(this);
}

void MagicView::OnUpdateModifyMaterial(CCmdUI* pCmdUI) 
{
   Solid* solid = GetDocument()->GetSolid();
   pCmdUI->Enable(solid && solid->GetModel());
}

void MagicView::OnModifyUVMap() 
{
   Selection*  seln  = GetDocument()->GetSelection();

   view_mode = VIEW_UV_MAP;
   SetupModelViews();

   if (seln && uvmap_view) {
      Poly* p = seln->GetPolys().first();

      if (p) {
         uvmap_view->UseMaterial(p->material);
         uvmap_view->UsePolys(seln->GetPolys());
      }
   }
}

void MagicView::OnUpdateModifyUVMap(CCmdUI* pCmdUI) 
{
   OnUpdateTextureMap(pCmdUI);
}

// +--------------------------------------------------------------------+

void MagicView::OnGridProperties() 
{
   GridProps dlg(grid, this);
   dlg.DoModal();
}

void MagicView::OnGridShow() 
{
   if (grid)
      grid->SetShow(!grid->IsShow());
}

void MagicView::OnGridSnap() 
{
   if (grid)
      grid->SetSnap(!grid->IsSnap());
}

void MagicView::OnUpdateGridSnap(CCmdUI* pCmdUI) 
{
   if (grid)
      pCmdUI->SetCheck(grid->IsSnap());
}

// +--------------------------------------------------------------------+

void MagicView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CView::OnLButtonDown(nFlags, point);
   SetCapture();

   // set focus to the view that was clicked:
   int index = GetWinIndexByPoint(point.x, point.y);
   SetFocusModelView(index);

   drag_start = point;
   drag_left  = true;
   drag_right = false;

   ModelView*  mv       = GetModelViewByIndex(index);
   MagicDoc*   pDoc     = GetDocument();
   Model*      model    = pDoc->GetSolid()->GetModel();
   Selector*   selector = pDoc->GetSelector();

   if (IsUVEdit()) {
      int select_mode = UVMapView::SELECT_APPEND;

      if (nFlags & MK_CONTROL)
         select_mode = UVMapView::SELECT_REMOVE;

      if (!uvmap_view->WillSelect(point)) {
         uvmap_view->Begin(select_mode);
         uvmap_view->AddMark(point);
      }
   }

   else if (mv && selector) {
      int select_mode = Selector::SELECT_APPEND;

      if (nFlags & MK_CONTROL)
         select_mode = Selector::SELECT_REMOVE;

      selector->Begin(model, mv->GetViewMode(), select_mode);
      selector->AddMark(point);
   }
}

void MagicView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CView::OnLButtonUp(nFlags, point);
   ReleaseCapture();

   drag_left = false;

   MagicDoc*   pDoc        = GetDocument();
   Selector*   selector    = pDoc->GetSelector();

   if (IsUVEdit())
      uvmap_view->End();

   else if (selector && selector->IsActive())
      selector->End();
}

void MagicView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CView::OnLButtonDblClk(nFlags, point);

   drag_left = false;

   MagicDoc*   pDoc        = GetDocument();
   Selector*   selector    = pDoc->GetSelector();

   if (IsUVEdit())
      uvmap_view->Clear();

   else if (selector)
      selector->Clear();
}

void MagicView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CView::OnRButtonDown(nFlags, point);
   SetCapture();

   // set focus to the view that was clicked:
   int index = GetWinIndexByPoint(point.x, point.y);
   SetFocusModelView(index);

   drag_start = point;
   drag_left  = false;
   drag_right = true;
}

void MagicView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CView::OnRButtonUp(nFlags, point);
   ReleaseCapture();

   drag_right = false;
}

void MagicView::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	CView::OnRButtonDblClk(nFlags, point);
   ReleaseCapture();

   drag_right = false;

   if (view_mode == VIEW_ALL) {
      view_mode = view_focus;
   }
   else if (IsUVEdit()) {
      CloseUVEditor();
      view_mode = VIEW_ALL;
      SetupModelViews();
   }
   else {
      view_mode = VIEW_ALL;
   }

   SetFocusModelView(view_focus);
}

void MagicView::OnMouseMove(UINT nFlags, CPoint point) 
{
   if (drag_right) {
      CPoint offset = point - drag_start;

      if (view_focus == VIEW_PERSPECTIVE) {
         ModelView* view = GetModelViewByIndex(view_focus);
         view->SpinBy(offset.x * 0.5 * DEGREES, 
                      offset.y * 0.5 * DEGREES);
      }

      else if (IsUVEdit()) {
         uvmap_view->MoveBy(offset.x, offset.y);
      }

      else {
         ModelView* view = GetModelViewByIndex(view_focus);
         view->MoveBy(offset.x, offset.y);
      }

      drag_start = point;
      Invalidate();
   }

   else if (drag_left) {
      CPoint      offset      = point - drag_start;
      MagicDoc*   pDoc        = GetDocument();
      Selector*   selector    = pDoc->GetSelector();

      if (IsUVEdit()) {
         if (uvmap_view->IsActive()) {
            uvmap_view->AddMark(point);
         }
         else {
            uvmap_view->DragBy(offset.x, offset.y);
            drag_start = point;
         }
      }

      else if (selector && selector->IsActive()) {
         selector->AddMark(point);
      }
   }

   // xy status message:
   if (view_focus != VIEW_PERSPECTIVE) {
      char        xy[80];
      CPoint      mouse    = LPtoWP(point);
      Selection*  seln     = GetDocument()->GetSelection();

      int nv = seln ? seln->GetVerts().size() : 0;
      int np = seln ? seln->GetPolys().size() : 0;

      if (np || nv)
         sprintf_s(xy, "(%05d,%05d)  Verts:%d Polys:%d", mouse.x, mouse.y, nv, np);
      else
         sprintf_s(xy, "(%05d,%05d)", mouse.x, mouse.y);
      MainFrame::StatusXY(xy);
   }

	CView::OnMouseMove(nFlags, point);
}

BOOL MagicView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
   if (view_focus == VIEW_PERSPECTIVE) {
      ModelView* view = GetModelViewByIndex(view_focus);

      if (view) {
         Camera*  cam = view->GetCamera();
         Point    pos = cam->Pos();
         double   len = pos.length();

         if (zDelta < 0) {
            if (len < 10000)
               pos *= 1.15;
         }
         else {
            if (len > 0.10)
               pos *= 0.85;
         }

         cam->MoveTo(pos);
      }
   }

   else if (IsUVEdit()) {
      if (zDelta < 0) {
         uvmap_view->ZoomOut();
      }
      else {
         uvmap_view->ZoomIn();
      }
   }

   else {
      if (zDelta > 0)
         OnViewZoomIn();
      else
         OnViewZoomOut();
   }

   return 0;
}

// +--------------------------------------------------------------------+

void MagicView::OnViewZoomNormal() 
{
   for (int i = 0; i < 4; i++) {
      ModelView* view = GetModelViewByIndex(i);

      if (view) {
         view->ZoomNormal();
      }
   }
}

void MagicView::OnViewZoomIn() 
{
   for (int i = 0; i < 4; i++) {
      ModelView* view = GetModelViewByIndex(i);

      if (view && view->GetViewMode() != ModelView::VIEW_PROJECT) {
         double fov = view->GetFieldOfView() * 1.15;
         view->SetFieldOfView(fov);
      }
   }
}

void MagicView::OnViewZoomOut() 
{
   for (int i = 0; i < 4; i++) {
      ModelView* view = GetModelViewByIndex(i);

      if (view && view->GetViewMode() != ModelView::VIEW_PROJECT) {
         double fov = view->GetFieldOfView() * 0.85;
         view->SetFieldOfView(fov);
      }
   }
}

// +--------------------------------------------------------------------+

void MagicView::OnViewModeWireframe() 
{
   ModelView* view = GetModelViewByIndex(view_focus);

   if (view) {
      view->SetFillMode(ModelView::FILL_WIRE);
   }
}

void MagicView::OnViewModeSolid() 
{
   ModelView* view = GetModelViewByIndex(view_focus);

   if (view) {
      view->SetFillMode(ModelView::FILL_SOLID);
   }
}

void MagicView::OnViewModeTextured() 
{
   ModelView* view = GetModelViewByIndex(view_focus);

   if (view) {
      view->SetFillMode(ModelView::FILL_TEXTURE);
   }
}

void MagicView::OnViewBackColor() 
{
   ModelView* view = GetModelViewByIndex(view_focus);

   if (view) {
      ActiveWindow*  win   = (ActiveWindow*) view->GetWindow();
      Color          c     = win->GetBackColor();
      COLORREF       crgb  = RGB(c.Red(), c.Green(), c.Blue());
      CColorDialog   chooser(crgb);

      if (chooser.DoModal() == IDOK) {
         crgb = chooser.GetColor();
         win->SetBackColor( Color(GetRValue(crgb), GetGValue(crgb), GetBValue(crgb)) );
      }
   }
}

// +--------------------------------------------------------------------+

void MagicView::OnSelectAll() 
{
   Solid*      solid    = GetDocument()->GetSolid();
   Selector*   selector = GetDocument()->GetSelector();

   if (IsUVEdit()) {
      uvmap_view->SelectAll();
   }

   else if (solid && selector) {
      selector->UseModel(solid->GetModel());
      selector->SelectAll(Selector::SELECT_APPEND);
   }
}

void MagicView::OnSelectNone() 
{
   Solid*      solid    = GetDocument()->GetSolid();
   Selector*   selector = GetDocument()->GetSelector();

   if (IsUVEdit()) {
      uvmap_view->SelectNone();
   }

   else if (solid && selector) {
      selector->UseModel(solid->GetModel());
      selector->SelectAll(Selector::SELECT_REMOVE);
   }
}

void MagicView::OnSelectInverse() 
{
   Solid*      solid    = GetDocument()->GetSolid();
   Selector*   selector = GetDocument()->GetSelector();

   if (IsUVEdit()) {
      uvmap_view->SelectInverse();
   }

   else if (solid && selector) {
      selector->UseModel(solid->GetModel());
      selector->SelectInverse();
   }
}

void MagicView::OnViewShadows() 
{
   view_shadows = !view_shadows;

   if (video)
      video->SetShadowEnabled(view_shadows);
}

void MagicView::OnUpdateViewShadows(CCmdUI* pCmdUI) 
{
   pCmdUI->SetCheck(view_shadows);
}

void MagicView::OnViewAnimatelight() 
{
   animate_light = !animate_light;
}

void MagicView::OnUpdateViewAnimatelight(CCmdUI* pCmdUI) 
{
   pCmdUI->SetCheck(animate_light);
}

void MagicView::OnViewBumpmaps() 
{
	view_bumpmaps = !view_bumpmaps;

   if (video)
      video->SetBumpMapEnabled(view_bumpmaps);
}

void MagicView::OnUpdateViewBumpmaps(CCmdUI* pCmdUI) 
{
   pCmdUI->SetCheck(view_bumpmaps);
}

void MagicView::OnViewVertexshader() 
{
   if (video) {
      VideoSettings* vs = (VideoSettings*) video->GetVideoSettings();
      vs->enable_vs = !vs->enable_vs;
   }
}

void MagicView::OnUpdateViewVertexshader(CCmdUI* pCmdUI) 
{
   if (video)
      pCmdUI->SetCheck(video->GetVideoSettings()->enable_vs);
}

void MagicView::OnViewPixelshader() 
{
   if (video) {
      VideoSettings* vs = (VideoSettings*) video->GetVideoSettings();
      vs->enable_ps = !vs->enable_ps;
   }
}

void MagicView::OnUpdateViewPixelshader(CCmdUI* pCmdUI) 
{
   if (video)
      pCmdUI->SetCheck(video->GetVideoSettings()->enable_ps);
}

void MagicView::OnViewVisibleshadows() 
{
   Shadow::SetVisibleShadowVolumes(!Shadow::GetVisibleShadowVolumes());
}

void MagicView::OnUpdateViewVisibleshadows(CCmdUI* pCmdUI) 
{
   pCmdUI->SetCheck(Shadow::GetVisibleShadowVolumes());
}

void MagicView::OnEditUndo() 
{
   MagicDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
   pDoc->Undo();

   Solid*      solid    = GetDocument()->GetSolid();
   Selector*   selector = GetDocument()->GetSelector();

   if (selector) {
      selector->UseModel(solid->GetModel());
      selector->Reselect();
   }

   Invalidate();
   pDoc->SetModifiedFlag(TRUE);
   pDoc->UpdateAllViews(this);
}

void MagicView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
   MagicDoc* pDoc = GetDocument();

   if (pDoc->NumUndo() > 0) {
      pCmdUI->Enable(TRUE);
      pCmdUI->SetText(CString("Undo ") + pDoc->GetUndoName() + CString("\tCtrl+Z"));
   }
   else {
      pCmdUI->Enable(FALSE);
      pCmdUI->SetText("Can't Undo\tCtrl+Z");
   }
}

void MagicView::OnEditRedo() 
{
   MagicDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
   pDoc->Redo();

   Solid*      solid    = GetDocument()->GetSolid();
   Selector*   selector = GetDocument()->GetSelector();

   if (selector) {
      selector->UseModel(solid->GetModel());
      selector->Reselect();
   }

   Invalidate();
   pDoc->SetModifiedFlag(TRUE);
   pDoc->UpdateAllViews(this);
}

void MagicView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
   MagicDoc* pDoc = GetDocument();

   if (pDoc->NumRedo() > 0) {
      pCmdUI->Enable(TRUE);
      pCmdUI->SetText(CString("Redo ") + pDoc->GetRedoName() + CString("\tCtrl+Y"));
   }
   else {
      pCmdUI->Enable(FALSE);
      pCmdUI->SetText("Can't Redo");
   }
}
