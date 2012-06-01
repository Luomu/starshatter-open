/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         ImgView.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Bitmap "Billboard" View class
*/

#ifndef ImgView_h
#define ImgView_h

#include "Types.h"
#include "View.h"

// +--------------------------------------------------------------------+

class  Bitmap;

// +--------------------------------------------------------------------+

class ImgView : public View
{
public:
	static const char* TYPENAME() { return "ImgView"; }

	ImgView(Window* c, Bitmap* bmp);
	virtual ~ImgView();

	// Operations:
	virtual void      Refresh();

	virtual Bitmap*   GetPicture()   const { return img;     }
	virtual void      SetPicture(Bitmap* bmp);
	virtual int       GetBlend()     const { return blend;   }
	virtual void      SetBlend(int b)      { blend = b;      }

protected:
	Bitmap*     img;
	int         x_offset, y_offset;
	int         width, height;
	int         blend;
};

#endif ImgView_h

