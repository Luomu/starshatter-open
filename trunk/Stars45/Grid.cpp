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
