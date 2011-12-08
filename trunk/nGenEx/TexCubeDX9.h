/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2006. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         TexCubeDX9.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Direct 3D Texture Cube for Env Mapping
*/

#ifndef TexCubeDX9_h
#define TexCubeDX9_h

#include "Bitmap.h"

// +--------------------------------------------------------------------+

class  Video;
class  VideoDX9;
class  Bitmap;
struct VD3D_texture_format;

// +--------------------------------------------------------------------+

class TexCubeDX9
{
public:
   TexCubeDX9(VideoDX9* video);
   virtual ~TexCubeDX9();

   IDirect3DCubeTexture9*  GetTexture();
   bool                    LoadTexture(Bitmap* bmp, int face);

private:
   VideoDX9*               video;
   IDirect3D9*             d3d;
   IDirect3DDevice9*       d3ddevice;

   IDirect3DCubeTexture9*  texture;
   Bitmap*                 faces[6];
   DWORD                   last_modified[6];
};

#endif // TexCubeDX9_h

