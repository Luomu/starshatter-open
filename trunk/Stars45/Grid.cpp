/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Grid.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Tactical Grid class
*/

#include "MemDebug.h"
#include "Grid.h"

#include "Game.h"
#include "Video.h"
#include "Window.h"

static const Color DARK_LINE( 8,  8,  8);
static const Color LITE_LINE(16, 16, 16);

// +--------------------------------------------------------------------+

Grid::Grid(int asize, int astep)
   : size(asize), step(astep), drawn(0)
{
   radius = (float) (size * 1.414);
}

Grid::~Grid()
{ }

// +--------------------------------------------------------------------+

int  Grid::CollidesWith(Graphic& o)                { return 0; }

// +--------------------------------------------------------------------+

void Grid::Render(Video* video, DWORD flags)
{
   if (!video || hidden) return;

   int c = 0;
   Color line;
   
   for (int i = 0; i <= size; i += step) {
      Point p1( i, 0, -size); p1 += Location();
      Point p2( i, 0,  size); p2 += Location();
      Point p3(-i, 0, -size); p3 += Location();
      Point p4(-i, 0,  size); p4 += Location();
      
      if (c) line = DARK_LINE;
      else   line = LITE_LINE;

      DrawLine(video, p1,p2,line);
      DrawLine(video, p3,p4,line);

      c++;
      if (c > 3) c = 0;
   }
   
   c = 0;

   for (int i = 0; i <= size; i += step) {
      Point p1(-size, 0,  i); p1 += Location();
      Point p2( size, 0,  i); p2 += Location();
      Point p3(-size, 0, -i); p3 += Location();
      Point p4( size, 0, -i); p4 += Location();
      
      if (c) line = DARK_LINE;
      else   line = LITE_LINE;

      DrawLine(video, p1,p2,line);
      DrawLine(video, p3,p4,line);

      c++;
      if (c > 3) c = 0;
   }
}
   
void Grid::DrawLine(Video* video, Point& p1, Point& p2, Color grid_color)
{
   Vec3 v[2];

   v[0] = p1;
   v[1] = p2;

   video->DrawLines(1, v, grid_color, Video::BLEND_ADDITIVE);
}
