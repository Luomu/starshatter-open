/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Bitmap.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Bitmap Resource class
*/

#ifndef Bitmap_h
#define Bitmap_h

#include "Res.h"
#include "Types.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Color;
class ColorIndex;

class Bitmap : public Resource
{
public:
	static const char* TYPENAME() { return "Bitmap"; }

	enum BMP_TYPES { BMP_SOLID, BMP_TRANSPARENT, BMP_TRANSLUCENT };

	Bitmap();
	Bitmap(int w, int h, ColorIndex* p=0, int t=BMP_SOLID);
	Bitmap(int w, int h, Color*      p,   int t=BMP_SOLID);
	virtual ~Bitmap();

	int            IsIndexed()       const { return pix   != 0;        }
	int            IsHighColor()     const { return hipix != 0;        }
	int            IsDual()          const { return IsIndexed() &&
		IsHighColor();     }

	void           SetType(int t)          { type = t;                 }
	int            Type()            const { return type;              }
	bool           IsSolid()         const { return type==BMP_SOLID;       }
	bool           IsTransparent()   const { return type==BMP_TRANSPARENT; }
	bool           IsTranslucent()   const { return type==BMP_TRANSLUCENT; }

	int            Width()           const { return width;   }
	int            Height()          const { return height;  }
	ColorIndex*    Pixels()          const { return pix;     }
	Color*         HiPixels()        const { return hipix;   }
	int            BmpSize()         const;
	int            RowSize()         const;

	ColorIndex     GetIndex(int x, int y) const;
	Color          GetColor(int x, int y) const;
	void           SetIndex(int x, int y, ColorIndex c);
	void           SetColor(int x, int y, Color      c);

	void           FillColor(Color c);

	void           ClearImage();
	void           BitBlt(int x, int y, const Bitmap& srcImage, int sx, int sy, int w, int h, bool blend=false);
	void           CopyBitmap(const Bitmap& rhs);
	void           CopyImage(int w, int h, BYTE* p, int t=BMP_SOLID);
	void           CopyHighColorImage(int w, int h, DWORD* p, int t=BMP_SOLID);
	void           CopyAlphaImage(int w, int h, BYTE* p);
	void           CopyAlphaRedChannel(int w, int h, DWORD* p);
	void           AutoMask(DWORD mask=0);

	virtual BYTE*  GetSurface();
	virtual int    Pitch() const;
	virtual int    PixSize() const;
	bool           ClipLine(int& x1, int& y1, int& x2, int& y2);
	bool           ClipLine(double& x1, double& y1, double& x2, double& y2);
	void           DrawLine(int x1, int y1, int x2, int y2, Color color);
	void           DrawRect(int x1, int y1, int x2, int y2, Color color);
	void           DrawRect(const Rect& r, Color color);
	void           FillRect(int x1, int y1, int x2, int y2, Color color);
	void           FillRect(const Rect& r, Color color);
	void           DrawEllipse(int x1, int y1, int x2, int y2, Color color, BYTE quad=0x0f);
	void           DrawEllipsePoints(int x0, int y0, int x, int y, Color c, BYTE quad);

	void           ScaleTo(int w, int h);
	void           MakeIndexed();
	void           MakeHighColor();
	void           MakeTexture();
	bool           IsTexture() const       { return texture; }
	void           TakeOwnership()         { ownpix = true; }

	const char*    GetFilename() const     { return filename; }
	void           SetFilename(const char* s);

	DWORD          LastModified()    const { return last_modified; }

	static Bitmap* Default();

	static Bitmap* GetBitmapByID(HANDLE bmp_id);
	static Bitmap* CheckCache(const char* filename);
	static void    AddToCache(Bitmap* bmp);
	static void    CacheUpdate();
	static void    ClearCache();
	static DWORD   CacheMemoryFootprint();

protected:
	int            type;
	int            width;
	int            height;
	int            mapsize;

	bool           ownpix;
	bool           alpha_loaded;
	bool           texture;

	ColorIndex*    pix;
	Color*         hipix;
	DWORD          last_modified;
	char           filename[64];
};

#endif Bitmap_h

