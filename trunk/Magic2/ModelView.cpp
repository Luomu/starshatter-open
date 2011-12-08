/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         ModelView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Implementation of the ModelView class
*/

#include "stdafx.h"
#include "Magic.h"

#include "MagicDoc.h"
#include "ModelView.h"
#include "Grid.h"
#include "Selector.h"
#include "Selection.h"

#include "ActiveWindow.h"
#include "Color.h"
#include "Light.h"
#include "Scene.h"
#include "Screen.h"
#include "Shadow.h"
#include "Solid.h"
#include "Video.h"

DWORD GetRealTime();

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static ModelView* views[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

// +--------------------------------------------------------------------+

ModelView::ModelView(Window* c, Scene* s, DWORD m)
   : CameraView(c, 0, s), view_mode(100), fill_mode(FILL_WIRE),
     grid(0), az(-PI/4), el(PI/4)
{
   UseCamera(&cam);
   SetViewMode(m);
}

ModelView::~ModelView()
{
   if (views[view_mode] == this)
      views[view_mode] = 0;
}

// +--------------------------------------------------------------------+

void 
ModelView::SetViewMode(DWORD m)
{
   if (view_mode != m) {
      view_mode = m;

      double x = 1e3 * sin(az) * cos(el);
      double y = 1e3 * cos(az) * cos(el);
      double z = 1e3 *           sin(el);

      switch (view_mode) {
      case VIEW_PLAN:
               cam.LookAt(Point(0,0,0),
                            Point(0,1e3,0),
                            Point(0,0,1));
               SetProjectionType(Video::PROJECTION_ORTHOGONAL);
               SetFieldOfView(1.75);
               break;

      case VIEW_FRONT:
               cam.LookAt(Point(0,0,0),
                            Point(0,0,1e3),
                            Point(0,1,0));
               SetProjectionType(Video::PROJECTION_ORTHOGONAL);
               SetFieldOfView(1.75);
               break;

      case VIEW_SIDE:
               cam.LookAt(Point(0,0,0),
                            Point(1e3,0,0),
                            Point(0,1,0));
               SetProjectionType(Video::PROJECTION_ORTHOGONAL);
               SetFieldOfView(1.75);
               break;

      case VIEW_PROJECT:
               cam.LookAt(Point(0,0,0),
                            Point(x,z,y),
                            Point(0,1,0));
               SetProjectionType(Video::PROJECTION_PERSPECTIVE);
               SetFieldOfView(1.75);
               break;
      }

      views[view_mode] = this;
   }
}

ModelView*
ModelView::FindView(DWORD mode)
{
   if (mode >= 0 && mode < 8)
      return views[mode];

   return 0;
}

// +--------------------------------------------------------------------+

void 
ModelView::MoveTo(Point pos)
{
   switch (view_mode) {
   case VIEW_PLAN:
            cam.LookAt(  pos - Point(0,1e3,0),
                         pos,
                         Point(0,0,1));
            break;

   case VIEW_FRONT:
            cam.LookAt(  pos - Point(0,0,1e3),
                         pos,
                         Point(0,1,0));
            break;

   case VIEW_SIDE:
            cam.LookAt(  pos - Point(1e3,0,0),
                         pos,
                         Point(0,1,0));
            break;
   }
}

void 
ModelView::MoveBy(double dx, double dy)
{
   Point pos = cam.Pos();

   dx *= 2.00/GetFieldOfView();
   dy *= 2.00/GetFieldOfView();

   switch (view_mode) {
   case VIEW_PLAN:
            pos.x -= dx;
            pos.z += dy;

            cam.LookAt(  pos - Point(0,1e3,0),
                         pos,
                         Point(0,0,1));
            break;

   case VIEW_FRONT:
            pos.x += dx;
            pos.y += dy;

            cam.LookAt(  pos - Point(0,0,1e3),
                         pos,
                         Point(0,1,0));
            break;

   case VIEW_SIDE:
            pos.z -= dx;
            pos.y += dy;

            cam.LookAt(  pos - Point(1e3,0,0),
                         pos,
                         Point(0,1,0));
            break;
   }
}

void 
ModelView::SpinBy(double phi, double theta)
{
   const double limit = (0.43 * PI);

   Point  pos = cam.Pos();
   double len = pos.length();

   az += phi;
   el += theta;

   if (az > PI)
      az = -2*PI + az;

   else if (az < -PI)
      az = 2*PI + az;

   if (el > limit)
      el = limit;
   else if (el < -limit)
      el = -limit;

   double x = len * sin(az) * cos(el);
   double y = len * cos(az) * cos(el);
   double z = len *           sin(el);

   cam.LookAt(Point(0,0,0), Point(x,z,y), Point(0,1,0));
}

void
ModelView::UseGrid(Grid* g)
{
   grid = g;
}

void
ModelView::ZoomNormal()
{
   DWORD v = view_mode; // remember current view mode
   view_mode = 100;     // force set view mode to setup the view 
   az = -PI/4;          // if this happens to be a perspective view, 
   el =  PI/4;          // reset the spin to the original 3/4ths view

   SetViewMode(v);      // restore default view params for this mode
}

// +--------------------------------------------------------------------+

void
ModelView::RenderScene()
{
   // activate all lights:
   ListIter<Light> light_iter = scene->Lights();

   while (++light_iter) {
      Light* light = light_iter.value();
      light->SetActive(true);
   }

   video->SetLights(scene->Lights());

   RenderGrid();


   bool old_shadows = video->IsShadowEnabled();
   bool old_bumps   = video->IsBumpMapEnabled();

   if (fill_mode != FILL_TEXTURE) {
      video->SetShadowEnabled(false);
      video->SetBumpMapEnabled(false);
      CameraView::RenderScene();
   }

   else {
      CameraView::RenderScene();
   }

   const char* title = "ModelView";
   switch (view_mode) {
   case VIEW_PLAN:      title = "Top";          break;
   case VIEW_FRONT:     title = "Front";        break;
   case VIEW_SIDE:      title = "Right Side";   break;
   case VIEW_PROJECT:   title = "Perspective";  break;
   }

   int  len = strlen(title);
   Rect r(6,4,200,20);

   r.x += window->GetRect().x;
   r.y += window->GetRect().y;

   video->DrawText(title, len, r, DT_LEFT|DT_SINGLELINE, Color::Black);

   r.x--;
   r.y--;

   video->DrawText(title, len, r, DT_LEFT|DT_SINGLELINE, Color::White);
   video->SetShadowEnabled(old_shadows);
   video->SetBumpMapEnabled(old_bumps);
}

// +--------------------------------------------------------------------+

void 
ModelView::Render(Graphic* g, DWORD flags)
{
   if (!g)
      return;

   if (!strcmp(g->Name(), "Selector")) {
      Selector* selector = (Selector*) g;

      if (selector->GetViewMode() != (int) view_mode)
         return;
   }

   else if (!strcmp(g->Name(), "Selection")) {
      Selection* selection = (Selection*) g;
      selection->UseView(this);
   }

   if (fill_mode == FILL_WIRE) {
      Material m;
      video->UseMaterial(&m);
      video->SetRenderState(Video::Z_ENABLE, FALSE);

      if (g->IsSolid()) {
         m.Kd        = ColorValue(0.0f, 0.0f, 0.0f, 0.5f);
         m.Ke        = ColorValue(0.8f, 0.8f, 0.9f, 1.0f);
         m.blend     = Video::BLEND_ALPHA;
         m.luminous  = true;

         video->SetRenderState(Video::FILL_MODE, Video::FILL_SOLID);
         g->Render(video, flags);
      }

      m.Ka     = Color::Black;
      m.Kd     = Color::Black;
      m.Ks     = Color::Black;
      m.Ke     = Color::Black;
      m.blend  = Video::BLEND_SOLID;

      video->SetRenderState(Video::FILL_MODE, Video::FILL_WIREFRAME);

      g->Render(video, flags);
      video->UseMaterial(0);
   }

   else if (fill_mode == FILL_SOLID) {
      if (g->IsSolid() && flags != Graphic::RENDER_SOLID)
         return;

      Material m;
      video->UseMaterial(&m);
      video->SetRenderState(Video::Z_ENABLE, TRUE);

      m.Kd     = ColorValue(0.5f, 0.5f, 0.5f, 1.0f);
      m.blend  = Video::BLEND_SOLID;

      video->SetRenderState(Video::FILL_MODE, Video::FILL_SOLID);

      if (g->IsSolid()) {
         MarkVisibleLights(g, flags);
         video->SetLights(scene->Lights());
      }

      g->Render(video, flags);
      video->UseMaterial(0);
   }

   else if (fill_mode == FILL_TEXTURE) {
      video->SetRenderState(Video::Z_ENABLE, TRUE);
      video->UseMaterial(0);

      if (g->IsSolid()) {
         MarkVisibleLights(g, flags);
         video->SetLights(scene->Lights());
      }

      g->Render(video, flags);
   }
}

void
ModelView::RenderGrid()
{
   if (!grid || !grid->IsShow())
      return;

   int   plane = Grid::GRID_XZ;

   if (view_mode == VIEW_FRONT)
      plane = Grid::GRID_XY;

   else if (view_mode == VIEW_SIDE)
      plane = Grid::GRID_YZ;

   grid->ShowPlane(plane);
   grid->MoveTo(camera_loc * -1);
   grid->ShowReference(view_mode != VIEW_PROJECT);
   grid->Render(video, Graphic::RENDER_SOLID);

   video->SetRenderState(Video::Z_ENABLE,          TRUE);
   video->SetRenderState(Video::LIGHTING_ENABLE,   TRUE);
   video->UseMaterial(0);
}

// +--------------------------------------------------------------------+

CPoint
ModelView::ProjectPoint(Vec3& p)
{
   CPoint result;

   switch (view_mode) {
   case VIEW_PLAN:
   case VIEW_FRONT:
   case VIEW_SIDE:
   case VIEW_PROJECT: {
         Vec3  pt = p;
         projector.Transform(pt);
         projector.Project(pt, false);
         
         result.x = (LONG) (pt.x + GetWindow()->X());
         result.y = (LONG) (pt.y + GetWindow()->Y());
      }
      break;

   default:
      result.x = 0;
      result.y = 0;
      break;
   }
   
   return result;
}
