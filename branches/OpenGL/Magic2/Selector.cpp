/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         Selector.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Source file for implementation of Selector
*/


#include "stdafx.h"
#include "Selector.h"
#include "MagicDoc.h"
#include "ModelView.h"
#include "Selection.h"

#include "Solid.h"

// +----------------------------------------------------------------------+

Selector::Selector(Selection* s)
   : nmarks(0), view_mode(0), select_mode(SELECT_REPLACE), model(0)
{
   if (s) {
      selection      = s;
      own_selection  = false;
   }
   else {
      selection      = new Selection;
      own_selection  = true;
   }

   strcpy_s(name, "Selector");
}

Selector::~Selector()
{
   if (selection && own_selection)
      delete selection;
}

// +----------------------------------------------------------------------+

const int BATCH_SIZE = 64;

void
Selector::Render(Video* video, DWORD flags)
{
   if (nmarks < 2 || flags == Graphic::RENDER_SOLID)
      return;

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

// +----------------------------------------------------------------------+

void
Selector::Clear()
{
   if (selection)
      selection->Clear();
}

void
Selector::Begin(Model* m, int mode, int seln_mode)
{
   nmarks      = 0;
   model       = m;
   view_mode   = mode;
   select_mode = seln_mode;
}

void
Selector::AddMark(CPoint& p)
{
   if (nmarks < MAX_MARK)
      marks[nmarks++] = p;
}

void
Selector::End()
{
   ModelView* view = ModelView::FindView(view_mode);
   view_mode = 0;
   
   // get the model:
   if (!model || !nmarks || !view) return;
   
   // if not adding to selection:
   if (select_mode == SELECT_REPLACE) {
      Clear();
   }

   // if only one mark:
   if (nmarks < 2) {
      CPoint pts[Poly::MAX_VERTS];

      // find all selected polys:
      ListIter<Surface> s_iter = model->GetSurfaces();
      while (++s_iter) {
         Surface* s = s_iter.value();

         if (s->IsHidden() || s->IsLocked() || s->IsSimplified())
            continue;

         for (int i = 0; i < s->NumPolys(); i++) {
            Poly* poly = s->GetPolys() + i;

            if (poly->nverts < 3)
               continue;

            for (int v = 0; v < poly->nverts; v++)
               pts[v] = view->ProjectPoint(s->GetVertexSet()->loc[poly->verts[v]]);

            CRgn rgn;
            rgn.CreatePolygonRgn(pts, poly->nverts, ALTERNATE);
         
            if (rgn.PtInRegion(marks[0])) {
               if (select_mode == SELECT_REMOVE) {
                  selection->RemovePoly(poly);
               }
               else {
                  selection->AddPoly(poly);
               }

               for (int v = 0; v < poly->nverts; v++) {
                  WORD vert = poly->verts[v];
                  if (select_mode == SELECT_REMOVE) {
                     selection->RemoveVert((WORD) s_iter.index(), vert);
                  }
                  else {
                     selection->AddVert((WORD) s_iter.index(), vert);
                  }
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
      ListIter<Surface> s_iter = model->GetSurfaces();
      while (++s_iter) {
         Surface* s = s_iter.value();
         VertexSet* vset = s->GetVertexSet();

         if (s->IsHidden() || s->IsLocked() || s->IsSimplified())
            continue;

         for (WORD i = 0; i < vset->nverts; i++) {
            CPoint p = view->ProjectPoint(s->GetVertexSet()->loc[i]);
            if (rgn.PtInRegion(p)) {
               if (select_mode == SELECT_REMOVE) {
                  selection->RemoveVert((WORD) s_iter.index(), i);
               }
               else {
                  selection->AddVert((WORD) s_iter.index(), i);
               }
            }
         }

         // find all selected polys:
         for (int i = 0; i < s->NumPolys(); i++) {
            Poly* poly = s->GetPolys() + i;

            bool will_select = true;
            for (int v = 0; v < poly->nverts && will_select; v++)
               will_select = will_select && 
                             selection->Contains((WORD) s_iter.index(), poly->verts[v]);

            if (will_select)
               selection->AddPoly(poly);
            else
               selection->RemovePoly(poly);
         }
      }
   }
}

// +----------------------------------------------------------------------+

void
Selector::UseModel(Model* m)
{
   model = m;

   if (selection)
      selection->UseModel(model);
}

// +----------------------------------------------------------------------+

void
Selector::SelectAll(int select_mode)
{
   Clear();

   if (model && select_mode != SELECT_REMOVE) {
      ListIter<Surface> iter = model->GetSurfaces();

      while (++iter) {
         Surface* s = iter.value();

         for (int i = 0; i < s->NumPolys(); i++) {
            selection->GetPolys().append(s->GetPolys() + i);
         }

         for (int i = 0; i < s->NumVerts(); i++) {
            DWORD value = (iter.index() << 16) | i;
			selection->GetVerts().push_back(value);
         }
      }
   }
}

// +----------------------------------------------------------------------+

void
Selector::SelectInverse()
{
   if (model && selection) {
      ListIter<Surface> iter = model->GetSurfaces();

      while (++iter) {
         Surface* s       = iter.value();
         WORD     s_index = iter.index();

         for (int i = 0; i < s->NumPolys(); i++) {
            Poly* p = s->GetPolys() + i;

            if (selection->Contains(p))
               selection->RemovePoly(p);
            else
               selection->AddPoly(p);
         }

         for (int i = 0; i < s->NumVerts(); i++) {
            if (selection->Contains(s_index, i))
               selection->RemoveVert(s_index, i);
            else
               selection->AddVert(s_index, i);
         }
      }
   }
}

// +----------------------------------------------------------------------+

void
Selector::SelectSurface(Surface* s, int select_mode)
{
   if (!s || !model)
      return;

   WORD index = (WORD) model->GetSurfaces().index(s);

   if (select_mode == SELECT_REMOVE) {
      for (int i = 0; i < s->NumPolys(); i++) {
         selection->RemovePoly(s->GetPolys() + i);
      }

      for (int i = 0; i < s->NumVerts(); i++) {
         selection->RemoveVert(index, i);
      }
   }
   else {
      for (WORD i = 0; i < s->NumPolys(); i++) {
         selection->AddPoly(s->GetPolys() + i);
      }

      for (int i = 0; i < s->NumVerts(); i++) {
         selection->AddVert(index, i);
      }
   }
}

void
Selector::SelectVert(Surface* s, int v, int select_mode)
{
   if (!s || !model)
      return;

   WORD index = (WORD) model->GetSurfaces().index(s);

   if (select_mode == SELECT_REMOVE) {
      selection->RemoveVert(index, (WORD) v);
   }
   else {
      selection->AddVert(index, (WORD) v);
   }
}

void
Selector::SelectPoly(Poly* poly, int select_mode)
{
   if (!poly || !model)
      return;

   if (select_mode == SELECT_REMOVE) {
      selection->RemovePoly(poly);
   }
   else {
      selection->AddPoly(poly);
   }
}

// +----------------------------------------------------------------------+

void
Selector::SelectMaterial(Material* m, int select_mode)
{
   if (select_mode == SELECT_REPLACE)
      Clear();

   if (model && select_mode != SELECT_REMOVE) {
      ListIter<Surface> iter = model->GetSurfaces();

      while (++iter) {
         Surface* s       = iter.value();
         WORD     s_index = iter.index();

         for (int i = 0; i < s->NumPolys(); i++) {
            Poly* p = s->GetPolys() + i;

            if (p->material == m) {
               selection->AddPoly(p);

               for (int v = 0; v < p->nverts; v++) {
                  selection->AddVert(s_index, p->verts[v]);
               }
            }
         }
      }
   }
}

// +----------------------------------------------------------------------+

void
Selector::Reselect()
{
   selection->GetPolys().clear();

   if (model) {
      ListIter<Surface> iter = model->GetSurfaces();

      while (++iter) {
         Surface* s       = iter.value();
         WORD     s_index = iter.index();

         // find all selected polys:
         for (int i = 0; i < s->NumPolys(); i++) {
            Poly* poly = s->GetPolys() + i;

            bool will_select = true;
            for (int v = 0; v < poly->nverts && will_select; v++)
               will_select = will_select && 
                             selection->Contains(s_index, poly->verts[v]);

            if (will_select)
               selection->AddPoly(poly);
         }
      }
   }
}
