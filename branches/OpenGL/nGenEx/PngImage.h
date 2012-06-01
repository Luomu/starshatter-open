/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         PngImage.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	PNG image file loader
*/

#ifndef PngImage_h
#define PngImage_h

#include "png.h"

// +--------------------------------------------------------------------+

enum { PNG_OK, PNG_NOMEM, PNG_INVALID, PNG_NOFILE };

// +--------------------------------------------------------------------+

struct PngImage
{
	static const char* TYPENAME() { return "PngImage"; }

	PngImage();
	~PngImage();

	int Load(char *filename);
	int LoadBuffer(unsigned char* buf, int len);
	int CreateImage(png_structp png_ptr, png_infop info_ptr);

	DWORD*         image;
	DWORD          width;
	DWORD          height;
	DWORD          bpp;
	bool           alpha_loaded;
};

// +--------------------------------------------------------------------+

#endif PngImage_h
