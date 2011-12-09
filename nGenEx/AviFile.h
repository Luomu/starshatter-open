/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         AviFile.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    PCX image file loader
*/

#ifndef AviFile_h
#define AviFile_h

#include "Text.h"
#include "Color.h"
#include "Bitmap.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

struct IAVIFile;
struct IAVIStream;

// +--------------------------------------------------------------------+

class AviFile
{
public:
   static const char* TYPENAME() { return "AviFile"; }

   // open for reading:
   AviFile(const char* fname);

   // create for writing
   AviFile(const char* fname, const Rect& rect, int frame_rate=30);
   ~AviFile();

   HRESULT  AddFrame(const Bitmap& bmp);
   HRESULT  GetFrame(double seconds, Bitmap& bmp);

private:
	Rect           rect;
	Text           filename;
	int            fps;  

	IAVIFile*      pfile;         // created by CreateAvi
	IAVIStream*    ps; 
	IAVIStream*    ps_comp;       // video stream, when first created
	DWORD          frame_size;    // total bytes per frame of video
	DWORD          nframe;        // which frame will be added next
	DWORD          nsamp;         // which sample will be added next
	bool           play;
	bool           iserr;         // if true, then no function will do anything
};

// +--------------------------------------------------------------------+


#endif AviFile_h
