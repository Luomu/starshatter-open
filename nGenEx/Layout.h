/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Layout.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Layout Manager class for ActiveWindow panels
*/

#ifndef Layout_h
#define Layout_h

#include "ActiveWindow.h"
#include "ArrayList.h"

// +--------------------------------------------------------------------+

class Layout
{
public:
   static const char* TYPENAME() { return "Layout"; }

   Layout();
   virtual ~Layout();

   virtual bool   DoLayout(ActiveWindow* panel);

   virtual void   Clear();
   virtual void   AddCol(DWORD min_width,  float col_factor);
   virtual void   AddRow(DWORD min_height, float row_factor);

   virtual void   SetConstraints(const ArrayList& min_x,
                                 const ArrayList& min_y,
                                 const FloatList& weight_x,
                                 const FloatList& weight_y);

   virtual void   SetConstraints(const FloatList& min_x,
                                 const FloatList& min_y,
                                 const FloatList& weight_x,
                                 const FloatList& weight_y);

   virtual void   SetConstraints(int            ncols,
                                 int            nrows,
                                 const int*     min_x,
                                 const int*     min_y,
                                 const float*   weight_x,
                                 const float*   weight_y);


protected:
   virtual void   ScaleWeights();
   virtual void   CalcCells(DWORD w, DWORD h, ArrayList& cell_x, ArrayList& cell_y);

   ArrayList   cols;
   ArrayList   rows;
   FloatList   col_weights;
   FloatList   row_weights;
};

#endif Layout_h

