/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         D3DXImage.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	D3DX image file loader
*/

#include <string>
#include "afxdlgs.h"
#include "MemDebug.h"
#include "D3DXImage.h"
#include "VideoDX9.h"

// +--------------------------------------------------------------------+

D3DXImage::D3DXImage()
: width(0), height(0), format(0), image(0)
{ }

D3DXImage::D3DXImage(WORD w, WORD h, DWORD* img)
{
	ZeroMemory(this, sizeof(D3DXImage));

	width                = w;
	height               = h;

	int pixels           = width * height;

	image = new(__FILE__,__LINE__) DWORD [pixels];

	if (image && pixels) {
		for (int i = 0; i < pixels; i++)
		image[i] = img[i];
	}
}

D3DXImage::~D3DXImage()
{
	delete [] image;
}

// +--------------------------------------------------------------------+

bool D3DXImage::Load(char *filename)
{
	bool success = false;
	FILE* f;

	fopen_s(&f, filename,"rb");
	if (f == NULL) {
		std::string errstring = "Magic was unable to locate the following texture: ";
		errstring += filename;
		errstring += "\nPlease locate the texture, and copy it to the same directory the Magic executable is in.";

		MessageBox(NULL, errstring.c_str(), "Help!", MB_ICONWARNING | MB_OK);
		return success;
	}

	int   len = 0;
	BYTE* buf = 0;

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = new(__FILE__,__LINE__) BYTE[len];

	if (buf) {
		fread(buf, len, 1, f);
		fclose(f);

		success = LoadBuffer(buf, len);
	}

	return success;
}

// +--------------------------------------------------------------------+

bool D3DXImage::LoadBuffer(BYTE* buf, int len)
{
	bool           success = false;
	HRESULT        hr      = E_FAIL;
	D3DXIMAGE_INFO info;

	if (buf == NULL)
	return success;

	hr = D3DXGetImageInfoFromFileInMemory(buf, len, &info);

	if (FAILED(hr))
	return success;

	width    = info.Width;
	height   = info.Height;
	format   = info.Format;

	if (image) {
		delete [] image;
		image = 0;
	}

	IDirect3DSurface9* surf = 0;
	IDirect3DDevice9*  dev  = VideoDX9::GetD3DDevice9();


	hr = dev->CreateOffscreenPlainSurface( width, 
	height, 
	D3DFMT_A8R8G8B8, 
	D3DPOOL_SYSTEMMEM, 
	&surf, 
	NULL);

	if (FAILED(hr))
	return success;

	hr = D3DXLoadSurfaceFromFileInMemory(  surf,             // dest surface
	NULL,             // dest palette (none)
	NULL,             // dest rect (entire image)
	buf,              // memory file
	len,              // size of data
	NULL,             // source rect (entire image)
	D3DX_DEFAULT,     // filter operation
	0,                // color key (none)
	NULL);            // image info

	if (SUCCEEDED(hr)) {
		D3DLOCKED_RECT locked_rect;
		hr = surf->LockRect(&locked_rect, NULL, D3DLOCK_READONLY);

		if (SUCCEEDED(hr)) {
			// copy surface into image
			image = new(__FILE__,__LINE__) DWORD[width*height];
			if (image) {
				for (DWORD i = 0; i < height; i++) {
					BYTE* dst = (BYTE*) (image            + i * width);
					BYTE* src = (BYTE*) locked_rect.pBits + i * locked_rect.Pitch;

					CopyMemory(dst, src, width * sizeof(DWORD));
				}

				success = true;
			}

			surf->UnlockRect();
		}
	}

	surf->Release();
	surf = 0;

	return success;
}

// +--------------------------------------------------------------------+

bool D3DXImage::Save(char *filename)
{
	bool           success = false;
	HRESULT        hr      = E_FAIL;

	if (!image || !width || !height)
	return success;

	FILE* f;
	fopen_s(&f, filename,"wb");
	if (f == NULL)
	return success;

	IDirect3DSurface9* surf = 0;
	IDirect3DDevice9*  dev  = VideoDX9::GetD3DDevice9();

	hr = dev->CreateOffscreenPlainSurface( width, 
	height, 
	D3DFMT_A8R8G8B8, 
	D3DPOOL_SYSTEMMEM, 
	&surf, 
	NULL);


	if (SUCCEEDED(hr)) {
		D3DLOCKED_RECT locked_rect;
		hr = surf->LockRect(&locked_rect, NULL, 0);

		if (SUCCEEDED(hr)) {
			// copy image into surface
			for (DWORD i = 0; i < height; i++) {
				BYTE* src = (BYTE*) (image            + i * width);
				BYTE* dst = (BYTE*) locked_rect.pBits + i * locked_rect.Pitch;

				CopyMemory(dst, src, width * sizeof(DWORD));
			}

			surf->UnlockRect();

			ID3DXBuffer*         buffer = 0;
			D3DXIMAGE_FILEFORMAT imgfmt = D3DXIFF_PNG;

			if (strstr(filename, ".jpg") || strstr(filename, ".JPG"))
			imgfmt = D3DXIFF_JPG;

			else if (strstr(filename, ".bmp") || strstr(filename, ".BMP"))
			imgfmt = D3DXIFF_BMP;

			hr = D3DXSaveSurfaceToFileInMemory(&buffer,  // destination
			imgfmt,  // type of file
			surf,    // image to save
			NULL,    // palette
			NULL);   // source rect (entire image)

			if (SUCCEEDED(hr)) {
				fwrite(buffer->GetBufferPointer(), buffer->GetBufferSize(), 1, f);
				success = true;
			}
		}
	}

	surf->Release();
	surf = 0;
	fclose(f);
	return success;
}

