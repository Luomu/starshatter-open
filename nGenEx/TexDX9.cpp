/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         TexDX9.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Direct3D Texture Cache
*/

#include "MemDebug.h"
#include "TexDX9.h"
#include "VideoDX9.h"
#include "Bitmap.h"
#include "Color.h"

// +--------------------------------------------------------------------+

void  Print(const char* fmt, ...);
void  VideoDX9Error(const char* msg, HRESULT err);

#define TEXDX9_VERBOSE 0
#define DX9_TEXTURE_CACHE_SIZE 256

#ifndef RELEASE
#define RELEASE(x) if (x) { x->Release(); x=NULL; }
#endif

// +--------------------------------------------------------------------+

void
TexCacheDX9Entry::Release()
{
	RELEASE(texture);
}

// +--------------------------------------------------------------------+

void
TexCacheDX9Entry::Unload()
{
	RELEASE(texture);
	bitmap_id      = 0;
	used_last      = 0;
	last_modified  = 0;
}         

// +--------------------------------------------------------------------+

TexCacheDX9::TexCacheDX9(VideoDX9* v)
: video(v), count(0), mru(0)
{
	d3d       = video->Direct3D();
	d3ddevice = video->D3DDevice();

	// clear the texture cache
	cache = new(__FILE__,__LINE__) TexCacheDX9Entry[DX9_TEXTURE_CACHE_SIZE];
	vidmem = video->VidMemFree();
}

TexCacheDX9::~TexCacheDX9()
{
	int used = 0;

	if (cache) {
		for (int i = 0; i < DX9_TEXTURE_CACHE_SIZE; i++) {
			if (cache[i].bitmap_id)
			used++;
			cache[i].Unload();
		}

		delete [] cache;
		cache = 0;
	}

	Print("TexCacheDX9: final used count = %d\n", used);
}

// +--------------------------------------------------------------------+

bool
TexCacheDX9::LoadTexture(Bitmap* bmp, TexCacheDX9Entry* entry)
{
	if (!d3ddevice) return false;

	HRESULT hr = D3D_OK;

	// create the texture, if necessary
	if (!entry->texture || entry->bitmap_id != bmp->Handle()) {
		hr = d3ddevice->CreateTexture(bmp->Width(),
		bmp->Height(),
		1,                // one mip level
		0,                // no specific usage
		D3DFMT_A8R8G8B8,  // format matching Color::rgba
		D3DPOOL_MANAGED,
		&entry->texture,
		0);

		if (FAILED(hr) || !entry->texture) {
			VideoDX9Error("LoadTexture - could not create texture", hr);
			return false;
		}
	}

	// lock the surface for writing
	D3DLOCKED_RECT locked_rect;
	hr = entry->texture->LockRect(0, &locked_rect, 0, 0);

	if (FAILED(hr)) {
		VideoDX9Error("LoadTexture - could not lock texture surface", hr);
		entry->Unload();
		return false;
	}

	// load the bitmap into the texture surface
	for (int i = 0; i < bmp->Height(); i++) {
		BYTE* src = (BYTE*) (bmp->HiPixels()  + i * bmp->Width());
		BYTE* dst = (BYTE*) locked_rect.pBits + i * locked_rect.Pitch;

		CopyMemory(dst, src, bmp->Width() * sizeof(Color));
	}

	// unlock the surface
	entry->texture->UnlockRect(0);

	entry->last_modified = bmp->LastModified();
	vidmem = video->VidMemFree();
	return true;
}

// +--------------------------------------------------------------------+

void
TexCacheDX9::CreateNormalMap(int index, float amp)
{
	if (d3ddevice && cache[index].texture && !cache[index].normal) {
		HRESULT            hr    = D3D_OK;
		TexCacheDX9Entry*  entry = &cache[index];
		Bitmap*            bmp   = Bitmap::GetBitmapByID(entry->bitmap_id);

		IDirect3DTexture9* normal_map = 0;

		// create the normal map texture
		hr = d3ddevice->CreateTexture(bmp->Width(),
		bmp->Height(),
		1,                // one mip levels
		0,                // no specific usage
		D3DFMT_A8R8G8B8,  // format matching Color::rgba
		D3DPOOL_MANAGED,
		&normal_map,
		0);

		if (FAILED(hr) || !normal_map) {
			VideoDX9Error("CreateNormalMap - could not create texture", hr);
			return;
		}

		D3DXComputeNormalMap(normal_map, entry->texture ,NULL, 0, D3DX_CHANNEL_RED, amp);

		entry->texture->Release();
		entry->texture = normal_map;
		entry->normal  = true;

		// The D3DX function destroys the alpha channel data
		// when it builds the normal map.  We want the original
		// height data stored in the alpha channel, so we need
		// to add it back in now.

		// lock the surface for writing
		D3DLOCKED_RECT locked_rect;
		hr = normal_map->LockRect(0, &locked_rect, 0, 0);

		if (FAILED(hr)) {
			VideoDX9Error("CreateNormalMap - could not insert height channel", hr);
			return;
		}

		// load the bitmap into the texture surface
		for (int i = 0; i < bmp->Height(); i++) {
			BYTE* src = (BYTE*) (bmp->HiPixels()  + i * bmp->Width());
			BYTE* dst = (BYTE*) locked_rect.pBits + i * locked_rect.Pitch;

			src += 2; // red channel
			dst += 3; // alpha channel

			for (int n = 0; n < bmp->Width(); n++) {
				*dst = *src;

				dst += 4;
				src += 4;
			}
		}

		// unlock the surface
		normal_map->UnlockRect(0);
	}
}

// +--------------------------------------------------------------------+

IDirect3DTexture9*
TexCacheDX9::FindTexture(Bitmap* bmp)
{
	int avail = -1;

	if (!bmp)
	return 0;

	// check most recently used:
	if (cache[mru].bitmap_id == bmp->Handle()) {
		cache[mru].used_last = frame_number;

		// need to refresh?
		if (cache[mru].last_modified < bmp->LastModified()) {
			LoadTexture(bmp, &cache[mru]);
			cache[mru].normal = false;
		}

		return cache[mru].texture;
	}

	// find cache entry, or first free:
	for (int i = 0; i < DX9_TEXTURE_CACHE_SIZE; i++)
	if (cache[i].bitmap_id == bmp->Handle()) {
		cache[i].used_last = frame_number;
		mru = i;

		// need to refresh?
		if (cache[mru].last_modified < bmp->LastModified()) {
			LoadTexture(bmp, &cache[mru]);
			cache[mru].normal = false;
		}

		return cache[i].texture;
	}
	else if (avail < 0 && cache[i].bitmap_id == 0)
	avail = i;

	// no free space
	if (avail < 0)
	if (FreeUpCache())
	return FindTexture(bmp);
	else
	return 0;

	TexCacheDX9Entry* entry = &cache[avail];
	entry->bitmap_id = bmp->Handle();
	entry->used_last = frame_number;

	if (LoadTexture(bmp, entry)) {
#if TEXDX9_VERBOSE
		Print("   Tex %3d: id=%2d, size=%3d, type=%d, hTex=%3d, frame=%6d   vmf=%8d\n",
		avail, bmp->Handle(), bmp->Width(), bmp->Type(),
		cache[avail].texture, cache[avail].used_last, vidmem);
#endif
		mru = avail;
		cache[mru].normal = false;
		return entry->texture;
	}
	else {
		// failed to load texture,
		// erase cache entry:
		entry->Unload();
	}

	return 0;
}

// +--------------------------------------------------------------------+

IDirect3DTexture9*
TexCacheDX9::FindNormalMap(Bitmap* bmp, float amp)
{
	int avail = -1;

	if (!bmp)
	return 0;

	// check most recently used:
	if (cache[mru].bitmap_id == bmp->Handle()) {
		cache[mru].used_last = frame_number;

		// need to refresh?
		if (cache[mru].last_modified < bmp->LastModified()) {
			LoadTexture(bmp, &cache[mru]);
			cache[mru].normal = false;
		}

		if (!cache[mru].normal)
		CreateNormalMap(mru, amp);

		return cache[mru].texture;
	}

	// find cache entry, or first free:
	for (int i = 0; i < DX9_TEXTURE_CACHE_SIZE; i++)
	if (cache[i].bitmap_id == bmp->Handle()) {
		cache[i].used_last = frame_number;
		mru = i;

		// need to refresh?
		if (cache[i].last_modified < bmp->LastModified()) {
			LoadTexture(bmp, &cache[mru]);
			cache[i].normal = false;
		}

		if (!cache[i].normal)
		CreateNormalMap(i, amp);

		return cache[i].texture;
	}
	else if (avail < 0 && cache[i].bitmap_id == 0)
	avail = i;

	// no free space
	if (avail < 0)
	if (FreeUpCache())
	return FindTexture(bmp);
	else
	return 0;

	TexCacheDX9Entry* entry = &cache[avail];
	entry->bitmap_id = bmp->Handle();
	entry->used_last = frame_number;

	if (LoadTexture(bmp, entry)) {
#if TEXDX9_VERBOSE
		Print("   Tex %3d: id=%2d, size=%3d, type=%d, hTex=%3d, frame=%6d   vmf=%8d\n",
		avail, bmp->Handle(), bmp->Width(), bmp->Type(),
		cache[avail].texture, cache[avail].used_last, vidmem);
#endif
		mru = avail;
		CreateNormalMap(mru, amp);
		return entry->texture;
	}
	else {
		// failed to load texture,
		// erase cache entry:
		entry->Unload();
	}

	return 0;
}

// +--------------------------------------------------------------------+

int
TexCacheDX9::FreeLRU(int tries)
{
	int   unloaded = 0;

	while (tries--) {
		int   oldest   = -1;
		DWORD old      = frame_number;

		for (int i = 0; i < DX9_TEXTURE_CACHE_SIZE; i++) {
			DWORD ul = cache[i].used_last;
			
			if (ul && ul < old && ul != frame_number) {
				old = ul;
				oldest = i;
			}
		}

		if (oldest >= 0) {
			cache[oldest].Unload();
			unloaded++;
		}
		else
		break;
	}

	vidmem = video->VidMemFree();

#if TEXDX9_VERBOSE
	Print("   FreeLRU() frame=%6d unloaded=%2d vmf=%8d\n", frame_number, unloaded, vidmem);
#endif

	return unloaded;
}

// +--------------------------------------------------------------------+

int
TexCacheDX9::FreeUpCache()
{
	int unloaded = 0;

	for (int i = 0; i < DX9_TEXTURE_CACHE_SIZE; i++) {
		if (cache[i].used_last && cache[i].used_last < frame_number) {
			cache[i].Unload();
			unloaded++;
		}
	}

	vidmem = video->VidMemFree();

	Print("   FreeUpCache() frame=%6d unloaded=%2d vmf=%8d\n", frame_number, unloaded, vidmem);

	return unloaded;
}

// +--------------------------------------------------------------------+

void
TexCacheDX9::InvalidateCache()
{
	for (int i = 0; i < DX9_TEXTURE_CACHE_SIZE; i++) {
		cache[i].Unload();
		cache[i].normal = false;
	}

	vidmem = video->VidMemFree();
}
