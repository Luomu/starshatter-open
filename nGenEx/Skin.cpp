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
    FILE:         Skin.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Classes for rendering solid meshes of polygons
*/

#include "MemDebug.h"
#include "Skin.h"
#include "Solid.h"

void  Print(const char* fmt, ...);

// +--------------------------------------------------------------------+

Skin::Skin(const char* n)
{
    if (n && *n) {
        strncpy_s(name, n, NAMELEN);
        name[NAMELEN-1] = 0;
    }

    else {
        ZeroMemory(name, NAMELEN);
    }

    ZeroMemory(path, 256);
}

// +--------------------------------------------------------------------+

Skin::~Skin()
{
    cells.destroy();
}

// +--------------------------------------------------------------------+

void
Skin::SetName(const char* n)
{
    if (n && *n) {
        strncpy_s(name, n, NAMELEN);
        name[NAMELEN-1] = 0;
    }
}

void
Skin::SetPath(const char* n)
{
    if (n && *n) {
        strncpy_s(path, n, 256);
        path[255] = 0;
    }

    else {
        ZeroMemory(path, 256);
    }
}

// +--------------------------------------------------------------------+

void
Skin::AddMaterial(const Material* mtl)
{
    if (!mtl) return;

    bool found = false;

    ListIter<SkinCell> iter = cells;
    while (++iter && !found) {
        SkinCell* s = iter.value();

        if (s->skin && !strcmp(s->skin->name, mtl->name)) {
            s->skin = mtl;
            found   = true;
        }
    }

    if (!found) {
        SkinCell* s = new(__FILE__,__LINE__) SkinCell(mtl);
        cells.append(s);
    }
}

// +--------------------------------------------------------------------+

void
Skin::ApplyTo(Model* model) const
{
    if (model) {
        for (int i = 0; i < cells.size(); i++) {
            SkinCell* s = cells[i];

            if (s->skin) {
                s->orig = model->ReplaceMaterial(s->skin);
            }
        }
    }
}

void
Skin::Restore(Model* model) const
{
    if (model) {
        for (int i = 0; i < cells.size(); i++) {
            SkinCell* s = cells[i];

            if (s->orig) {
                model->ReplaceMaterial(s->orig);
                s->orig = 0;
            }
        }
    }
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

SkinCell::SkinCell(const Material* mtl)
    : skin(mtl), orig(0)
{
}

SkinCell::~SkinCell()
{
    delete skin;
}

// +--------------------------------------------------------------------+

int
SkinCell::operator == (const SkinCell& other) const
{
    if (skin == other.skin)
    return true;

    if (skin && other.skin)
    return !strcmp(skin->name, other.skin->name);

    return false;
}

// +--------------------------------------------------------------------+

const char*
SkinCell::Name() const
{
    if (skin)
    return skin->name;

    return "Invalid Skin Cell";
}

// +--------------------------------------------------------------------+

void
SkinCell::SetSkin(const Material* mtl)
{
    skin = mtl;
}

void
SkinCell::SetOrig(const Material* mtl)
{
    orig = mtl;
}
