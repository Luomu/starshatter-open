/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         UVMapView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Implementation of the UVMapView class
*/

#include "stdafx.h"
#include "Magic.h"

#include "MagicDoc.h"
#include "UVMapView.h"
#include "Selector.h"
#include "Selection.h"

#include "ActiveWindow.h"
#include "Color.h"
#include "Screen.h"
#include "Video.h"

DWORD GetRealTime();

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// +--------------------------------------------------------------------+

UVMapView::UVMapView(Window* c)
   : View(c), material(0), zoom(1), x_offset(0), y_offset(0),
     nmarks(0), select_mode(SELECT_REPLACE), active(false)
{
}

UVMapView::~UVMapView()
{
}

// +--------------------------------------------------------------------+

const int BATCH_SIZE = 64;

void 
UVMapView::Refresh()
{
   video = Video::GetInstance();
   if (!video)
      return;

   window->FillRect(window->GetRect(), Color::LightGray);

   if (material && material->tex_diffuse) {
      Bitmap*  bmp = material->tex_diffuse;
      int      w   = bmp->Width();
      int      h   = bmp->Height();

      double   cx  = window->Width()  / 2 + x_offset;
      double   cy  = window->Height() / 2 + y_offset;

      int      x1  = (int) (cx - (zoom * w/2));
      int      x2  = (int) (cx + (zoom * w/2));
      int      y1  = (int) (cy - (zoom * h/2));
      int      y2  = (int) (cy + (zoom * h/2));

      window->DrawBitmap(x1, y1, x2, y2, bmp);

      ListIter<Poly> iter = polys;
      while (++iter) {
         Poly*       p     = iter.value();
         VertexSet*  vset  = p->vertex_set;

         if (p->material != material)
            continue;

         for (int i = 0; i < p->nverts; i++) {
            int    n1   = p->verts[i];
            int    n2   = p->verts[0];
            if (i < p->nverts-1)
               n2 = p->verts[i+1];

            double tu1  = vset->tu[n1];
            double tv1  = vset->tv[n1];
            double tu2  = vset->tu[n2];
            double tv2  = vset->tv[n2];

            x1 = (int) (cx + zoom * w * (tu1-0.5));
            x2 = (int) (cx + zoom * w * (tu2-0.5));
            y1 = (int) (cy + zoom * h * (tv1-0.5));
            y2 = (int) (cy + zoom * h * (tv2-0.5));

            window->DrawLine(x1,   y1,   x2,   y2,   Color::Yellow);

            if (IsSelected(p, i))
               window->FillRect(x1-3, y1-3, x1+3, y1+3, Color::Yellow);
            else
               window->DrawRect(x1-2, y1-2, x1+2, y1+2, Color::Yellow);
         }
      }
   }

   if (active && nmarks > 1) {
      float points[4*BATCH_SIZE + 4];

      for (int batch = 0; batch < nmarks; batch += BATCH_SIZE) {
         int end = batch + BATCH_SIZE;
         if (end > nmarks-1)
            end = nmarks-1;
         int nlines = end-batch;

         for (int i = 0; i < nlines; i++) {
            points[4*i+0] = (float) marks[batch + i].x;
            points[4*i+1] = (float) marks[batch + i].y;
            points[4*i+2] = (float) marks[batch + i + 1].x;
            points[4*i+3] = (float) marks[batch + i + 1].y;
         }

         video->DrawScreenLines(nlines, points, Color::Cyan);
      }
   }

   const char* title = "UV Editor";

   int  len = strlen(title);
   Rect r(6,4,200,20);

   r.x += window->GetRect().x;
   r.y += window->GetRect().y;

   video->DrawText(title, len, r, DT_LEFT|DT_SINGLELINE, Color::Black);

   r.x--;
   r.y--;

   video->DrawText(title, len, r, DT_LEFT|DT_SINGLELINE, Color::White);
}

// +--------------------------------------------------------------------+

void
UVMapView::UseMaterial(Material* m)
{
   if (material != m) {
      material = m;
      zoom     = 1;
      x_offset = 0;
      y_offset = 0;
   }
}

void 
UVMapView::UsePolys(List<Poly>& p)
{
   polys.clear();
   polys.append(p);
}

void
UVMapView::MoveBy(double dx, double dy)
{
   x_offset += dx;
   y_offset += dy;
}

void
UVMapView::DragBy(double dx, double dy)
{
   if (!material || !material->tex_diffuse || selverts.size() < 1)
      return;

   Bitmap*  bmp = material->tex_diffuse;
   double   w   = zoom * bmp->Width();
   double   h   = zoom * bmp->Height();
   float    du  = (float) (dx/w);
   float    dv  = (float) (dy/h);

   for (auto svi = selverts.begin(); svi != selverts.end(); ++svi) {
      DWORD value = *svi;
      DWORD p     = value >> 16;
      DWORD n     = value & 0xffff;

      Poly* poly = polys[p];
      if (poly && n < poly->nverts) {
         VertexSet*  vset = poly->vertex_set;
         int         v    = poly->verts[n];

         vset->tu[v] += du;
         vset->tv[v] += dv;
      }
   }
}

// +----------------------------------------------------------------------+

void
UVMapView::Clear()
{
   selverts.clear();
}

void
UVMapView::Begin(int seln_mode)
{
   nmarks      = 0;
   select_mode = seln_mode;
   active      = true;
}

void
UVMapView::AddMark(CPoint& p)
{
   if (nmarks < MAX_MARK)
      marks[nmarks++] = p;
}

void
UVMapView::End()
{
   active = false;

   // get the model:
   if (!nmarks || !material || !material->tex_diffuse) return;
   
   // if not adding to selection:
   if (select_mode == SELECT_REPLACE) {
      Clear();
   }

   Bitmap*  bmp = material->tex_diffuse;
   int      w   = bmp->Width();
   int      h   = bmp->Height();

   double   cx  = window->Width()  / 2 + x_offset;
   double   cy  = window->Height() / 2 + y_offset;


   // if only one mark:
   if (nmarks < 2) {
      // find all selected verts:
      ListIter<Poly> iter = polys;
      while (++iter) {
         Poly*       p     = iter.value();
         VertexSet*  vset  = p->vertex_set;

         for (int i = 0; i < p->nverts; i++) {
            int    n1   = p->verts[i];
            double tu1  = vset->tu[n1];
            double tv1  = vset->tv[n1];

            int x1 = (int) (cx + zoom * w * (tu1-0.5));
            int y1 = (int) (cy + zoom * h * (tv1-0.5));

            int dx = abs(marks[0].x - x1);
            int dy = abs(marks[0].y - y1);

            if (dx < 4 && dy < 4) {
               WORD  p_index = iter.index();
               DWORD value   = (p_index << 16) | i;

               if (select_mode == SELECT_REMOVE) {
				   for (auto svi = selverts.begin(); svi != selverts.end(); ++svi) {
					   if (*svi == value) {
						   selverts.erase(svi);
					   }
				   }
               }
               else {
				   bool contains = false;
				   for (auto svi = selverts.begin(); svi != selverts.end(); ++svi) {
					   if (*svi == value) {
							contains = true;
					   }
				   }
				   if (!contains)
					   selverts.push_back(value);
               }
            }
         }
      }
   }

   // otherwise, build a region:
   else {
      CRgn rgn;
      rgn.CreatePolygonRgn(marks, nmarks, WINDING);

      // find all selected verts:
      ListIter<Poly> iter = polys;
      while (++iter) {
         Poly*       p     = iter.value();
         VertexSet*  vset  = p->vertex_set;

         for (int i = 0; i < p->nverts; i++) {
            int    n1   = p->verts[i];
            double tu1  = vset->tu[n1];
            double tv1  = vset->tv[n1];

            int x1 = (int) (cx + zoom * w * (tu1-0.5));
            int y1 = (int) (cy + zoom * h * (tv1-0.5));

            CPoint p(x1,y1);

            if (rgn.PtInRegion(p)) {
               WORD  p_index = iter.index();
               DWORD value   = (p_index << 16) | i;

               if (select_mode == SELECT_REMOVE) {
				   for (auto svi = selverts.begin(); svi != selverts.end(); ++svi) {
					   if (*svi == value)
						   selverts.erase(svi);
				   }
               }
               else {
				   bool contains = false;
				   for (auto svi = selverts.begin(); svi != selverts.end(); ++svi) {
					   if (*svi == value)
						   contains = true;
				   }
				   if (!contains)
					   selverts.push_back(value);
               }
            }
         }
      }
   }

   nmarks = 0;
}

// +----------------------------------------------------------------------+

void
UVMapView::SelectAll()
{
   selverts.clear();

   ListIter<Poly> iter = polys;
   while (++iter) {
      Poly* p = iter.value();

      if (p->material != material)
         continue;

      for (int i = 0; i < p->nverts; i++) {
         WORD  p_index = iter.index();
         DWORD value   = (p_index << 16) | i;
		 selverts.push_back(value);
      }
   }
}

void
UVMapView::SelectNone()
{
   selverts.clear();
}

void
UVMapView::SelectInverse()
{
   ListIter<Poly> iter = polys;
   while (++iter) {
      Poly* p = iter.value();

      if (p->material != material)
         continue;

      for (int i = 0; i < p->nverts; i++) {
         WORD  p_index = iter.index();
         DWORD value   = (p_index << 16) | i;

		 bool contains = false;
		 auto svi = selverts.begin();
		 for (; svi != selverts.end(); ++svi) {
			 if (*svi == value) contains = true;
			 break;
		 }

         if (contains)
            selverts.erase(svi);
         else
			 selverts.push_back(value);
      }
   }
}

bool
UVMapView::IsSelected(Poly* poly, WORD v)
{
   WORD p = polys.index(poly);
   DWORD value = (p << 16) | v;

   bool contains = false;
	
	for (auto svi = selverts.begin(); svi != selverts.end(); ++svi) {
		return true;
	}

   return false;
}

bool
UVMapView::WillSelect(CPoint& p)
{
   if (!material || !material->tex_diffuse)
      return false;
   
   Bitmap*  bmp = material->tex_diffuse;
   int      w   = bmp->Width();
   int      h   = bmp->Height();

   double   cx  = window->Width()  / 2 + x_offset;
   double   cy  = window->Height() / 2 + y_offset;

   // find first selected vert:
   ListIter<Poly> iter = polys;
   while (++iter) {
      Poly*       poly  = iter.value();
      VertexSet*  vset  = poly->vertex_set;

      for (int i = 0; i < poly->nverts; i++) {
         int    n1   = poly->verts[i];
         double tu1  = vset->tu[n1];
         double tv1  = vset->tv[n1];

         int x1 = (int) (cx + zoom * w * (tu1-0.5));
         int y1 = (int) (cy + zoom * h * (tv1-0.5));

         int dx = abs(p.x - x1);
         int dy = abs(p.y - y1);

         if (dx < 4 && dy < 4) {
            return true;
         }
      }
   }

   return false;
}
