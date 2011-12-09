/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         AviFile.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	AviFile reader / writer
*/


#include "MemDebug.h"
#include "AviFile.h"

#include <vfw.h>
#include <stdio.h>
#include <stdlib.h>

// +--------------------------------------------------------------------+

void Print(const char* fmt, ...);

// +--------------------------------------------------------------------+

AviFile::AviFile(const char* fname)
: filename(fname), fps(0), play(true), pfile(0), ps(0), ps_comp(0),
nframe(0), nsamp(0), iserr(false), frame_size(0)
{
	AVIFileInit();
}

AviFile::AviFile(const char* fname, const Rect& r, int frame_rate)
: filename(fname), rect(r), fps(frame_rate), play(false), pfile(0),
ps(0), ps_comp(0), nframe(0), nsamp(0), iserr(false)
{
	Print("\n\nAviFile(%s, w=%d, h=%d, f=%d FPS)\n", fname, r.w, r.h, fps);
	frame_size = r.w * r.h * 3;

	AVIFileInit();
	HRESULT hr = AVIFileOpen(&pfile, fname, OF_WRITE|OF_CREATE, 0);

	if (hr != AVIERR_OK) {
		Print("AviFile - open failed. %08x\n", hr);
		iserr = true;
		return;
	}

	Print("AviFile - open successful\n");

	AVISTREAMINFO strhdr; 
	ZeroMemory(&strhdr,sizeof(strhdr));

	strhdr.fccType    = streamtypeVIDEO;
	strhdr.fccHandler = 0; 
	strhdr.dwScale    = 1000 / fps;
	strhdr.dwRate     = 1000;
	strhdr.dwSuggestedBufferSize  = frame_size;

	SetRect(&strhdr.rcFrame, 0, 0, r.w, r.h);

	hr = AVIFileCreateStream(pfile, &ps, &strhdr);

	if (hr != AVIERR_OK) {
		Print("AviFile - create stream failed. %08x\n", hr);
		iserr = true;
		return;
	}

	Print("AviFile - create stream successful\n");

	AVICOMPRESSOPTIONS opts; 
	ZeroMemory(&opts,sizeof(opts));
	opts.fccType      = streamtypeVIDEO;
	//opts.fccHandler = mmioFOURCC('W','M','V','3');
	opts.fccHandler   = mmioFOURCC('D','I','B',' ');  // (full frames)
	opts.dwFlags      = 8;

	hr = AVIMakeCompressedStream(&ps_comp, ps, &opts, 0);
	if (hr != AVIERR_OK) {
		Print("AviFile - compressed stream failed. %08x\n", hr);
		iserr=true; 
		return;
	}

	Print("AviFile - make compressed stream successful\n");

	BITMAPINFOHEADER bmih;
	ZeroMemory(&bmih, sizeof(BITMAPINFOHEADER));

	bmih.biSize          = sizeof(BITMAPINFOHEADER);
	bmih.biBitCount      = 24;
	bmih.biCompression   = BI_RGB;
	bmih.biWidth         = rect.w;
	bmih.biHeight        = rect.h;
	bmih.biPlanes        = 1;
	bmih.biSizeImage     = frame_size;

	hr = AVIStreamSetFormat(ps_comp, 0, &bmih, sizeof(BITMAPINFOHEADER));

	if (hr != AVIERR_OK) {
		Print("AviFile - stream format failed. %08x\n", hr);
		iserr=true; 
		return;
	}

	Print("AviFile - stream format successful\n");
}

AviFile::~AviFile()
{
	if (!play) {
		Print("*** Closing AVI file '%s' with %d frames\n", (const char*) filename, nframe);
	}

	if (ps_comp)   AVIStreamRelease(ps_comp);
	if (ps)        AVIStreamRelease(ps);
	if (pfile)     AVIFileRelease(pfile);

	AVIFileExit();
}

// +--------------------------------------------------------------------+
//
// Note that AVI frames use DIB format - Y is inverted.
// So we need to flip the native bmp before sending to the
// file.

HRESULT
AviFile::AddFrame(const Bitmap& bmp)
{
	HRESULT hr = E_FAIL;

	if (!iserr && !play && bmp.IsHighColor() && bmp.Width() == rect.w && bmp.Height() == rect.h) {
		int      w      = bmp.Width();
		int      h      = bmp.Height();
		BYTE*    buffer = new(__FILE__,__LINE__) BYTE[frame_size];
		BYTE*    dst    = buffer;

		for (int y = 0; y < bmp.Height(); y++) {
			Color* src = bmp.HiPixels()  + (h - 1 - y) * w;

			for (int x = 0; x < bmp.Width(); x++) {
				*dst++ = (BYTE) src->Blue();
				*dst++ = (BYTE) src->Green();
				*dst++ = (BYTE) src->Red();
				src++;
			}
		}

		hr = AVIStreamWrite(ps_comp, nframe, 1, buffer, frame_size, AVIIF_KEYFRAME, 0, 0);
		
		if (SUCCEEDED(hr)) {
			nframe++;
		}
		else {
			Print("AVIStreamWriteFile failed. %08x\n", hr);
			iserr = true;
		}

		delete [] buffer;
	}

	return hr;
}

// +--------------------------------------------------------------------+

HRESULT
AviFile::GetFrame(double seconds, Bitmap& bmp)
{
	HRESULT hr = E_FAIL;
	return hr;
}

