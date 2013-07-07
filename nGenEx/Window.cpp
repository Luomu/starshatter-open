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

    SUBSYSTEM:    nGenEx.lib
    FILE:         Window.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Window class
*/

#include "MemDebug.h"
#include "Window.h"
#include "Bitmap.h"
#include "Color.h"
#include "Fix.h"
#include "Font.h"
#include "Polygon.h"
#include "Screen.h"
#include "Video.h"
#include "View.h"

static VertexSet vset4(4);

// +--------------------------------------------------------------------+

Window::Window(Screen* s, int ax, int ay, int aw, int ah)
    : screen(s), rect(ax, ay, aw, ah), shown(true), font(0)
{ }

// +--------------------------------------------------------------------+

Window::~Window()
{
    view_list.destroy();
}

// +--------------------------------------------------------------------+

bool
Window::AddView(View* v)
{
    if (!v) return false;

    if (!view_list.contains(v))
    view_list.append(v);

    return true;
}

bool
Window::DelView(View* v)
{
    if (!v) return false;

    return view_list.remove(v) == v;
}

void
Window::MoveTo(const Rect& r)
{
    if (rect.x == r.x &&
            rect.y == r.y &&
            rect.w == r.w &&
            rect.h == r.h)
    return;

    rect = r;

    ListIter<View> v = view_list;
    while (++v)
    v->OnWindowMove();
}

// +--------------------------------------------------------------------+

void
Window::Paint()
{
    ListIter<View> v = view_list;
    while (++v)
    v->Refresh();
}

// +--------------------------------------------------------------------+

static inline void swap(int& a, int& b) { int tmp=a; a=b; b=tmp; }
static inline void sort(int& a, int& b) { if (a>b) swap(a,b);    }
static inline void swap(double& a, double& b) { double tmp=a; a=b; b=tmp; }
static inline void sort(double& a, double& b) { if (a>b) swap(a,b);    }

Rect
Window::ClipRect(const Rect& r)
{
    Rect clip_rect = r;

    clip_rect.x += rect.x;
    clip_rect.y += rect.y;

    if (clip_rect.x < rect.x) {
        clip_rect.w -= rect.x - clip_rect.x;
        clip_rect.x = rect.x;
    }

    if (clip_rect.y < rect.y) {
        clip_rect.h -= rect.y - clip_rect.y;
        clip_rect.y = rect.y;
    }

    if (clip_rect.x + clip_rect.w > rect.x + rect.w)
    clip_rect.w = rect.x + rect.w - clip_rect.x;

    if (clip_rect.y + clip_rect.h > rect.y + rect.h)
    clip_rect.h = rect.y + rect.h - clip_rect.y;

    return clip_rect;
}

// +--------------------------------------------------------------------+

bool
Window::ClipLine(int& x1, int& y1, int& x2, int& y2)
{
    // vertical lines:
    if (x1==x2) {
        clip_vertical:
        sort(y1,y2);
        if (x1 < 0 || x1 >= rect.w) return false;
        if (y1 < 0) y1 = 0;
        if (y2 >= rect.h) y2 = rect.h;
        return true;
    }

    // horizontal lines:
    if (y1==y2) {
        clip_horizontal:
        sort(x1,x2);
        if (y1 < 0 || y1 >= rect.h) return false;
        if (x1 < 0) x1 = 0;
        if (x2 > rect.w) x2 = rect.w;
        return true;
    }

    // general lines:

    // sort left to right:
    if (x1 > x2) {
        swap(x1,x2);
        swap(y1,y2);
    }

    double m = (double)(y2-y1) / (double)(x2-x1);
    double b = (double) y1 - (m * x1);

    // clip:
    if (x1 < 0)        { x1 = 0; y1 = (int) b; }
    if (x1 >= rect.w)  return false;
    if (x2 < 0)        return false;
    if (x2 > rect.w-1) { x2 = rect.w-1; y2 = (int) (m * x2 + b); }

    if (y1 < 0       && y2 < 0)       return false;
    if (y1 >= rect.h && y2 >= rect.h) return false;

    if (y1 < 0)       { y1 = 0; x1 = (int) (-b/m); }
    if (y1 >= rect.h) { y1 = rect.h-1; x1 = (int) ((y1-b)/m); }
    if (y2 < 0)       { y2 = 0; x2 = (int) (-b/m); }
    if (y2 >= rect.h) { y2 = rect.h-1; x2 = (int) ((y2-b)/m); }

    if (x1 == x2)
    goto clip_vertical;

    if (y1 == y2)
    goto clip_horizontal;   

    return true;
}

// +--------------------------------------------------------------------+

bool
Window::ClipLine(double& x1, double& y1, double& x2, double& y2)
{
    // vertical lines:
    if (x1==x2) {
        clip_vertical:
        sort(y1,y2);
        if (x1 < 0 || x1 >= rect.w) return false;
        if (y1 < 0) y1 = 0;
        if (y2 >= rect.h) y2 = rect.h;
        return true;
    }

    // horizontal lines:
    if (y1==y2) {
        clip_horizontal:
        sort(x1,x2);
        if (y1 < 0 || y1 >= rect.h) return false;
        if (x1 < 0) x1 = 0;
        if (x2 > rect.w) x2 = rect.w;
        return true;
    }

    // general lines:

    // sort left to right:
    if (x1 > x2) {
        swap(x1,x2);
        swap(y1,y2);
    }

    double m = (double)(y2-y1) / (double)(x2-x1);
    double b = (double) y1 - (m * x1);

    // clip:
    if (x1 < 0)        { x1 = 0; y1 = b; }
    if (x1 >= rect.w)  return false;
    if (x2 < 0)        return false;
    if (x2 > rect.w-1) { x2 = rect.w-1; y2 = (m * x2 + b); }

    if (y1 < 0       && y2 < 0)       return false;
    if (y1 >= rect.h && y2 >= rect.h) return false;

    if (y1 < 0)       { y1 = 0; x1 = (-b/m); }
    if (y1 >= rect.h) { y1 = rect.h-1; x1 = ((y1-b)/m); }
    if (y2 < 0)       { y2 = 0; x2 = (-b/m); }
    if (y2 >= rect.h) { y2 = rect.h-1; x2 = ((y2-b)/m); }

    if (x1 == x2)
    goto clip_vertical;

    if (y1 == y2)
    goto clip_horizontal;   

    return true;
}

// +--------------------------------------------------------------------+

void
Window::DrawLine(int x1, int y1, int x2, int y2, Color color, int blend)
{
    if (!screen || !screen->GetVideo()) return;

    if (ClipLine(x1,y1,x2,y2)) {
        float points[4];

        points[0] = (float) (rect.x + x1);
        points[1] = (float) (rect.y + y1);
        points[2] = (float) (rect.x + x2);
        points[3] = (float) (rect.y + y2);

        Video* video = screen->GetVideo();
        video->DrawScreenLines(1, points, color, blend);
    }
}

// +--------------------------------------------------------------------+

void
Window::DrawRect(int x1, int y1, int x2, int y2, Color color, int blend)
{
    if (!screen || !screen->GetVideo()) return;

    sort(x1,x2);
    sort(y1,y2);

    if (x1 > rect.w || x2 < 0 || y1 > rect.h || y2 < 0)
    return;

    float points[16];

    points[ 0] = (float) (rect.x + x1);
    points[ 1] = (float) (rect.y + y1);
    points[ 2] = (float) (rect.x + x2);
    points[ 3] = (float) (rect.y + y1);

    points[ 4] = (float) (rect.x + x2);
    points[ 5] = (float) (rect.y + y1);
    points[ 6] = (float) (rect.x + x2);
    points[ 7] = (float) (rect.y + y2);

    points[ 8] = (float) (rect.x + x2);
    points[ 9] = (float) (rect.y + y2);
    points[10] = (float) (rect.x + x1);
    points[11] = (float) (rect.y + y2);

    points[12] = (float) (rect.x + x1);
    points[13] = (float) (rect.y + y2);
    points[14] = (float) (rect.x + x1);
    points[15] = (float) (rect.y + y1);

    Video* video = screen->GetVideo();
    video->DrawScreenLines(4, points, color, blend);
}

void
Window::DrawRect(const Rect& r, Color color, int blend)
{
    DrawRect(r.x, r.y, r.x+r.w, r.y+r.h, color, blend);
}

// +--------------------------------------------------------------------+

void
Window::FillRect(int x1, int y1, int x2, int y2, Color color, int blend)
{
    if (!screen || !screen->GetVideo()) return;

    sort(x1,x2);
    sort(y1,y2);

    if (x1 > rect.w || x2 < 0 || y1 > rect.h || y2 < 0)
    return;

    vset4.space = VertexSet::SCREEN_SPACE;
    for (int i = 0; i < 4; i++) {
        vset4.diffuse[i]   = color.Value();
    }

    vset4.s_loc[0].x = (float) (rect.x + x1) - 0.5f;
    vset4.s_loc[0].y = (float) (rect.y + y1) - 0.5f;
    vset4.s_loc[0].z = 0.0f;
    vset4.rw[0]      = 1.0f;
    vset4.tu[0]      = 0.0f;
    vset4.tv[0]      = 0.0f;

    vset4.s_loc[1].x = (float) (rect.x + x2) - 0.5f;
    vset4.s_loc[1].y = (float) (rect.y + y1) - 0.5f;
    vset4.s_loc[1].z = 0.0f;
    vset4.rw[1]      = 1.0f;
    vset4.tu[1]      = 1.0f;
    vset4.tv[1]      = 0.0f;

    vset4.s_loc[2].x = (float) (rect.x + x2) - 0.5f;
    vset4.s_loc[2].y = (float) (rect.y + y2) - 0.5f;
    vset4.s_loc[2].z = 0.0f;
    vset4.rw[2]      = 1.0f;
    vset4.tu[2]      = 1.0f;
    vset4.tv[2]      = 1.0f;

    vset4.s_loc[3].x = (float) (rect.x + x1) - 0.5f;
    vset4.s_loc[3].y = (float) (rect.y + y2) - 0.5f;
    vset4.s_loc[3].z = 0.0f;
    vset4.rw[3]      = 1.0f;
    vset4.tu[3]      = 0.0f;
    vset4.tv[3]      = 1.0f;

    Poly poly(0);
    poly.nverts = 4;
    poly.vertex_set = &vset4;
    poly.verts[0]   = 0;
    poly.verts[1]   = 1;
    poly.verts[2]   = 2;
    poly.verts[3]   = 3;

    Video* video = screen->GetVideo();
    video->DrawScreenPolys(1, &poly, blend);
}

void
Window::FillRect(const Rect& r, Color color, int blend)
{
    FillRect(r.x, r.y, r.x+r.w, r.y+r.h, color, blend);
}

// +--------------------------------------------------------------------+

void
Window::DrawLines(int nPts, POINT* pts, Color color, int blend)
{
    if (nPts < 2 || nPts > 16)
    return;

    if (!screen || !screen->GetVideo())
    return;

    float f[64];
    int   n = 0;

    for (int i = 0; i < nPts-1; i++) {
        f[n++] = (float) rect.x + pts[i].x;
        f[n++] = (float) rect.y + pts[i].y;
        f[n++] = (float) rect.x + pts[i+1].x;
        f[n++] = (float) rect.y + pts[i+1].y;
    }

    Video* video = screen->GetVideo();
    video->DrawScreenLines(nPts-1, f, color, blend);
}

void
Window::DrawPoly(int nPts, POINT* pts, Color color, int blend)
{
    if (nPts < 3 || nPts > 8)
    return;

    if (!screen || !screen->GetVideo())
    return;

    float f[32];
    int   n = 0;

    for (int i = 0; i < nPts-1; i++) {
        f[n++] = (float) rect.x + pts[i].x;
        f[n++] = (float) rect.y + pts[i].y;
        f[n++] = (float) rect.x + pts[i+1].x;
        f[n++] = (float) rect.y + pts[i+1].y;
    }

    f[n++] = (float) rect.x + pts[nPts-1].x;
    f[n++] = (float) rect.y + pts[nPts-1].y;
    f[n++] = (float) rect.x + pts[0].x;
    f[n++] = (float) rect.y + pts[0].y;

    Video* video = screen->GetVideo();
    video->DrawScreenLines(nPts, f, color, blend);
}

void
Window::FillPoly(int nPts, POINT* pts, Color color, int blend)
{
    if (nPts < 3 || nPts > 4)
    return;

    if (!screen || !screen->GetVideo())
    return;

    vset4.space = VertexSet::SCREEN_SPACE;
    for (int i = 0; i < nPts; i++) {
        vset4.diffuse[i]  = color.Value();
        vset4.s_loc[i].x  = (float) (rect.x + pts[i].x) - 0.5f;
        vset4.s_loc[i].y  = (float) (rect.y + pts[i].y) - 0.5f;
        vset4.s_loc[i].z  = 0.0f;
        vset4.rw[i]       = 1.0f;
        vset4.tu[i]       = 0.0f;
        vset4.tv[i]       = 0.0f;
    }

    Poly poly(0);
    poly.nverts = nPts;
    poly.vertex_set = &vset4;
    poly.verts[0]   = 0;
    poly.verts[1]   = 1;
    poly.verts[2]   = 2;
    poly.verts[3]   = 3;

    Video* video = screen->GetVideo();
    video->DrawScreenPolys(1, &poly, blend);
}

// +--------------------------------------------------------------------+

void
Window::DrawBitmap(int x1, int y1, int x2, int y2, Bitmap* img, int blend)
{
    Rect clip_rect;
    clip_rect.w = rect.w;
    clip_rect.h = rect.h;

    ClipBitmap(x1,y1,x2,y2,img,Color::White,blend,clip_rect);
}

void
Window::FadeBitmap(int x1, int y1, int x2, int y2, Bitmap* img, Color c, int blend)
{
    Rect clip_rect;
    clip_rect.w = rect.w;
    clip_rect.h = rect.h;

    ClipBitmap(x1,y1,x2,y2,img,c,blend,clip_rect);
}

void
Window::ClipBitmap(int x1, int y1, int x2, int y2, Bitmap* img, Color c, int blend, const Rect& clip_rect)
{
    if (!screen || !screen->GetVideo() || !img) return;

    Rect clip = clip_rect;

    // clip the clip rect to the window rect:
    if (clip.x < 0) {
        clip.w -= clip.x;
        clip.x = 0;
    }

    if (clip.x + clip.w > rect.w) {
        clip.w -= (clip.x + clip.w - rect.w);
    }

    if (clip.y < 0) {
        clip.h -= clip.y;
        clip.y = 0;
    }

    if (clip.y + clip.h > rect.h) {
        clip.h -= (clip.y + clip.h - rect.h);
    }

    // now clip the bitmap to the validated clip rect:
    sort(x1,x2);
    sort(y1,y2);

    if (x1 > clip.x + clip.w || x2 < clip.x || y1 > clip.y + clip.h || y2 < clip.y)
    return;

    vset4.space = VertexSet::SCREEN_SPACE;
    for (int i = 0; i < 4; i++) {
        vset4.diffuse[i] = c.Value();
    }

    float u1 = 0.0f;
    float u2 = 1.0f;
    float v1 = 0.0f;
    float v2 = 1.0f;
    float iw = (float) (x2-x1);
    float ih = (float) (y2-y1);
    int   x3 = clip.x + clip.w;
    int   y3 = clip.y + clip.h;

    if (x1 < clip.x) {
        u1 = (clip.x - x1) / iw;
        x1 = clip.x;
    }

    if (x2 > x3) {
        u2 = 1.0f - (x2 - x3) / iw;
        x2 = x3;
    }

    if (y1 < clip.y) {
        v1 = (clip.y - y1) / ih;
        y1 = clip.y;
    }

    if (y2 > y3) {
        v2 = 1.0f - (y2 - y3) / ih;
        y2 = y3;
    }

    vset4.s_loc[0].x = (float) (rect.x + x1) - 0.5f;
    vset4.s_loc[0].y = (float) (rect.y + y1) - 0.5f;
    vset4.s_loc[0].z = 0.0f;
    vset4.rw[0]      = 1.0f;
    vset4.tu[0]      = u1;
    vset4.tv[0]      = v1;

    vset4.s_loc[1].x = (float) (rect.x + x2) - 0.5f;
    vset4.s_loc[1].y = (float) (rect.y + y1) - 0.5f;
    vset4.s_loc[1].z = 0.0f;
    vset4.rw[1]      = 1.0f;
    vset4.tu[1]      = u2;
    vset4.tv[1]      = v1;

    vset4.s_loc[2].x = (float) (rect.x + x2) - 0.5f;
    vset4.s_loc[2].y = (float) (rect.y + y2) - 0.5f;
    vset4.s_loc[2].z = 0.0f;
    vset4.rw[2]      = 1.0f;
    vset4.tu[2]      = u2;
    vset4.tv[2]      = v2;

    vset4.s_loc[3].x = (float) (rect.x + x1) - 0.5f;
    vset4.s_loc[3].y = (float) (rect.y + y2) - 0.5f;
    vset4.s_loc[3].z = 0.0f;
    vset4.rw[3]      = 1.0f;
    vset4.tu[3]      = u1;
    vset4.tv[3]      = v2;

    Material mtl;
    mtl.tex_diffuse = img;

    Poly poly(0);
    poly.nverts = 4;
    poly.vertex_set = &vset4;
    poly.material   = &mtl;
    poly.verts[0]   = 0;
    poly.verts[1]   = 1;
    poly.verts[2]   = 2;
    poly.verts[3]   = 3;

    Video* video = screen->GetVideo();

    video->SetRenderState(Video::TEXTURE_WRAP, 0);
    video->DrawScreenPolys(1, &poly, blend);
    video->SetRenderState(Video::TEXTURE_WRAP, 1);
}

// +--------------------------------------------------------------------+

void
Window::TileBitmap(int x1, int y1, int x2, int y2, Bitmap* img, int blend)
{
    if (!screen || !screen->GetVideo())             return;
    if (!img    || !img->Width() || !img->Height()) return;

    vset4.space = VertexSet::SCREEN_SPACE;
    for (int i = 0; i < 4; i++) {
        vset4.diffuse[i]   = Color::White.Value();
    }

    float xscale = (float) rect.w / (float) img->Width();
    float yscale = (float) rect.h / (float) img->Height();

    vset4.s_loc[0].x = (float) (rect.x + x1) - 0.5f;
    vset4.s_loc[0].y = (float) (rect.y + y1) - 0.5f;
    vset4.s_loc[0].z = 0.0f;
    vset4.rw[0]      = 1.0f;
    vset4.tu[0]      = 0.0f;
    vset4.tv[0]      = 0.0f;

    vset4.s_loc[1].x = (float) (rect.x + x2) - 0.5f;
    vset4.s_loc[1].y = (float) (rect.y + y1) - 0.5f;
    vset4.s_loc[1].z = 0.0f;
    vset4.rw[1]      = 1.0f;
    vset4.tu[1]      = xscale;
    vset4.tv[1]      = 0.0f;

    vset4.s_loc[2].x = (float) (rect.x + x2) - 0.5f;
    vset4.s_loc[2].y = (float) (rect.y + y2) - 0.5f;
    vset4.s_loc[2].z = 0.0f;
    vset4.rw[2]      = 1.0f;
    vset4.tu[2]      = xscale;
    vset4.tv[2]      = yscale;

    vset4.s_loc[3].x = (float) (rect.x + x1) - 0.5f;
    vset4.s_loc[3].y = (float) (rect.y + y2) - 0.5f;
    vset4.s_loc[3].z = 0.0f;
    vset4.rw[3]      = 1.0f;
    vset4.tu[3]      = 0.0f;
    vset4.tv[3]      = yscale;

    Material mtl;
    mtl.tex_diffuse = img;

    Poly poly(0);
    poly.nverts = 4;
    poly.vertex_set = &vset4;
    poly.material   = &mtl;
    poly.verts[0]   = 0;
    poly.verts[1]   = 1;
    poly.verts[2]   = 2;
    poly.verts[3]   = 3;

    Video* video = screen->GetVideo();
    video->DrawScreenPolys(1, &poly, blend);
}

// +--------------------------------------------------------------------+

static float ellipse_pts[256];

void
Window::DrawEllipse(int x1, int y1, int x2, int y2, Color color, int blend)
{
    Video* video = screen->GetVideo();

    if (!video)
    return;

    sort(x1,x2);
    sort(y1,y2);

    if (x1 > rect.w || x2 < 0 || y1 > rect.h || y2 < 0)
    return;

    double w2 = (x2-x1)/2.0;
    double h2 = (y2-y1)/2.0;
    double cx = rect.x + x1 + w2;
    double cy = rect.y + y1 + h2;
    double r  = w2;
    int    ns = 4;
    int    np = 0;

    if (h2 > r)
    r = h2;

    if (r > 2*ns)
    ns = (int) (r/2);

    if (ns > 64)
    ns = 64;

    double theta = 0;
    double dt    = (PI/2) / ns;

    // quadrant 1 (lower right):
    if (cx < (rect.x+rect.w) && cy < (rect.y + rect.h)) {
        theta = 0;
        np    = 0;

        for (int i = 0; i < ns; i++) {
            double ex1 = x1 + w2 + cos(theta) * w2;
            double ey1 = y1 + h2 + sin(theta) * h2;

            theta += dt;

            double ex2 = x1 + w2 + cos(theta) * w2;
            double ey2 = y1 + h2 + sin(theta) * h2;

            if (ClipLine(ex1, ey1, ex2, ey2)) {
                ellipse_pts[np++] = (float) (rect.x + ex1);
                ellipse_pts[np++] = (float) (rect.y + ey1);
                ellipse_pts[np++] = (float) (rect.x + ex2);
                ellipse_pts[np++] = (float) (rect.y + ey2);
            }
        }

        video->DrawScreenLines(np/4, ellipse_pts, color, blend);
    }

    // quadrant 2 (lower left):
    if (cx > rect.x && cy < (rect.y + rect.h)) {
        theta = 90*DEGREES;
        np    = 0;

        for (int i = 0; i < ns; i++) {
            double ex1 = x1 + w2 + cos(theta) * w2;
            double ey1 = y1 + h2 + sin(theta) * h2;

            theta += dt;

            double ex2 = x1 + w2 + cos(theta) * w2;
            double ey2 = y1 + h2 + sin(theta) * h2;

            if (ClipLine(ex1, ey1, ex2, ey2)) {
                ellipse_pts[np++] = (float) (rect.x + ex1);
                ellipse_pts[np++] = (float) (rect.y + ey1);
                ellipse_pts[np++] = (float) (rect.x + ex2);
                ellipse_pts[np++] = (float) (rect.y + ey2);
            }
        }

        video->DrawScreenLines(np/4, ellipse_pts, color, blend);
    }

    // quadrant 3 (upper left):
    if (cx > rect.x && cy > rect.y) {
        theta = 180*DEGREES;
        np    = 0;

        for (int i = 0; i < ns; i++) {
            double ex1 = x1 + w2 + cos(theta) * w2;
            double ey1 = y1 + h2 + sin(theta) * h2;

            theta += dt;

            double ex2 = x1 + w2 + cos(theta) * w2;
            double ey2 = y1 + h2 + sin(theta) * h2;

            if (ClipLine(ex1, ey1, ex2, ey2)) {
                ellipse_pts[np++] = (float) (rect.x + ex1);
                ellipse_pts[np++] = (float) (rect.y + ey1);
                ellipse_pts[np++] = (float) (rect.x + ex2);
                ellipse_pts[np++] = (float) (rect.y + ey2);
            }
        }

        video->DrawScreenLines(np/4, ellipse_pts, color, blend);
    }

    // quadrant 4 (upper right):
    if (cx < (rect.x+rect.w) && cy > rect.y) {
        theta = 270*DEGREES;
        np    = 0;

        for (int i = 0; i < ns; i++) {
            double ex1 = x1 + w2 + cos(theta) * w2;
            double ey1 = y1 + h2 + sin(theta) * h2;

            theta += dt;

            double ex2 = x1 + w2 + cos(theta) * w2;
            double ey2 = y1 + h2 + sin(theta) * h2;

            if (ClipLine(ex1, ey1, ex2, ey2)) {
                ellipse_pts[np++] = (float) (rect.x + ex1);
                ellipse_pts[np++] = (float) (rect.y + ey1);
                ellipse_pts[np++] = (float) (rect.x + ex2);
                ellipse_pts[np++] = (float) (rect.y + ey2);
            }
        }

        video->DrawScreenLines(np/4, ellipse_pts, color, blend);
    }

}

void
Window::FillEllipse(int x1, int y1, int x2, int y2, Color color, int blend)
{
    Video* video = screen->GetVideo();

    if (!video)
    return;

    sort(x1,x2);
    sort(y1,y2);

    if (x1 > rect.w || x2 < 0 || y1 > rect.h || y2 < 0)
    return;

    double w2 = (x2-x1)/2.0;
    double h2 = (y2-y1)/2.0;
    double cx = x1 + w2;
    double cy = y1 + h2;
    double r  = w2;
    int    ns = 4;
    int    np = 0;

    if (h2 > r)
    r = h2;

    if (r > 2*ns)
    ns = (int) (r/2);

    if (ns > 64)
    ns = 64;

    double theta = -PI / 2;
    double dt    =  PI / ns;

    for (int i = 0; i < ns; i++) {
        double ex1 = cos(theta) * w2;
        double ey1 = sin(theta) * h2;

        theta += dt;

        double ex2 = cos(theta) * w2;
        double ey2 = sin(theta) * h2;

        POINT pts[4];

        pts[0].x = (int) (cx - ex1);
        pts[0].y = (int) (cy + ey1);

        pts[1].x = (int) (cx + ex1);
        pts[1].y = (int) (cy + ey1);

        pts[2].x = (int) (cx + ex2);
        pts[2].y = (int) (cy + ey2);

        pts[3].x = (int) (cx - ex2);
        pts[3].y = (int) (cy + ey2);

        if (pts[0].x > rect.w && pts[3].x > rect.w)
        continue;

        if (pts[1].x < 0 && pts[2].x < 0)
        continue;

        if (pts[0].y > rect.h)
        return;
        
        if (pts[2].y < 0)
        continue;

        if (pts[0].x < 0)       pts[0].x = 0;
        if (pts[3].x < 0)       pts[3].x = 0;
        if (pts[1].x > rect.w)  pts[1].x = rect.w;
        if (pts[2].x > rect.w)  pts[2].x = rect.w;

        if (pts[0].y < 0)       pts[0].y = 0;
        if (pts[1].y < 0)       pts[1].y = 0;
        if (pts[2].y > rect.h)  pts[2].y = rect.h;
        if (pts[3].y > rect.h)  pts[3].y = rect.h;

        FillPoly(4, pts, color, blend);
    }
}

// +--------------------------------------------------------------------+

void
Window::Print(int x1, int y1, const char* fmt, ...)
{
    if (!font || x1<0 || y1<0 || x1>=rect.w || y1>=rect.h || !fmt)
    return;

    x1 += rect.x;
    y1 += rect.y;

    char msgbuf[512];
    vsprintf_s(msgbuf, fmt, (char *)(&fmt+1));
    font->DrawString(msgbuf, strlen(msgbuf), x1, y1, rect);
}

void
Window::DrawText(const char* txt, int count, Rect& txt_rect, DWORD flags)
{
    if (!font)
    return;

    if (txt && !count)
    count = strlen(txt);

    // clip the rect:
    Rect clip_rect = txt_rect;

    if (clip_rect.x < 0) {
        int dx = -clip_rect.x;
        clip_rect.x += dx;
        clip_rect.w -= dx;
    }

    if (clip_rect.y < 0) {
        int dy = -clip_rect.y;
        clip_rect.y += dy;
        clip_rect.h -= dy;
    }

    if (clip_rect.w < 1 || clip_rect.h < 1)
    return;

    if (clip_rect.x + clip_rect.w > rect.w)
    clip_rect.w = rect.w - clip_rect.x;

    if (clip_rect.y + clip_rect.h > rect.h)
    clip_rect.h = rect.h - clip_rect.y;

    clip_rect.x += rect.x;
    clip_rect.y += rect.y;

    if (font && txt && count) {
        font->DrawText(txt, count, clip_rect, flags);
        font->SetAlpha(1);
    }

    // if calc only, update the rectangle:
    if (flags & DT_CALCRECT) {
        txt_rect.h = clip_rect.h;
        txt_rect.w = clip_rect.w;
    }
}

