/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         VideoDX9VertexBuffer.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Direct3D Video class for DirectX 9
*/

#include "MemDebug.h"
#include "VideoDX9VertexBuffer.h"
#include "Color.h"

// +--------------------------------------------------------------------+

void                 VideoDX9Error(const char* msg, HRESULT dderr);
extern int           VD3D_describe_things;

#ifndef RELEASE
#define RELEASE(x) if (x) { x->Release(); x=NULL; }
#endif

// +--------------------------------------------------------------------+

VideoDX9VertexBuffer::VideoDX9VertexBuffer(VideoDX9*   dx9,
                                           UINT        nverts,
                                           UINT        vsize,
                                           DWORD       format,
                                           DWORD       usage)
 : video(dx9), vertex_buffer(0),
   num_verts(nverts), num_locked(0), vert_size(vsize), next_vert(0),
   is_dynamic(false)
{
   UINT len = num_verts * vert_size;

   if (video && len) {
      is_dynamic   = (usage & D3DUSAGE_DYNAMIC) ? true : false;
      D3DPOOL pool = is_dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;

      HRESULT hr = video->D3DDevice()->CreateVertexBuffer(len, 
                                                          usage,
                                                          format,
                                                          pool,
                                                          &vertex_buffer,
                                                          0);

      if (FAILED(hr)) {
         static int report = 10;
         if (report) {
            VideoDX9Error("Could not create vertex buffer.", hr);
            report--;
         }

         num_verts = 0;
         vert_size = 0;
         next_vert = 0;
      }
   }
}

VideoDX9VertexBuffer::~VideoDX9VertexBuffer()
{
   RELEASE(vertex_buffer);
}

// +--------------------------------------------------------------------+

BYTE*
VideoDX9VertexBuffer::Lock(UINT count)
{
   if (vertex_buffer && count <= num_verts) {
      DWORD flags = 0;

      if (count == 0)
         count = num_verts;
      
      if (is_dynamic) {
         flags = D3DLOCK_NOOVERWRITE;

         if (next_vert + count > num_verts) {
            next_vert = 0;
            flags = D3DLOCK_DISCARD;
         }
      }

      void*    result = 0;
      HRESULT  hr     = 0;

      hr = vertex_buffer->Lock(next_vert * vert_size,
                               count     * vert_size,
                               &result,
                               flags);

      if (SUCCEEDED(hr)) {
         num_locked = count;
         return (BYTE*) result;
      }
   }

   return 0;
}

void
VideoDX9VertexBuffer::Unlock()
{
   if (vertex_buffer && num_locked > 0) {
      vertex_buffer->Unlock();

      next_vert  += num_locked;
      num_locked = 0;
   }
}

bool
VideoDX9VertexBuffer::Select(int stream)
{
   if (video && vertex_buffer) {
      HRESULT hr = E_FAIL;

      if (num_locked > 0)
         Unlock();

      hr = video->D3DDevice()->SetStreamSource(stream,
                                               vertex_buffer,
                                               0,
                                               vert_size);

      if (SUCCEEDED(hr))
         return true;
   }

   return false;
}

// +--------------------------------------------------------------------+

UINT
VideoDX9VertexBuffer::GetNumVerts() const
{
   return num_verts;
}

UINT
VideoDX9VertexBuffer::GetVertSize() const
{
   return vert_size;
}

UINT
VideoDX9VertexBuffer::GetNextVert() const
{
   return next_vert;
}


// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+


VideoDX9IndexBuffer::VideoDX9IndexBuffer(VideoDX9*   dx9,
                                         UINT        nind,
                                         DWORD       usage)
 : video(dx9), index_buffer(0), 
   num_indices(nind), num_locked(0), next_index(0),
   is_dynamic(false)
{
   UINT len = num_indices * sizeof(WORD);

   if (video && len) {
      is_dynamic   = (usage & D3DUSAGE_DYNAMIC) ? true : false;
      D3DPOOL pool = is_dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;

      HRESULT hr = video->D3DDevice()->CreateIndexBuffer(len, 
                                                         usage,
                                                         D3DFMT_INDEX16,
                                                         pool,
                                                         &index_buffer,
                                                         0);

      if (FAILED(hr)) {
         static int report = 10;
         if (report) {
            VideoDX9Error("Could not create index buffer.", hr);
            report--;
         }

         num_indices = 0;
         next_index  = 0;
      }
   }
}

VideoDX9IndexBuffer::~VideoDX9IndexBuffer()
{
   RELEASE(index_buffer);
}

// +--------------------------------------------------------------------+

WORD*
VideoDX9IndexBuffer::Lock(UINT count)
{
   if (index_buffer && count <= num_indices) {
      DWORD flags = 0;

      if (count == 0)
         count = num_indices;
      
      if (is_dynamic) {
         flags = D3DLOCK_NOOVERWRITE;

         if (next_index + count > num_indices) {
            next_index = 0;
            flags = D3DLOCK_DISCARD;
         }
      }

      void*    result = 0;
      HRESULT  hr     = 0;

      hr = index_buffer->Lock(next_index * 2,
                              count      * 2,
                              &result,
                              flags);

      if (SUCCEEDED(hr)) {
         num_locked = count;
         return (WORD*) result;
      }
   }

   return 0;
}

void
VideoDX9IndexBuffer::Unlock()
{
   if (index_buffer && num_locked > 0) {
      index_buffer->Unlock();

      next_index += num_locked;
      num_locked = 0;
   }
}

bool
VideoDX9IndexBuffer::Select()
{
   if (video && index_buffer) {
      if (num_locked > 0)
         Unlock();

      HRESULT hr = video->D3DDevice()->SetIndices(index_buffer);

      if (SUCCEEDED(hr))
         return true;
   }

   return false;
}

// +--------------------------------------------------------------------+

UINT
VideoDX9IndexBuffer::GetNumIndices() const
{
   return num_indices;
}

UINT
VideoDX9IndexBuffer::GetNextIndex() const
{
   return next_index;
}

