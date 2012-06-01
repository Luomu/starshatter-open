/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         D3DXImage.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	D3DX image file loader
*/

#ifndef D3DXImage_h
#define D3DXImage_h

#include "Types.h"

// +--------------------------------------------------------------------+

struct D3DXImage
{
	static const char* TYPENAME() { return "D3DXImage"; }

	D3DXImage();
	D3DXImage(WORD w, WORD h, DWORD* img);
	~D3DXImage();

	bool Load(char *filename);
	bool Save(char *filename);

	bool LoadBuffer(BYTE* buf, int len);

	DWORD*         image;
	DWORD          width;
	DWORD          height;
	DWORD          format;
};

// +--------------------------------------------------------------------+

#endif D3DXImage_h
