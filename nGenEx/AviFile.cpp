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

#pragma warning(suppress: 6001)
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

