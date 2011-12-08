/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         VideoDX9VertexBuffer.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Vertex and Index Buffer classes for DirectX 9 
*/

#ifndef VideoDX9VertexBuffer_h
#define VideoDX9VertexBuffer_h

#include "VideoDX9.h"

// +--------------------------------------------------------------------+

class VideoDX9VertexBuffer
{
public:
   VideoDX9VertexBuffer(VideoDX9*   dx9,
                        UINT        num_verts,
                        UINT        vert_size,
                        DWORD       format,
                        DWORD       usage);
   ~VideoDX9VertexBuffer();

   BYTE*    Lock(UINT count);
   void     Unlock();
   bool     Select(int stream=0);

   UINT     GetNumVerts() const;
   UINT     GetVertSize() const;
   UINT     GetNextVert() const;

private:
   VideoDX9*               video;
   IDirect3DVertexBuffer9* vertex_buffer;

   UINT     num_verts;
   UINT     num_locked;
   UINT     vert_size;
   UINT     next_vert;
   bool     is_dynamic;
};

// +--------------------------------------------------------------------+

class VideoDX9IndexBuffer
{
public:
   VideoDX9IndexBuffer(VideoDX9*    dx9,
                        UINT        num_indices,
                        DWORD       usage);
   ~VideoDX9IndexBuffer();

   WORD*    Lock(UINT count);
   void     Unlock();
   bool     Select();

   UINT     GetNumIndices() const;
   UINT     GetNextIndex()  const;

private:
   VideoDX9*               video;
   IDirect3DIndexBuffer9*  index_buffer;

   UINT     num_indices;
   UINT     num_locked;
   UINT     next_index;
   bool     is_dynamic;
};

#endif VideoDX9VertexBuffer_h

