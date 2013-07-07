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

