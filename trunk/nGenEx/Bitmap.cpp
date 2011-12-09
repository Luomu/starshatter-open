/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Bitmap.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Bitmap Resource class
*/

#include "MemDebug.h"
#include "Bitmap.h"
#include "Video.h"
#include "Color.h"
#include "Game.h"

// +--------------------------------------------------------------------+

DWORD GetRealTime();

static inline void swap(int& a, int& b) { int tmp=a; a=b; b=tmp; }
static inline void sort(int& a, int& b) { if (a>b) swap(a,b);    }
static inline void swap(double& a, double& b) { double tmp=a; a=b; b=tmp; }
static inline void sort(double& a, double& b) { if (a>b) swap(a,b);    }
static void draw_strip(BYTE* dst, int pitch, int pixsize, int x, int y, int len, Color color);
static void draw_vline(BYTE* dst, int pitch, int pixsize, int x, int y, int len, Color color);

class WinPlot
{
public:
	WinPlot(Bitmap* bmp);
	void plot(int x, int y, DWORD val, int exch=0);

private:
	BYTE* s;
	int   pitch, pixsize;
};

WinPlot::WinPlot(Bitmap* bmp)
{
	s       = bmp->GetSurface();
	pitch   = bmp->Pitch();
	pixsize = bmp->PixSize();
}

void WinPlot::plot(int x, int y, DWORD val, int exch)
{
	if (exch) swap(x,y);
	BYTE* dst = s + y*pitch + x*pixsize;

	switch (pixsize) {
	case 1: *dst = (BYTE) val; break;
	case 2: { LPWORD  dst2 = (LPWORD)  dst; *dst2 = (WORD)  val; } break;
	case 4: { LPDWORD dst4 = (LPDWORD) dst; *dst4 = (DWORD) val; } break;
	}
}

// +--------------------------------------------------------------------+

Bitmap::Bitmap()
: type(BMP_SOLID), width(0), height(0),
ownpix(false), alpha_loaded(false), texture(false),
pix(0), hipix(0), mapsize(0),
last_modified(0)
{
	strcpy_s(filename, "Bitmap()");
}

Bitmap::Bitmap(int w, int h, ColorIndex* p, int t)
: type(t), width(w), height(h),
ownpix(false), alpha_loaded(false), texture(false),
pix(p), hipix(0), mapsize(w*h),
last_modified(GetRealTime())
{
	sprintf_s(filename, "Bitmap(%d, %d, index, type=%d)", w, h, (int) t);
}

Bitmap::Bitmap(int w, int h, Color* p, int t)
: type(t), width(w), height(h),
ownpix(false), alpha_loaded(false), texture(false),
pix(0), hipix(p), mapsize(w*h),
last_modified(GetRealTime())
{
	sprintf_s(filename, "Bitmap(%d, %d, hicolor, type=%d)", w, h, (int) t);
}

// +--------------------------------------------------------------------+

Bitmap::~Bitmap()
{
	if (ownpix) {
		delete [] pix;
		delete [] hipix;
	}
}

// +--------------------------------------------------------------------+

int
Bitmap::BmpSize() const
{
	return mapsize * PixSize();
}

int
Bitmap::RowSize() const
{
	return width;
}

int
Bitmap::Pitch() const
{
	return width * PixSize();
}

int
Bitmap::PixSize() const
{
	if (hipix)
	return sizeof(Color);

	else if (pix)
	return sizeof(ColorIndex);

	return 0;
}

BYTE*
Bitmap::GetSurface()
{
	if (ownpix) {
		if (hipix)
		return (BYTE*) hipix;

		return (BYTE*) pix;
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
Bitmap::BitBlt(int x, int y, const Bitmap& srcBmp, int sx, int sy, int w, int h, bool blend)
{
	if (!ownpix || x < 0 || y < 0 || x >= width || y >= height)
	return;

	if (sx < 0 || sy < 0 || sx >= srcBmp.Width() || sy >= srcBmp.Height())
	return;

	if (hipix) {
		if (srcBmp.HiPixels()) {
			int    dpitch = width;
			int    spitch = srcBmp.Width();
			int    rowlen = w * sizeof(Color);
			Color* dst    = hipix + (y*dpitch) + x;
			Color* src    = srcBmp.HiPixels() + (sy*spitch) + sx;

			if (!blend) {
				for (int i = 0; i < h; i++) {
					memcpy(dst, src, rowlen);
					dst += dpitch;
					src += spitch;
				}
			}
			else {
				for (int i = 0; i < h; i++) {
					Color* ps = src;
					Color* pd = dst;

					for (int n = 0; n < w; n++) {
						if (ps->Value()) 
						pd->Set(Color::FormattedBlend(ps->Value(), pd->Value()));
						ps++;
						pd++;
					}

					dst += dpitch;
					src += spitch;
				}
			}
		}

		else {
			int    dpitch = width;
			int    spitch = srcBmp.Width();
			Color* dst    = hipix + (y*dpitch) + x;
			ColorIndex* src = srcBmp.Pixels() + (sy*spitch) + sx;

			if (!blend) {
				for (int j = 0; j < h; j++) {
					for (int i = 0; i < w; i++) {
						dst[i].Set(src[i].Formatted());
					}

					dst += dpitch;
					src += spitch;
				}
			}
			else {
				for (int i = 0; i < h; i++) {
					ColorIndex* ps = src;
					Color* pd = dst;

					for (int n = 0; n < w; n++) {
						if (ps->Index())
						pd->Set(Color::FormattedBlend(ps->Formatted(), pd->Value()));
						ps++;
						pd++;
					}
				}

				dst += dpitch;
				src += spitch;
			}
		}
	}

	else if (pix) {
		if (srcBmp.Pixels()) {
			int    dpitch = width;
			int    spitch = srcBmp.Width();
			int    rowlen = w;
			Color* dst    = hipix + (y*dpitch) + x;
			Color* src    = srcBmp.HiPixels() + (sy*spitch) + sx;

			for (int i = 0; i < h; i++) {
				memcpy(dst, src, rowlen);
				dst += dpitch;
				src += spitch;
			}
		}
	}

	alpha_loaded  = srcBmp.alpha_loaded;
	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::CopyBitmap(const Bitmap& rhs)
{
	if (ownpix) {
		delete [] pix;
		delete [] hipix;
		pix   = 0;
		hipix = 0;
	}

	type           = rhs.type;
	width          = rhs.width;
	height         = rhs.height;
	alpha_loaded   = rhs.alpha_loaded;
	texture        = rhs.texture;
	ownpix         = true;

	mapsize = width * height;

	if (rhs.pix) {
		pix = new(__FILE__,__LINE__) ColorIndex[mapsize];

		if (!pix) {
			width = 0;
			height = 0;
			mapsize = 0;
		}

		else {
			memcpy(pix, rhs.pix, mapsize*sizeof(ColorIndex));
		}
	}

	if (rhs.hipix) {
		hipix = new(__FILE__,__LINE__) Color[mapsize];

		if (!hipix && !pix) {
			width = 0;
			height = 0;
			mapsize = 0;
		}

		else {
			memcpy(hipix, rhs.hipix, mapsize*sizeof(Color));
		}
	}

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::ClearImage()
{
	if (ownpix) {
		delete [] pix;
		delete [] hipix;
		pix   = 0;
		hipix = 0;
	}

	type    = BMP_SOLID;
	width   = 0;
	height  = 0;
	mapsize = 0;
	ownpix  = false;
	texture = false;

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::CopyImage(int w, int h, BYTE* p, int t)
{
	if (ownpix) {
		delete [] pix;
		pix   = 0;
	}
	else {
		hipix = 0;
	}

	type    = t;
	width   = w;
	height  = h;
	ownpix  = true;
	texture = false;
	mapsize = w * h;

	pix = new(__FILE__,__LINE__) ColorIndex[mapsize];

	if (!pix) {
		width = 0;
		height = 0;
		mapsize = 0;
	}

	else {
		memcpy(pix, p, mapsize);
	}

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::CopyHighColorImage(int w, int h, DWORD* p, int t)
{
	if (ownpix) {
		delete [] hipix;
		hipix = 0;
	}
	else {
		pix   = 0;
	}

	type    = t;
	width   = w;
	height  = h;
	ownpix  = true;
	texture = false;
	mapsize = w * h;

	hipix = new(__FILE__,__LINE__) Color[mapsize];

	if (!hipix) {
		width = 0;
		height = 0;
		mapsize = 0;
	}

	else {
		memcpy(hipix, p, mapsize*sizeof(DWORD));
	}

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::CopyAlphaImage(int w, int h, BYTE* a)
{
	if (!hipix || width != w || height != h)
	return;

	type           = BMP_TRANSLUCENT;
	alpha_loaded   = true;

	Color* p = hipix;

	for (int i = 0; i < mapsize; i++) {
		p->SetAlpha(*a);
		p++;
		a++;
	}

	last_modified = GetRealTime();
}

void
Bitmap::CopyAlphaRedChannel(int w, int h, DWORD* a)
{
	if (!hipix || width != w || height != h)
	return;

	type           = BMP_TRANSLUCENT;
	alpha_loaded   = true;

	Color* p = hipix;

	for (int i = 0; i < mapsize; i++) {
		p->SetAlpha((BYTE) ((*a & Color::RMask) >> Color::RShift));
		p++;
		a++;
	}

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::AutoMask(DWORD mask)
{
	if (!hipix || !mapsize || alpha_loaded)
	return;

	type           = BMP_TRANSLUCENT;
	alpha_loaded   = true;

	Color* p = hipix;
	DWORD  m = mask & Color::RGBMask;

	for (int i = 0; i < mapsize; i++) {
		if ((p->Value() & Color::RGBMask) == m)
		p->SetAlpha(0);
		else
		p->SetAlpha(255);

		p++;
	}

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::FillColor(Color c)
{
	if (!width  || !height)
	return;

	if (pix) {
		ColorIndex* p     = pix;
		BYTE        index = c.Index();

		for (int i = 0; i < mapsize; i++)
		*p++ = index;
	}

	if (hipix) {
		Color*      p     = hipix;
		DWORD       value = c.Value();

		for (int i = 0; i < mapsize; i++) {
			p->Set(value);
			p++;
		}
	}

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::ScaleTo(int w, int h)
{
	if (w < 1 || h < 1)
	return;

	double dx = (double) width  / (double) w;
	double dy = (double) height / (double) h;

	bool mem_ok = true;

	if (hipix) {
		Color*   src = hipix;
		Color*   buf = new(__FILE__,__LINE__) Color[w*h];
		Color*   dst = buf;

		if (!buf) {
			mem_ok = false;
		}

		else {
			for (int y = 0; y < h; y++) {
				int y_offset = (int) (y * dy);
				for (int x = 0; x < w; x++) {
					int x_offset = (int) (x * dx);
					src = hipix + (y_offset * width) + x_offset;
					*dst++ = *src;
				}
			}

			if (ownpix)
			delete [] hipix;

			hipix  = buf;
			ownpix = true;
		}
	}

	if (pix) {
		ColorIndex* src = pix;
		ColorIndex* buf = new(__FILE__,__LINE__) ColorIndex[w*h];
		ColorIndex* dst = buf;

		if (!buf) {
			mem_ok = false;
		}

		else {
			for (int y = 0; y < h; y++) {
				int y_offset = (int) (y * dy);
				for (int x = 0; x < w; x++) {
					int x_offset = (int) (x * dx);
					src = pix + (y_offset * width) + x_offset;
					*dst++ = *src;
				}
			}

			if (ownpix)
			delete [] pix;

			pix    = buf;
			ownpix = true;
		}
	}

	if (mem_ok) {
		width   = w;
		height  = h;
		mapsize = width * height;
	}
}

// +--------------------------------------------------------------------+

void
Bitmap::MakeIndexed()
{
	if (hipix) {
		if (pix && ownpix)
		delete [] pix;
		pix = new(__FILE__,__LINE__) ColorIndex[mapsize];

		if (pix) {
			Color*      src = hipix;
			ColorIndex* dst = pix;

			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					*dst++ = src->Index();
					src++;
				}
			}

			if (!ownpix)
			hipix = 0;

			ownpix = true;
		}
	}
}

// +--------------------------------------------------------------------+

void
Bitmap::MakeHighColor()
{
	if (pix) {
		if (hipix && ownpix)
		delete [] hipix;

		hipix = new(__FILE__,__LINE__) Color[mapsize];

		if (hipix) {
			ColorIndex* src = pix;
			Color*      dst = hipix;

			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					*dst++ = src->Index();
					src++;
				}
			}

			if (!ownpix)
			pix = 0;

			ownpix = true;
		}
	}
}

// +--------------------------------------------------------------------+

static int FindBestTexSize(int n, int max_size)
{
	int delta = 100000;
	int best  = 1;

	for (int i = 0; i < 12; i++) {
		int size = 1 << i;

		if (size > max_size)
		break;

		int dx   = abs(n-size);

		if (size < n)
		dx *= 4;

		if (dx < delta) {
			delta = dx;
			best  = size;
		}
	}

	return best;
}

void
Bitmap::MakeTexture()
{
	if (width < 1 || height < 1 || (!pix && !hipix)) {
		if (ownpix) {
			delete [] pix;
			delete [] hipix;
		}

		width    = 0;
		height   = 0;
		pix      = 0;
		hipix    = 0;
		texture  = false;
		return;
	}

	// texture surface format is 32-bit RGBA:
	if (pix && !hipix) {
		MakeHighColor();
	}

	// check size and aspect ratio:
	int max_tex_size   = Game::MaxTexSize();
	int max_tex_aspect = Game::MaxTexAspect();

	int best_width     = FindBestTexSize(width,  max_tex_size);
	int best_height    = FindBestTexSize(height, max_tex_size);
	int aspect         = 1;

	// correct sizes for aspect if necessary:
	if (best_width > best_height) {
		aspect = best_width / best_height;

		if (aspect > max_tex_aspect)
		best_height = best_width / max_tex_aspect;
	}

	else {
		aspect = best_height / best_width;

		if (aspect > max_tex_aspect)
		best_width = best_height / max_tex_aspect;
	}

	// rescale if necessary:
	if (width != best_width || height != best_height)
	ScaleTo(best_width, best_width);

	texture = true;
}

// +--------------------------------------------------------------------+

ColorIndex
Bitmap::GetIndex(int x, int y) const
{
	ColorIndex result(0);

	if (x < 0 || y < 0 || x > width-1 || y > height-1)
	return result;

	if (pix) {
		result = *(pix + y*width + x);
	}
	else if (hipix) {
		result = (hipix + y*width + x)->Index();
	}

	return result;
}

// +--------------------------------------------------------------------+

Color
Bitmap::GetColor(int x, int y) const
{
	Color result = Color::Black;

	if (x < 0 || y < 0 || x > width-1 || y > height-1)
	return result;

	if (pix) {
		result = (pix + y*width + x)->Index();
	}
	else if (hipix) {
		result = *(hipix + y*width + x);
	}

	return result;
}

// +--------------------------------------------------------------------+

void
Bitmap::SetIndex(int x, int y, ColorIndex c)
{
	if (x < 0 || y < 0 || x > width || y > height)
	return;

	if (pix) {
		*(pix + y*width + x) = c;
	}
	else if (hipix) {
		*(hipix + y*width + x) = c.Index();
	}

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::SetColor(int x, int y, Color c)
{
	if (x < 0 || y < 0 || x > width || y > height)
	return;

	if (pix) {
		*(pix + y*width + x) = c.Index();
	}
	else if (hipix) {
		*(hipix + y*width + x) = c;
	}

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::SetFilename(const char* s)
{
	if (s) {
		int n = strlen(s);

		if (n >= 60) {
			ZeroMemory(filename, sizeof(filename));
			strcpy_s(filename, "...");
			strcat_s(filename, s + n - 59);
			filename[63] = 0;
		}

		else {
			strcpy_s(filename, s);
		}
	}
}

// +--------------------------------------------------------------------+

Bitmap*
Bitmap::Default()
{
	static Bitmap def;

	if (!def.width) {
		def.width = def.height = 64;
		def.mapsize = 64*64;
		def.ownpix = true;
		def.pix = new(__FILE__,__LINE__) ColorIndex[def.mapsize];
		
		// 8 bit palette mode
		if (def.pix) {
			ColorIndex* p = def.pix;
			
			for (int y = 0; y < 64; y++) {
				for (int x = 0; x < 64; x++) {
					double distance = sqrt((x-32.0)*(x-32.0) + (y-32.0)*(y-32.0));
					if (distance > 31.0) distance = 31.0;
					BYTE color = 24 + (BYTE) distance;
					
					if (x == 0 || y == 0) color = 255;
					else if (x == 32 || y == 32) color = 251;

					*p++ = color;
				}
			}
		}
	}

	return &def;
}

// +--------------------------------------------------------------------+

static List<Bitmap>  bitmap_cache;

Bitmap*
Bitmap::GetBitmapByID(HANDLE bmp_id)
{
	for (int i = 0; i < bitmap_cache.size(); i++) {
		if (bitmap_cache[i]->Handle() == bmp_id) {
			return bitmap_cache[i];
		}
	}

	return 0;
}

Bitmap*
Bitmap::CheckCache(const char* filename)
{
	for (int i = 0; i < bitmap_cache.size(); i++) {
		if (!_stricmp(bitmap_cache[i]->GetFilename(), filename)) {
			return bitmap_cache[i];
		}
	}

	return 0;
}

void
Bitmap::AddToCache(Bitmap* bmp)
{
	bitmap_cache.append(bmp);
}

void
Bitmap::CacheUpdate()
{
	for (int i = 0; i < bitmap_cache.size(); i++) {
		Bitmap* bmp = bitmap_cache[i];

		if (bmp->IsTexture())
		bmp->MakeTexture();
	}
}

void
Bitmap::ClearCache()
{
	bitmap_cache.destroy();
}

DWORD
Bitmap::CacheMemoryFootprint()
{
	DWORD result = sizeof(bitmap_cache);
	result += bitmap_cache.size() * sizeof(Bitmap*);

	for (int i = 0; i < bitmap_cache.size(); i++) {
		Bitmap* bmp = bitmap_cache[i];

		if (bmp->pix)
		result += bmp->mapsize * sizeof(ColorIndex);

		if (bmp->hipix)
		result += bmp->mapsize * sizeof(Color);
	}

	return result;
}

// +--------------------------------------------------------------------+

bool
Bitmap::ClipLine(int& x1, int& y1, int& x2, int& y2)
{
	// vertical lines:
	if (x1==x2) {
		clip_vertical:
		sort(y1,y2);
		if (x1 < 0 || x1 >= width) return false;
		if (y1 < 0) y1 = 0;
		if (y2 >= height) y2 = height;
		return true;
	}

	// horizontal lines:
	if (y1==y2) {
		clip_horizontal:
		sort(x1,x2);
		if (y1 < 0 || y1 >= height) return false;
		if (x1 < 0) x1 = 0;
		if (x2 > width) x2 = width;
		return true;
	}

	// general lines:

	// sort left to right:
	if (x1 > x2) {
		swap(x1,x2);
		swap(y1,y2);
	}

	double m = (double)(y2-y1) / (double)(x2-x1);
	double b = (double) y1 - (m * x1);

	// clip:
	if (x1 < 0)        { x1 = 0; y1 = (int) b; }
	if (x1 >= width)   return false;
	if (x2 < 0)        return false;
	if (x2 > width-1)  { x2 = width-1; y2 = (int) (m * x2 + b); }

	if (y1 < 0       && y2 < 0)       return false;
	if (y1 >= height && y2 >= height) return false;

	if (y1 < 0)       { y1 = 0; x1 = (int) (-b/m); }
	if (y1 >= height) { y1 = height-1; x1 = (int) ((y1-b)/m); }
	if (y2 < 0)       { y2 = 0; x2 = (int) (-b/m); }
	if (y2 >= height) { y2 = height-1; x2 = (int) ((y2-b)/m); }

	if (x1 == x2)
	goto clip_vertical;

	if (y1 == y2)
	goto clip_horizontal;   

	return true;
}

// +--------------------------------------------------------------------+

bool
Bitmap::ClipLine(double& x1, double& y1, double& x2, double& y2)
{
	// vertical lines:
	if (x1==x2) {
		clip_vertical:
		sort(y1,y2);
		if (x1 < 0 || x1 >= width) return false;
		if (y1 < 0) y1 = 0;
		if (y2 >= height) y2 = height;
		return true;
	}

	// horizontal lines:
	if (y1==y2) {
		clip_horizontal:
		sort(x1,x2);
		if (y1 < 0 || y1 >= height) return false;
		if (x1 < 0) x1 = 0;
		if (x2 > width) x2 = width;
		return true;
	}

	// general lines:

	// sort left to right:
	if (x1 > x2) {
		swap(x1,x2);
		swap(y1,y2);
	}

	double m = (double)(y2-y1) / (double)(x2-x1);
	double b = (double) y1 - (m * x1);

	// clip:
	if (x1 < 0)        { x1 = 0; y1 = b; }
	if (x1 >= width)   return false;
	if (x2 < 0)        return false;
	if (x2 > width-1)  { x2 = width-1; y2 = (m * x2 + b); }

	if (y1 < 0       && y2 < 0)       return false;
	if (y1 >= height && y2 >= height) return false;

	if (y1 < 0)       { y1 = 0; x1 = (-b/m); }
	if (y1 >= height) { y1 = height-1; x1 = ((y1-b)/m); }
	if (y2 < 0)       { y2 = 0; x2 = (-b/m); }
	if (y2 >= height) { y2 = height-1; x2 = ((y2-b)/m); }

	if (x1 == x2)
	goto clip_vertical;

	if (y1 == y2)
	goto clip_horizontal;   

	return true;
}

// +--------------------------------------------------------------------+

void
Bitmap::DrawLine(int x1, int y1, int x2, int y2, Color color)
{
	BYTE* s = GetSurface();

	if (!s) return;

	last_modified = GetRealTime();

	// vertical lines:
	if (x1==x2) {
		draw_vertical:
		sort(y1,y2);
		int fh = y2-y1;
		if (x1 < 0 || x1 >= width) return;
		if (y1 < 0) y1 = 0;
		if (y2 >= height) y2 = height;
		fh = y2-y1;
		draw_vline(s, Pitch(), PixSize(), x1, y1, fh, color);
		return;
	}

	// horizontal lines:
	if (y1==y2) {
		draw_horizontal:
		sort(x1,x2);
		int fw = x2-x1;
		if (y1 < 0 || y1 >= height) return;
		if (x1 < 0) x1 = 0;
		if (x2 > width) x2 = width;
		fw = x2-x1;
		draw_strip(s, Pitch(), PixSize(), x1, y1, fw, color);
		return;
	}

	// general lines:

	// sort left to right:
	if (x1 > x2) {
		swap(x1,x2);
		swap(y1,y2);
	}

	double m = (double)(y2-y1) / (double)(x2-x1);
	double b = (double) y1 - (m * x1);

	// clip:
	if (x1 < 0)        { x1 = 0; y1 = (int) b; }
	if (x1 >= width)   return;
	if (x2 < 0)        return;
	if (x2 > width-1)  { x2 = width-1; y2 = (int) (m * x2 + b); }

	if (y1 < 0       && y2 < 0)       return;
	if (y1 >= height && y2 >= height) return;

	if (y1 < 0)       { y1 = 0; x1 = (int) (-b/m); }
	if (y1 >= height) { y1 = height-1; x1 = (int) ((y1-b)/m); }
	if (y2 < 0)       { y2 = 0; x2 = (int) (-b/m); }
	if (y2 >= height) { y2 = height-1; x2 = (int) ((y2-b)/m); }

	if (x1 > x2)
	return;

	if (x1 == x2)
	goto draw_vertical;

	if (y1 == y2)
	goto draw_horizontal;   

	// plot the line using
	/*
	Symmetric Double Step Line Algorithm
	by Brian Wyvill
	from "Graphics Gems", Academic Press, 1990
*/

	WinPlot  plotter(this);

	DWORD    pix = color.Value();
	int      sign_x=1, sign_y=1, step, reflect;
	int      i, inc1, inc2, c, D, x_end, pixleft;
	int      dx = x2 - x1;
	int      dy = y2 - y1;

	if (dx < 0) {
		sign_x = -1;
		dx *= -1;
	}
	if (dy < 0) {
		sign_y = -1;
		dy *= -1;
	}

	// decide increment sign by the slope sign
	if (sign_x == sign_y)
	step = 1;
	else
	step = -1;

	if (dy > dx) {          // chooses axis of greatest movement (make * dx)
		swap(x1, y1);
		swap(x2, y2);
		swap(dx, dy);
		reflect = 1;
	} else
	reflect = 0;

	if (x1 > x2) {          // start from the smaller coordinate
		swap(x1,x2);
		swap(y1,y2);
	}

	/* Note dx=n implies 0 - n or (dx+1) pixels to be set */
	/* Go round loop dx/4 times then plot last 0,1,2 or 3 pixels */
	/* In fact (dx-1)/4 as 2 pixels are already plottted */
	x_end   = (dx - 1) / 4;
	pixleft = (dx - 1) % 4;     /* number of pixels left over at the end */

	plotter.plot(x1, y1, pix, reflect);
	plotter.plot(x2, y2, pix, reflect);  /* plot first two points */

	inc2 = 4 * dy - 2 * dx;
	if (inc2 < 0) {        /* slope less than 1/2 */
		c = 2 * dy;
		inc1 = 2 * c;
		D = inc1 - dx;

		for (i = 0; i < x_end; i++) {    /* plotting loop */
			++x1;
			--x2;
			if (D < 0) {
				/* pattern 1 forwards */
				plotter.plot(x1, y1, pix, reflect);
				plotter.plot(++x1, y1, pix, reflect);
				/* pattern 1 backwards */
				plotter.plot(x2, y2, pix, reflect);
				plotter.plot(--x2, y2, pix, reflect);
				D += inc1;
			}
			else {
				if (D < c) {
					/* pattern 2 forwards */
					plotter.plot(x1, y1, pix, reflect);
					plotter.plot(++x1, y1 += step, pix, reflect);
					/* pattern 2 backwards */
					plotter.plot(x2, y2, pix, reflect);
					plotter.plot(--x2, y2 -= step, pix, reflect);
				}
				else {
					/* pattern 3 forwards */
					plotter.plot(x1, y1 += step, pix, reflect);
					plotter.plot(++x1, y1, pix, reflect);
					/* pattern 3 backwards */
					plotter.plot(x2, y2 -= step, pix, reflect);
					plotter.plot(--x2, y2, pix, reflect);
				}
				D += inc2;
			}
		}               /* end for */

		/* plot last pattern */
		if (pixleft) {
			if (D < 0) {
				plotter.plot(++x1, y1, pix, reflect);  /* pattern 1 */
				if (pixleft > 1)
				plotter.plot(++x1, y1, pix, reflect);
				if (pixleft > 2)
				plotter.plot(--x2, y2, pix, reflect);
			}
			else {
				if (D < c) {
					plotter.plot(++x1, y1, pix, reflect);  /* pattern 2  */
					if (pixleft > 1)
					plotter.plot(++x1, y1 += step, pix, reflect);
					if (pixleft > 2)
					plotter.plot(--x2, y2, pix, reflect);
				}
				else {
					/* pattern 3 */
					plotter.plot(++x1, y1 += step, pix, reflect);
					if (pixleft > 1)
					plotter.plot(++x1, y1, pix, reflect);
					if (pixleft > 2)
					plotter.plot(--x2, y2 -= step, pix, reflect);
				}
			}
		}               /* end if pixleft */
	}
	/* end slope < 1/2 */
	else {                  /* slope greater than 1/2 */
		c = 2 * (dy - dx);
		inc1 = 2 * c;
		D = inc1 + dx;
		for (i = 0; i < x_end; i++) {
			++x1;
			--x2;
			if (D > 0) {
				/* pattern 4 forwards */
				plotter.plot(x1, y1 += step, pix, reflect);
				plotter.plot(++x1, y1 += step, pix, reflect);
				/* pattern 4 backwards */
				plotter.plot(x2, y2 -= step, pix, reflect);
				plotter.plot(--x2, y2 -= step, pix, reflect);
				D += inc1;
			} else {
				if (D < c) {
					/* pattern 2 forwards */
					plotter.plot(x1, y1, pix, reflect);
					plotter.plot(++x1, y1 += step, pix, reflect);

					/* pattern 2 backwards */
					plotter.plot(x2, y2, pix, reflect);
					plotter.plot(--x2, y2 -= step, pix, reflect);
				} else {
					/* pattern 3 forwards */
					plotter.plot(x1, y1 += step, pix, reflect);
					plotter.plot(++x1, y1, pix, reflect);
					/* pattern 3 backwards */
					plotter.plot(x2, y2 -= step, pix, reflect);
					plotter.plot(--x2, y2, pix, reflect);
				}
				D += inc2;
			}
		}               /* end for */
		/* plot last pattern */
		if (pixleft) {
			if (D > 0) {
				plotter.plot(++x1, y1 += step, pix, reflect);  /* pattern 4 */
				if (pixleft > 1)
				plotter.plot(++x1, y1 += step, pix, reflect);
				if (pixleft > 2)
				plotter.plot(--x2, y2 -= step, pix, reflect);
			} else {
				if (D < c) {
					plotter.plot(++x1, y1, pix, reflect);  /* pattern 2  */
					if (pixleft > 1)
					plotter.plot(++x1, y1 += step, pix, reflect);
					if (pixleft > 2)
					plotter.plot(--x2, y2, pix, reflect);
				} else {
					/* pattern 3 */
					plotter.plot(++x1, y1 += step, pix, reflect);
					if (pixleft > 1)
					plotter.plot(++x1, y1, pix, reflect);
					if (pixleft > 2) {
						if (D > c) /* step 3 */
						plotter.plot(--x2, y2 -= step, pix, reflect);
						else /* step 2 */
						plotter.plot(--x2, y2, pix, reflect);
					}
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
Bitmap::DrawRect(int x1, int y1, int x2, int y2, Color color)
{
	sort(x1,x2);
	sort(y1,y2);

	int fw = x2-x1;
	int fh = y2-y1;

	if (fw == 0 || fh == 0) return;

	// perform clip
	int left   = (x1 >= 0);
	int right  = (x2 <= width);
	int top    = (y1 >= 0);
	int bottom = (y2 <= height);

	BYTE* s = GetSurface();
	if (!s) return;
	int pitch   = Pitch();
	int pixsize = PixSize();

	if (left)   draw_vline(s, pitch, pixsize, x1, y1, fh, color);
	if (right)  draw_vline(s, pitch, pixsize, x2, y1, fh, color);
	if (top)    draw_strip(s, pitch, pixsize, x1, y1, fw, color);
	if (bottom) draw_strip(s, pitch, pixsize, x1, y2, fw, color);

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::DrawRect(const Rect& r, Color color)
{
	if (r.w == 0 || r.h == 0) return;

	int x1 = r.x;
	int y1 = r.y;
	int x2 = r.x + r.w;
	int y2 = r.y + r.h;

	// perform clip
	int left   = (x1 >= 0);
	int right  = (x2 <= width);
	int top    = (y1 >= 0);
	int bottom = (y2 <= height);

	BYTE* s = GetSurface();
	if (!s) return;
	int pitch   = Pitch();
	int pixsize = PixSize();

	if (left)   draw_vline(s, pitch, pixsize, x1, y1, r.h, color);
	if (right)  draw_vline(s, pitch, pixsize, x2, y1, r.h, color);
	if (top)    draw_strip(s, pitch, pixsize, x1, y1, r.w, color);
	if (bottom) draw_strip(s, pitch, pixsize, x1, y2, r.w, color);

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::FillRect(int x1, int y1, int x2, int y2, Color color)
{
	// perform clip
	if (x1 < 0)       x1 = 0;
	if (x2 > width-1) x2 = width-1;
	if (y1 < 0)       y1 = 0;
	if (y2 > height)  y2 = height;

	int fw = x2-x1;
	int fh = y2-y1;

	if (fw == 0 || fh == 0) return;

	BYTE* s = GetSurface();
	if (!s) return;
	int pitch   = Pitch();
	int pixsize = PixSize();

	for (int i = 0; i < fh; i++)
	draw_strip(s, pitch, pixsize, x1, y1+i, fw, color);

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::FillRect(const Rect& r, Color color)
{
	int x1 = r.x;
	int y1 = r.y;
	int x2 = r.x + r.w;
	int y2 = r.y + r.h;

	// perform clip
	if (x1 < 0)       x1 = 0;
	if (x2 > width-1) x2 = width-1;
	if (y1 < 0)       y1 = 0;
	if (y2 > height)  y2 = height;

	int fw = x2-x1;
	int fh = y2-y1;

	if (fw == 0 || fh == 0) return;

	BYTE* s = GetSurface();
	if (!s) return;
	int pitch   = Pitch();
	int pixsize = PixSize();

	for (int i = 0; i < fh; i++)
	draw_strip(s, pitch, pixsize, x1, y1+i, fw, color);

	last_modified = GetRealTime();
}

// +--------------------------------------------------------------------+

void
Bitmap::DrawEllipse(int x1, int y1, int x2, int y2, Color color, BYTE quad)
{
	BYTE* orig = GetSurface();
	BYTE* s    = orig;

	if (!s) return;

	sort(x1,x2);
	sort(y1,y2);

	int fw = x2-x1;
	int fh = y2-y1;

	if (fw < 1      || fh < 1) return;

	// clip:
	if (x1 >= width  || x2 < 0) return;
	if (y1 >= height || y2 < 0) return;

	double a  = fw / 2.0;
	double b  = fh / 2.0;
	double a2 = a*a;
	double b2 = b*b;

	int x = 0;
	int y = (int) b;
	int x0 = (x1+x2)/2;
	int y0 = (y1+y2)/2;

	// clip super-giant ellipses:
	if (x1 < 0 && y1 < 0 && x2 > width && y2 > height) {
		double r2 = (a2<b2) ? a2 : b2;
		
		if (r2 > 32 * height)
		return;
		
		double ul = (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0);
		double ur = (x2-x0)*(x2-x0) + (y1-y0)*(y1-y0);
		double ll = (x1-x0)*(x1-x0) + (y2-y0)*(y2-y0);
		double lr = (x2-x0)*(x2-x0) + (y2-y0)*(y2-y0);
		
		if (ul > r2 && ur > r2 && ll > r2 && lr > r2)
		return;
	}

	DrawEllipsePoints(x0,y0,x,y,color,quad);

	// region 1
	double d1 = (b2)-(a2*b)+(0.25*a2);
	while ((a2)*(y-0.5) > (b2)*(x+1)) {
		if (d1 < 0)
		d1 += b2*(2*x+3);
		else {
			d1 += b2*(2*x+3) + a2*(-2*y+2);
			y--;
		}
		x++;

		DrawEllipsePoints(x0,y0,x,y,color,quad);
	}

	// region 2
	double d2 = b2*(x+0.5)*(x+0.5) + a2*(y-1)*(y-1) - a2*b2;
	while (y > 0) {
		if (d2 < 0) {
			d2 += b2*(2*x+2) + a2*(-2*y+3);
			x++;
		}
		else
		d2 += a2*(-2*y+3);
		y--;

		DrawEllipsePoints(x0,y0,x,y,color,quad);
	}

	last_modified = GetRealTime();
}

void
Bitmap::DrawEllipsePoints(int x0, int y0, int x, int y, Color c, BYTE quad)
{
	BYTE* s = GetSurface();

	if (!s) return;

	int   pitch = Pitch();
	int   pixsize = PixSize();

	int   left   = x0-x;
	int   right  = x0+x+1;
	int   top    = y0-y;
	int   bottom = y0+y+1;

	// clip:
	if (left >= width  || right  < 0) return;
	if (top  >= height || bottom < 0) return;

	BYTE* dst = 0;
	DWORD cf  = c.Value();

	if (left >= 0 && top >= 0 && quad&1) {
		dst = s + top*pitch + left*pixsize;
		
		switch (pixsize) {
		case 1: *dst = (BYTE) cf; break;
		case 2: { LPWORD  sw = (LPWORD)  dst; *sw = (WORD)  cf; } break;
		case 4: { LPDWORD sd = (LPDWORD) dst; *sd = (DWORD) cf; } break;
		}
	}

	if (right < width && top >= 0 && quad&2) {
		dst = s + top*pitch + right*pixsize;
		
		switch (pixsize) {
		case 1: *dst = (BYTE) cf; break;
		case 2: { LPWORD  sw = (LPWORD)  dst; *sw = (WORD)  cf; } break;
		case 4: { LPDWORD sd = (LPDWORD) dst; *sd = (DWORD) cf; } break;
		}
	}

	if (left >= 0 && bottom < height && quad&4) {
		dst = s + bottom*pitch + left*pixsize;
		
		switch (pixsize) {
		case 1: *dst = (BYTE) cf; break;
		case 2: { LPWORD  sw = (LPWORD)  dst; *sw = (WORD)  cf; } break;
		case 4: { LPDWORD sd = (LPDWORD) dst; *sd = (DWORD) cf; } break;
		}
	}

	if (right < width && bottom < height && quad&4) {
		dst = s + bottom*pitch + right*pixsize;
		
		switch (pixsize) {
		case 1: *dst = (BYTE) cf; break;
		case 2: { LPWORD  sw = (LPWORD)  dst; *sw = (WORD)  cf; } break;
		case 4: { LPDWORD sd = (LPDWORD) dst; *sd = (DWORD) cf; } break;
		}
	}
}

// +--------------------------------------------------------------------+

static void draw_strip(BYTE* s, int pitch, int pixsize, int x, int y, int len, Color color)
{
	if (!s) return;
	s += y*pitch + x*pixsize;

	DWORD value = color.Formatted();

	switch (pixsize) {
	case 1: {
			if (len > 1)
			memset(s, (BYTE) value, len);
		}
		break;

	case 2: {
			LPWORD  sw = (LPWORD) s;
			for (int x = 0; x < len; x++) {
				*sw++ = (WORD) value;
			}
		}
		break;

	case 4: {
			Color* sd = (Color*) s;
			for (int x = 0; x < len; x++) {
				*sd++ = color;
			}
		}
		break;
	}
}

// +--------------------------------------------------------------------+

static void draw_vline(BYTE* s, int pitch, int pixsize, int x, int y, int len, Color color)
{
	if (!s) return;
	s += (y)*pitch + (x)*pixsize;

	DWORD value = color.Formatted();

	switch (pixsize) {
	case 1: {
			for (int y = 0; y < len; y++) {
				*s = (BYTE) value;
				s += pitch;
			}
		}
		break;

	case 2: {
			LPWORD  sw = (LPWORD) s;
			pitch /= 2;

			for (int y = 0; y < len; y++) {
				*sw = (WORD) value;
				sw += pitch;
			}
		}
		break;

	case 4: {
			Color* sd = (Color*) s;
			pitch /= 4;

			for (int y = 0; y < len; y++) {
				*sd = color;
				sd += pitch;
			}
		}
		break;
	}
}


