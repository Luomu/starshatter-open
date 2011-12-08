/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Layout.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Layout Resource class implementation
*/

#include "MemDebug.h"
#include "Layout.h"

// +--------------------------------------------------------------------+

Layout::Layout()
{ }

Layout::~Layout()
{ }

// +--------------------------------------------------------------------+

bool
Layout::DoLayout(ActiveWindow* panel)
{
   if (!panel || panel->GetChildren().size() < 1)
      return false;

   if (cols.size() < 1 || rows.size() < 1)
      return false;

   ArrayList   cell_x;
   ArrayList   cell_y;

   ScaleWeights();
   CalcCells(panel->Width(), panel->Height(), cell_x, cell_y);

   ListIter<ActiveWindow> iter = panel->GetChildren();
   while (++iter) {
      ActiveWindow* w = iter.value();
      Rect          c = w->GetCells();
      Rect          r;
      Rect          rp = panel->GetRect();

      if (c.x < 0)                     c.x = 0;
      else if (c.x >= cell_x.size())   c.x = cell_x.size() - 1;
      if (c.y < 0)                     c.y = 0;
      else if (c.y >= cell_y.size())   c.y = cell_y.size() - 1;
      if (c.x+c.w  >= cell_x.size())   c.w = cell_x.size() - c.x - 1;
      if (c.y+c.h  >= cell_y.size())   c.h = cell_y.size() - c.y - 1;

      r.x = cell_x[c.x]     + w->GetCellInsets().left;
      r.y = cell_y[c.y]     + w->GetCellInsets().top;
      r.w = cell_x[c.x+c.w] - w->GetCellInsets().right  - r.x;
      r.h = cell_y[c.y+c.h] - w->GetCellInsets().bottom - r.y;

      r.x += panel->X();
      r.y += panel->Y();

      if (w->GetFixedWidth() && w->GetFixedWidth() < r.w)
         r.w = w->GetFixedWidth();

      if (w->GetFixedHeight() && w->GetFixedHeight() < r.h)
         r.h = w->GetFixedHeight();

      if (w->GetID() == 330 || w->GetID() == 125) {
         int y1 = r.y  + r.h;
         int y2 = rp.y + rp.h;
      }

      if (w->GetHidePartial() && (r.x + r.w > rp.x + rp.w)) {
         w->MoveTo(Rect(0,0,0,0));
      }

      else if (w->GetHidePartial() && (r.y + r.h > rp.y + rp.h)) {
         w->MoveTo(Rect(0,0,0,0));
      }

      else {
         w->MoveTo(r);
      }
   }

   return true;   
}

// +--------------------------------------------------------------------+

void
Layout::ScaleWeights()
{
   int   i;
   float total = 0;

   for (i = 0; i < col_weights.size(); i++)
      total += col_weights[i];

   if (total > 0) {
      for (i = 0; i < col_weights.size(); i++)
         col_weights[i] = col_weights[i] / total;
   }

   total = 0;
   for (i = 0; i < row_weights.size(); i++)
      total += row_weights[i];

   if (total > 0) {
      for (i = 0; i < row_weights.size(); i++)
         row_weights[i] = row_weights[i] / total;
   }
}

// +--------------------------------------------------------------------+

void
Layout::CalcCells(DWORD w, DWORD h, ArrayList& cell_x, ArrayList& cell_y)
{
   DWORD       x     = 0;
   DWORD       y     = 0;
   DWORD       min_x = 0;
   DWORD       min_y = 0;
   DWORD       ext_x = 0;
   DWORD       ext_y = 0;
   int         i;

   for (i = 0; i < cols.size(); i++)
      min_x += cols[i];

   for (i = 0; i < rows.size(); i++)
      min_y += rows[i];

   if (min_x < w)
      ext_x = w - min_x;

   if (min_y < h)
      ext_y = h - min_y;

   cell_x.append(x);
   for (i = 0; i < cols.size(); i++) {
      x += cols[i] + (DWORD) (ext_x * col_weights[i]);
      cell_x.append(x);
   }

   cell_y.append(y);
   for (i = 0; i < rows.size(); i++) {
      y += rows[i] + (DWORD) (ext_y * row_weights[i]);
      cell_y.append(y);
   }
}

// +--------------------------------------------------------------------+

void
Layout::Clear()
{
   cols.clear();
   rows.clear();

   col_weights.clear();
   row_weights.clear();
}

void
Layout::AddCol(DWORD min_width, float col_factor)
{
   cols.append(min_width);
   col_weights.append(col_factor);
}

void
Layout::AddRow(DWORD min_height, float row_factor)
{
   rows.append(min_height);
   row_weights.append(row_factor);
}

void
Layout::SetConstraints(const ArrayList& min_x,
                       const ArrayList& min_y,
                       const FloatList& weight_x,
                       const FloatList& weight_y)
{
   Clear();

   if (min_x.size() == weight_x.size() && 
       min_y.size() == weight_y.size()) {

      cols.append(min_x);
      rows.append(min_y);

      col_weights.append(weight_x);
      row_weights.append(weight_y);
   }
}

void
Layout::SetConstraints(const FloatList& min_x,
                       const FloatList& min_y,
                       const FloatList& weight_x,
                       const FloatList& weight_y)
{
   Clear();

   if (min_x.size() == weight_x.size() && 
       min_y.size() == weight_y.size()) {

      for (int i = 0; i < min_x.size(); i++)
         cols.append((DWORD) min_x[i]);

      for (int i = 0; i < min_y.size(); i++)
         rows.append((DWORD) min_y[i]);

      col_weights.append(weight_x);
      row_weights.append(weight_y);
   }
}

void
Layout::SetConstraints(int            ncols,
                       int            nrows,
                       const int*     min_x,
                       const int*     min_y,
                       const float*   weight_x,
                       const float*   weight_y)
{
   Clear();

   if (nrows > 0 && ncols > 0) {
      int i = 0;

      for (i = 0; i < ncols; i++) {
         cols.append(min_x[i]);
         col_weights.append(weight_x[i]);
      }

      for (i = 0; i < nrows; i++) {
         rows.append(min_y[i]);
         row_weights.append(weight_y[i]);
      }
   }
}
