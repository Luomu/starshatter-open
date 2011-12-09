/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Color.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Universal Color Format class
*/

#include "MemDebug.h"
#include "Color.h"
#include "Video.h"
#include "PCX.h"
#include "Fix.h"

// +--------------------------------------------------------------------+

void Print(const char* fmt, ...);

Color          Color::White      = Color(255,255,255);
Color          Color::Black      = Color(  0,  0,  0);
Color          Color::Gray       = Color(128,128,128);
Color          Color::LightGray  = Color(192,192,192);
Color          Color::DarkGray   = Color( 64, 64, 64);
Color          Color::BrightRed  = Color(255,  0,  0);
Color          Color::BrightBlue = Color(  0,  0,255);
Color          Color::BrightGreen= Color(  0,255,  0);
Color          Color::DarkRed    = Color(128,  0,  0);
Color          Color::DarkBlue   = Color(  0,  0,128);
Color          Color::DarkGreen  = Color(  0,128,  0);
Color          Color::Yellow     = Color(255,255,  0);
Color          Color::Cyan       = Color(  0,255,255);
Color          Color::Magenta    = Color(255,  0,255);
Color          Color::Tan        = Color(180,150,120);
Color          Color::Brown      = Color(128,100, 80);
Color          Color::Violet     = Color(128,  0,128);
Color          Color::Orange     = Color(255,150, 20);

bool           Color::standard_format = false;
int            Color::texture_alpha_level = 0;
ColorFormat    Color::format     = ColorFormat(256);
ColorFormat    Color::texture_format[4]  = { ColorFormat(256), ColorFormat(256), ColorFormat(256), ColorFormat(256) };
PALETTEENTRY   Color::palette[256];
BYTE           Color::table[32768];
double         Color::fade = 1.0;
Color          Color::fade_color;
Video*         Color::video = 0;
DWORD          ColorIndex::texture_palettes[4][256];
DWORD          ColorIndex::unfaded_palette[256];
DWORD          ColorIndex::formatted_palette[256];
DWORD          ColorIndex::shade_table[256*256];
BYTE           ColorIndex::blend_table[256*256];
DWORD*         ColorIndex::texture_palette = ColorIndex::texture_palettes[0];

// +--------------------------------------------------------------------+

Color::Color(BYTE index)
{
	PALETTEENTRY* p = &palette[index];

	Set(p->peRed, p->peGreen, p->peBlue);
}

// +--------------------------------------------------------------------+

Color&
Color::operator+=(const Color& c)
{
	int  r = Red()   + c.Red();     if (r > 255) r = 255;
	int  g = Green() + c.Green();   if (g > 255) g = 255;
	int  b = Blue()  + c.Blue();    if (b > 255) b = 255;

	Set((BYTE) r, (BYTE) g, (BYTE) b);
	return *this;
}

Color
Color::operator+(DWORD d) const
{
	int r = Red()   + ((d & RMask) >> RShift);   if (r > 255) r = 255;
	int g = Green() + ((d & GMask) >> GShift);   if (g > 255) g = 255;
	int b = Blue()  + ((d & BMask) >> BShift);   if (b > 255) b = 255;

	return Color((BYTE) r,(BYTE) g,(BYTE) b);
}

// +--------------------------------------------------------------------+

Color
Color::operator+(const Color& c) const
{
	float src_alpha = c.fAlpha();
	float dst_alpha = 1.0f - src_alpha;

	BYTE  r = (BYTE)((fRed()  *dst_alpha + c.fRed()  *src_alpha)*255.0f);
	BYTE  g = (BYTE)((fGreen()*dst_alpha + c.fGreen()*src_alpha)*255.0f);
	BYTE  b = (BYTE)((fBlue() *dst_alpha + c.fBlue() *src_alpha)*255.0f);

	return Color(r, g, b);
}

// +--------------------------------------------------------------------+

Color
Color::operator*(const Color& c) const
{
	BYTE  r = (BYTE) ((fRed()   * c.fRed())   *255.0f);
	BYTE  g = (BYTE) ((fGreen() * c.fGreen()) *255.0f);
	BYTE  b = (BYTE) ((fBlue()  * c.fBlue())  *255.0f);

	return Color(r, g, b);
}

// +--------------------------------------------------------------------+

Color
Color::operator*(double scale) const
{
	int r = fast_f2i(Red()   * scale); if (r > 255) r = 255;
	int g = fast_f2i(Green() * scale); if (g > 255) g = 255;
	int b = fast_f2i(Blue()  * scale); if (b > 255) b = 255;
	int a = fast_f2i(Alpha() * scale); if (a > 255) a = 255;

	return Color((BYTE) r, (BYTE) g, (BYTE) b, (BYTE) a);
}

Color
Color::dim(double scale) const
{
	int r = fast_f2i(Red()   * scale);
	int g = fast_f2i(Green() * scale);
	int b = fast_f2i(Blue()  * scale);

	return Color((BYTE) r, (BYTE) g, (BYTE) b, (BYTE) Alpha());
}

// +--------------------------------------------------------------------+

DWORD
Color::Formatted() const
{
	if (format.pal) {
		return Index();
	}

	else {
		if (fade != 1.0) {
			double step = (1.0 - fade);

			DWORD r = ((int) ((fRed()   - (fRed()   - fade_color.fRed())  * step)*255.0)) >> format.rdown;
			DWORD g = ((int) ((fGreen() - (fGreen() - fade_color.fGreen())* step)*255.0)) >> format.gdown;
			DWORD b = ((int) ((fBlue()  - (fBlue()  - fade_color.fBlue()) * step)*255.0)) >> format.bdown;
			DWORD a = Alpha()>>format.adown;

			return (r<<format.rshift)|(g<<format.gshift)|(b<<format.bshift)|(a<<format.ashift);

		}

		else if (standard_format) {
			return rgba;
		}

		else {
			DWORD r = Red()  >>format.rdown;
			DWORD g = Green()>>format.gdown;
			DWORD b = Blue() >>format.bdown;
			DWORD a = Alpha()>>format.adown;
			
			return (r<<format.rshift)|(g<<format.gshift)|(b<<format.bshift)|(a<<format.ashift);
		}
	}
}

// +--------------------------------------------------------------------+

Color
Color::Faded() const
{
	if (fade != 1.0) {
		double step = (1.0 - fade);

		DWORD r = ((int) ((fRed()   - (fRed()   - fade_color.fRed())  * step)*255.0));
		DWORD g = ((int) ((fGreen() - (fGreen() - fade_color.fGreen())* step)*255.0));
		DWORD b = ((int) ((fBlue()  - (fBlue()  - fade_color.fBlue()) * step)*255.0));
		DWORD a = Alpha();

		return Color((BYTE) r, (BYTE) g, (BYTE) b, (BYTE) a);

	}

	else {
		return *this;
	}
}

// +--------------------------------------------------------------------+

DWORD
Color::Unfaded() const
{
	if (standard_format) {
		return rgba;
	}

	if (format.pal) {
		return Index();
	}
	else {
		DWORD r = Red()  >>format.rdown;
		DWORD g = Green()>>format.gdown;
		DWORD b = Blue() >>format.bdown;
		DWORD a = Alpha()>>format.adown;
		
		return (r<<format.rshift)|(g<<format.gshift)|(b<<format.bshift)|(a<<format.ashift);
	}
}

// +--------------------------------------------------------------------+

DWORD
Color::TextureFormat(int keep_alpha) const
{
	if (texture_format[texture_alpha_level].pal) {
		return Index();
	}
	else if (rgba == 0) {
		return 0;
	}
	else {
		ColorFormat& tf = texture_format[texture_alpha_level];
		
		DWORD r = Red();
		DWORD g = Green();
		DWORD b = Blue();
		DWORD a = 0;
		
		if (keep_alpha) {
			a = Alpha()>>tf.adown;
		}

		else if (texture_alpha_level == 1) {
			// transparent:
			a = 255>>tf.adown;
		}
		
		else if (texture_alpha_level == 2) {
			// translucent:
			if (r || g || b)
			a = ((r+g+b+255)>>2)>>tf.adown;
		}

		r = r >>tf.rdown;
		g = g >>tf.gdown;
		b = b >>tf.bdown;

		return (r<<tf.rshift)|(g<<tf.gshift)|(b<<tf.bshift)|(a<<tf.ashift);
	}
}

// +--------------------------------------------------------------------+

Color
Color::ShadeColor(int shade) const
{
	double   fr = fRed(),      sr = fr;
	double   fg = fGreen(),    sg = fg;
	double   fb = fBlue(),     sb = fb;
	double   range = SHADE_LEVELS;

	// first shade:
	if (shade < SHADE_LEVELS) {          // shade towards black
		sr = fr * (shade/range);
		sg = fg * (shade/range);
		sb = fb * (shade/range);
	}
	else if (shade > SHADE_LEVELS) {     // shade towards white
		double step = (shade - range)/range;
		
		sr = fr - (fr - 1.0) * step;
		sg = fg - (fg - 1.0) * step;
		sb = fb - (fb - 1.0) * step;
	}

	return Color((BYTE) (sr*255.0), (BYTE) (sg*255.0), (BYTE) (sb*255.0), (BYTE) Alpha());
}

// +--------------------------------------------------------------------+

DWORD
Color::Shaded(int shade) const
{
	return ShadeColor(shade).Formatted();
}

// +--------------------------------------------------------------------+

Color
Color::Unformat(DWORD formatted_color)
{
	if (format.pal) {
		return Color((BYTE) formatted_color);
	}
	else if (standard_format) {
		Color c;
		c.Set(formatted_color);
		return c;
	}
	else {
		BYTE r = (BYTE) ((formatted_color & format.rmask)>>format.rshift) << format.rdown;
		BYTE g = (BYTE) ((formatted_color & format.gmask)>>format.gshift) << format.gdown;
		BYTE b = (BYTE) ((formatted_color & format.bmask)>>format.bshift) << format.bdown;
		BYTE a = (BYTE) ((formatted_color & format.amask)>>format.ashift) << format.adown;
		
		return Color(r,g,b,a);
	}
}

// +--------------------------------------------------------------------+

Color
Color::Scale(const Color& c1, const Color& c2, double scale)
{
	BYTE r = (BYTE) ((c1.fRed()   + (c2.fRed()   - c1.fRed()  )*scale) * 255);
	BYTE g = (BYTE) ((c1.fGreen() + (c2.fGreen() - c1.fGreen())*scale) * 255);
	BYTE b = (BYTE) ((c1.fBlue()  + (c2.fBlue()  - c1.fBlue() )*scale) * 255);
	BYTE a = (BYTE) ((c1.fAlpha() + (c2.fAlpha() - c1.fAlpha())*scale) * 255);

	return Color(r,g,b,a);
}

// +--------------------------------------------------------------------+

DWORD
Color::FormattedBlend(DWORD c1, DWORD c2)
{
	if (format.pal) {
		return ColorIndex::blend_table[(BYTE) c1 * 256 + (BYTE) c2];
	}
	else {
		ColorFormat& cf = format;

		DWORD r = (c1 & cf.rmask) + (c2 & cf.rmask);
		DWORD g = (c1 & cf.gmask) + (c2 & cf.gmask);
		DWORD b = (c1 & cf.bmask) + (c2 & cf.bmask);

		if (r & ~cf.rmask) r = cf.rmask;
		if (g & ~cf.gmask) g = cf.gmask;
		if (b & ~cf.bmask) b = cf.bmask;

		return (DWORD) (r|g|b);
	}
}

// +--------------------------------------------------------------------+

void
Color::UseVideo(Video* v)
{
	video = v;
}

// +--------------------------------------------------------------------+

void
Color::UseFormat(const ColorFormat& cf)
{
	format = cf;

	if (format.rmask == RMask && format.gmask == GMask && format.bmask == BMask)
	standard_format = true;
	else
	standard_format = false;

	if (cf.pal) {
		for (int i = 0; i < 256; i++) {
			ColorIndex::formatted_palette[i]   = i;
			ColorIndex::unfaded_palette[i]     = i;
		}
	}
	else {
		double old_fade = fade;

		for (int i = 0; i < 256; i++) {
			ColorIndex::formatted_palette[i] = Color(i).Formatted();

			fade = 1.0;
			ColorIndex::unfaded_palette[i]   = Color(i).Formatted();
			fade = old_fade;
		}
	}
}

// +--------------------------------------------------------------------+

void
Color::UseTextureFormat(const ColorFormat& cf, int alpha_level)
{
	texture_format[alpha_level] = cf;

	if (cf.pal) {
		for (int i = 0; i < 256; i++) {
			ColorIndex::texture_palettes[alpha_level][i] = i;
		}
	}
	else {
		double old_fade = fade;

		for (int i = 0; i < 256; i++) {
			int old_texture_alpha_level = texture_alpha_level;
			texture_alpha_level = alpha_level;
			ColorIndex::texture_palettes[alpha_level][i] = Color(i).TextureFormat();
			texture_alpha_level = old_texture_alpha_level;
		}
	}
}

// +--------------------------------------------------------------------+

void
Color::WithTextureFormat(int alpha_level)
{
	texture_alpha_level = alpha_level;
	ColorIndex::texture_palette = ColorIndex::texture_palettes[alpha_level];
}

// +--------------------------------------------------------------------+

static BYTE MatchRGB(PALETTEENTRY* pal, BYTE r, BYTE g, BYTE b)
{
	double mindist = 100000000.0;
	BYTE   match   = 0;

	for (int i = 0; i < 256; i++) {
		PALETTEENTRY* p = pal++;

		double dr = p->peRed    - r;
		double dg = p->peGreen  - g;
		double db = p->peBlue   - b;
		
		double d = (dr*dr) + (dg*dg) + (db*db);

		if (d < mindist) {
			mindist = d;
			match   = i;
			
			if (d < 1.0)
			return match;
		}
	}

	return match;
}

// +--------------------------------------------------------------------+

void
Color::SetPalette(PALETTEENTRY* pal, int palsize, BYTE* invpal)
{
	for (int i = 0; i < palsize; i++)
	palette[i] = pal[i];

	if (invpal) {
		for (int i = 0; i < 32768; i++)
		table[i] = invpal[i];
	}
	else {
		for (int i = 0; i < 32768; i++) {
			BYTE r = (i >> 10) & 0x1f;
			BYTE g = (i >>  5) & 0x1f;
			BYTE b = (i      ) & 0x1f;
			
			Color c(r<<3, g<<3, b<<3);
			
			table[i] = MatchRGB(palette, r<<3, g<<3, b<<3);
		}
	}

	// set up formatted palette:
	UseFormat(format);

	for (int i = 0; i < 4; i++)
	UseTextureFormat(texture_format[i], i);

	// set up shade table:
	double old_fade = fade;
	fade = 1.0;
	BuildShadeTable();
	fade = old_fade;

	// and blend table:
	BuildBlendTable();
}

// +--------------------------------------------------------------------+

void
Color::SavePalette(const char* basename)
{
	char  filename[256];

	sprintf_s(filename, "%s.ipl", basename);
	FILE* f;
	fopen_s(&f, filename, "wb");
	if (f) {
		fwrite(table, sizeof(table), 1, f);
		fclose(f);
	}
}

// +--------------------------------------------------------------------+

void
Color::SetFade(double f, Color c, int build_shade)
{
	static int shade_built = 0;

	if (fade == f && fade_color == c && (build_shade ? shade_built : 1))
	return;

	fade       = f;
	fade_color = c;

	// set up formatted palette:
	UseFormat(format);

	// if this is a paletted video mode,
	// modify the video palette as well:
	if (format.pal && video) {
		PALETTEENTRY fade_palette[256];
		
		double step = (1.0 - fade);
		for (int i = 0; i < 256; i++) {
			PALETTEENTRY& entry = fade_palette[i];
			ColorIndex    c     = ColorIndex(i);

			entry.peRed   = ((int) ((c.fRed()   - (c.fRed()   - fade_color.fRed())  * step)*255.0));
			entry.peGreen = ((int) ((c.fGreen() - (c.fGreen() - fade_color.fGreen())* step)*255.0));
			entry.peBlue  = ((int) ((c.fBlue()  - (c.fBlue()  - fade_color.fBlue()) * step)*255.0));
			entry.peFlags = 0;
		}
	}

	// otherwise, we need to re-compute 
	// the shade table:
	else {
		if (build_shade) {
			BuildShadeTable();
			shade_built = 1;
		}
		else {
			shade_built = 0;
		}
	}
}

// +--------------------------------------------------------------------+

void
Color::BuildShadeTable()
{
	for (int shade = 0; shade < SHADE_LEVELS*2; shade++)
	for (int index = 0; index < 256; index++)
	ColorIndex::shade_table[shade*256+index] = Color(index).Shaded(shade);
}

// +--------------------------------------------------------------------+

void
Color::BuildBlendTable()
{
	for (int src = 0; src < 256; src++) {
		for (int dst = 0; dst < 256; dst++) {
			ColorIndex src_clr = ColorIndex(src);
			ColorIndex dst_clr = ColorIndex(dst);
			
			int r = src_clr.Red()   + dst_clr.Red();
			int g = src_clr.Green() + dst_clr.Green();
			int b = src_clr.Blue()  + dst_clr.Blue();
			
			if (r>255) r=255;
			if (g>255) g=255;
			if (b>255) b=255;

			ColorIndex::blend_table[src*256+dst] = Color((BYTE)r,(BYTE)g,(BYTE)b).Index();
		}
	}
}

// +--------------------------------------------------------------------+

void
Color::SaveShadeTable(const char* basename)
{
	if (!format.pal)
	return;

	char filename[256];
	sprintf_s(filename, "%s_clut.pcx", basename);

	BYTE  clut[256*256];
	BYTE* pc = clut;
	int i;
	for (i = 0; i < SHADE_LEVELS*2; i++)
	for (int j = 0; j < 256; j++)
	*pc++ = (BYTE) ColorIndex::shade_table[i*256+j];

	for (; i < 256; i++)
	for (int j = 0; j < 256; j++)
	*pc++ = (BYTE) 0;

	PcxImage pcx(256, 256, clut, (BYTE*) palette);
	pcx.Save(filename);
}


// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+


ColorValue&
ColorValue::operator+=(const ColorValue& c)
{
	r += c.r;
	g += c.g;
	b += c.b;

	return *this;
}

ColorValue
ColorValue::operator+(const ColorValue& c) const
{
	float src_alpha = c.a;
	float dst_alpha = 1.0f - a;

	float fr = (r * dst_alpha) + (c.r * src_alpha);
	float fg = (g * dst_alpha) + (c.g * src_alpha);
	float fb = (b * dst_alpha) + (c.b * src_alpha);

	return ColorValue(fr, fg, fb);
}

ColorValue
ColorValue::operator*(const ColorValue& c) const
{
	return ColorValue(r*c.r, g*c.g, b*c.b);
}

ColorValue
ColorValue::operator*(double scale) const
{
	return ColorValue((float) (r*scale),
	(float) (g*scale),
	(float) (b*scale),
	(float) (a*scale));
}

ColorValue
ColorValue::dim(double scale) const
{
	return ColorValue((float) (r*scale),
	(float) (g*scale),
	(float) (b*scale));
}

// +--------------------------------------------------------------------+

inline BYTE bclamp(float x) { return (BYTE) ((x<0) ? 0 : (x>255) ? 255 : x); }

Color
ColorValue::ToColor() const
{
	return Color(bclamp(r * 255.0f), 
	bclamp(g * 255.0f),
	bclamp(b * 255.0f),
	bclamp(a * 255.0f));
}
