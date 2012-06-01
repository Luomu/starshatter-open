/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Font.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Font Resource class implementation
*/

#include "MemDebug.h"
#include "Font.h"
#include "Polygon.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "Video.h"

DWORD GetRealTime();

// +--------------------------------------------------------------------+

Font::Font()
: flags(0), height(0), baseline(0), interspace(0), spacewidth(0),
imagewidth(0), image(0), expansion(0), alpha(1), blend(Video::BLEND_ALPHA),
scale(1), material(0), vset(0), polys(0), npolys(0),
caret_index(-1), caret_x(0), caret_y(0), tgt_bitmap(0)
{
	ZeroMemory(name,  sizeof(name));
	ZeroMemory(glyph, sizeof(glyph));
	ZeroMemory(kern,  sizeof(kern));
}

Font::Font(const char* n)
: flags(0), height(0), baseline(0), interspace(0), spacewidth(4),
imagewidth(0), image(0), expansion(0), alpha(1), blend(Video::BLEND_ALPHA),
scale(1), material(0), vset(0), polys(0), npolys(0),
caret_index(-1), caret_x(0), caret_y(0), tgt_bitmap(0)
{
	ZeroMemory(glyph, sizeof(glyph));
	ZeroMemory(kern,  sizeof(kern));
	CopyMemory(name,  n, sizeof(name));

	if (!Load(name)) {
		flags       = 0;
		height      = 0;
		baseline    = 0;
		interspace  = 0;
		spacewidth  = 0;
		imagewidth  = 0;
		image       = 0;

		ZeroMemory(glyph, sizeof(glyph));
		ZeroMemory(kern,  sizeof(kern));
	}
}

// +--------------------------------------------------------------------+

Font::~Font()
{
	if (image)     delete [] image;
	if (vset)      delete    vset;
	if (polys)     delete [] polys;
	if (material)  delete    material;
}

// +--------------------------------------------------------------------+

static char  kern_tweak[256][256];

bool
Font::Load(const char* name)
{
	if (!name || !name[0])
	return false;

	char  imgname[256];
	char  defname[256];
	wsprintf(defname, "%s.def", name);
	wsprintf(imgname, "%s.pcx", name);

	DataLoader* loader = DataLoader::GetLoader();
	if (!loader)
	return false;

	LoadDef(defname, imgname);

	for (int i = 0; i < 256; i++) {
		glyph[i].offset = GlyphOffset(i);
		glyph[i].width  = 0;
	}

	if (loader->LoadBitmap(imgname, bitmap)) {
		if (!bitmap.Pixels() && !bitmap.HiPixels())
		return false;

		scale      = bitmap.Width() / 256;
		imagewidth = bitmap.Width();
		if (height > bitmap.Height())
		height = bitmap.Height();

		int imgsize = bitmap.Width() * bitmap.Height();
		image = new(__FILE__,__LINE__) BYTE[imgsize];

		if (image) {
			if (bitmap.Pixels()) {
				CopyMemory(image, bitmap.Pixels(), imgsize);
			}

			else {
				for (int i = 0; i < imgsize; i++)
				image[i] = (BYTE) bitmap.HiPixels()[i].Alpha();
			}
		}

		material = new(__FILE__,__LINE__) Material;
		material->tex_diffuse = &bitmap;
	}
	else {
		return false;
	}

	for (int i = 0; i < 256; i++) {
		glyph[i].width = CalcWidth(i);
	}

	color = Color::White;

	if (!(flags & (FONT_FIXED_PITCH | FONT_NO_KERN)))
	AutoKern();

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			if (kern_tweak[i][j] < 100) {
				kern[i][j] = kern_tweak[i][j];
			}
		}
	}

	return true;   
}

void
Font::LoadDef(char* defname, char* imgname)
{
	for (int i = 0; i < 256; i++)
	for (int j = 0; j < 256; j++)
	kern_tweak[i][j] = 111;

	DataLoader* loader = DataLoader::GetLoader();
	if (!loader)
	return;

	BYTE* block;
	int blocklen = loader->LoadBuffer(defname, block, true);

	if (!block || blocklen < 4)
	return;

	Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
	Term*  term = parser.ParseTerm();

	if (!term) {
		Print("WARNING: could not parse '%s'\n", defname);
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "FONT") {
			Print("WARNING: invalid font def file '%s'\n", defname);
			return;
		}
	}

	do {
		delete term;

		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value().indexOf("image") == 0) {
					GetDefText(imgname, def, defname);
				}

				else if (def->name()->value() == "height") {
					int h=0;
					GetDefNumber(h, def, defname);

					if (h >= 0 && h <= 32)
					height = (BYTE) h;
				}

				else if (def->name()->value() == "baseline") {
					int b=0;
					GetDefNumber(b, def, defname);

					if (b >= 0 && b <= 32)
					baseline = (BYTE) b;
				}

				else if (def->name()->value() == "flags") {
					if (def->term()->isText()) {
						Text buf;
						GetDefText(buf, def, defname);
						buf.setSensitive(false);

						flags = 0;

						if (buf.contains("caps"))
						flags = flags | FONT_ALL_CAPS;

						if (buf.contains("kern"))
						flags = flags | FONT_NO_KERN;

						if (buf.contains("fixed"))
						flags = flags | FONT_FIXED_PITCH;
					}

					else {
						int f=0;
						GetDefNumber(f, def, defname);
						flags = (WORD) f;
					}
				}

				else if (def->name()->value() == "interspace") {
					int n=0;
					GetDefNumber(n, def, defname);

					if (n >= 0 && n <= 100)
					interspace = (BYTE) n;
				}

				else if (def->name()->value() == "spacewidth") {
					int n=0;
					GetDefNumber(n, def, defname);

					if (n >= 0 && n <= 100)
					spacewidth = (BYTE) n;
				}

				else if (def->name()->value() == "expansion") {
					GetDefNumber(expansion, def, defname);
				}

				else if (def->name()->value() == "kern") {
					TermStruct* val  = def->term()->isStruct();

					char a[8], b[8];
					int  k=111;

					a[0] = 0;
					b[0] = 0;

					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef) {
							if (pdef->name()->value() == "left" || pdef->name()->value() == "a")
							GetDefText(a, pdef, defname);

							else if (pdef->name()->value() == "right" || pdef->name()->value() == "b")
							GetDefText(b, pdef, defname);

							else if (pdef->name()->value() == "kern" || pdef->name()->value() == "k")
							GetDefNumber(k, pdef, defname);
						}
					}

					if (k < 100)
					kern_tweak[a[0]][b[0]] = k;
				}
				
				else {
					Print("WARNING: unknown object '%s' in '%s'\n",
					def->name()->value().data(), defname);
				}
			}
			else {
				Print("WARNING: term ignored in '%s'\n", defname);
				term->print();
			}
		}
	}
	while (term);

	loader->ReleaseBuffer(block);

}

// +--------------------------------------------------------------------+

static const int pipe_width  = 16;
static const int char_width  = 16;
static const int char_height = 16;
static const int row_items   = 16;
static const int row_width   = row_items * char_width;
static const int row_size    = char_height * row_width;

int
Font::GlyphOffset(BYTE c) const
{
	if (flags & FONT_ALL_CAPS)
	if (islower(c))
	c = toupper(c);

	return (c/row_items * row_size   * scale * scale + 
	c%row_items * char_width * scale);
}

int
Font::GlyphLocationX(BYTE c) const
{
	if (flags & FONT_ALL_CAPS)
	if (islower(c))
	c = toupper(c);

	return c%row_items * char_width;
}

int
Font::GlyphLocationY(BYTE c) const
{
	if (flags & FONT_ALL_CAPS)
	if (islower(c))
	c = toupper(c);

	return c/row_items * char_height;
}

// +--------------------------------------------------------------------+

int
Font::CalcWidth(BYTE c) const
{
	if (c >= PIPE_NBSP && c <= ARROW_RIGHT)
	return pipe_width;

	if (c >= 128 || !image)
	return 0;

	// all digits should be same size:
	if (isdigit(c))
	c = '0';

	int result  = 0;
	int w       = 16 * scale;
	int h       = 16 * scale;

	BYTE* src   = image + GlyphOffset(c);

	for (int y = 0; y < h; y++) {
		BYTE* pleft  = src;

		for (int x = 0; x < w; x++) {
			if (*pleft++ > 0 && x > result)
			result = x;
		}

		src += imagewidth;
	}

	return result + 2;
}

// +--------------------------------------------------------------------+

struct FontKernData
{
	double l[32];
	double r[32];
};

void
Font::FindEdges(BYTE c, double* l, double* r)
{
	if (!image)
	return;

	int w = glyph[c].width;
	int h = height;

	if (h > 32)
	h = 32;

	BYTE* src = image + GlyphOffset(c);

	for (int y = 0; y < h; y++) {
		BYTE* pleft  = src;
		BYTE* pright = src+w-1;

		*l = -1;
		*r = -1;

		for (int x = 0; x < w; x++) {
			if (*l == -1 && *pleft != 0)
			*l = x + 1 - (double) *pleft/255.0;
			if (*r == -1 && *pright != 0)
			*r = x + 1 - (double) *pright/255.0;

			pleft++;
			pright--;
		}

		src += imagewidth;
		l++;
		r++;
	}
}

static bool nokern(char c)
{
	if (c <= Font::ARROW_RIGHT)
	return true;

	const char* nokernchars = "0123456789+=<>-.,:;?'\"";

	if (strchr(nokernchars, c))
	return true;

	return false;
}

void
Font::AutoKern()
{
	FontKernData*  data = new(__FILE__,__LINE__) FontKernData[256];

	if (!data)
	return;

	int            h = height;
	if (h > 32)    h = 32;

	int i, j;

	// first, compute row edges for each glyph:

	for (i = 0; i < 256; i++) {
		ZeroMemory(&data[i], sizeof(FontKernData));

		char c = i;

		if ((flags & FONT_ALL_CAPS) && islower(c))
		c = toupper(c);

		if (glyph[(BYTE) c].width > 0) {
			FindEdges((BYTE) c, data[i].l, data[i].r);
		}
	}

	// then, compute the appropriate kern for each pair.
	// use a desired average distance of one pixel,
	// with a desired minimum distance of more than half a pixel:

	double desired_avg = 2.5 + expansion;
	double desired_min = 1;

	for (i = 0; i < 256; i++) {
		for (j = 0; j < 256; j++) {
			// no kerning between digits or dashes:
			if (nokern(i) || nokern(j)) {
				kern[i][j] = (char) 0;
			}

			else {
				double delta = 0;
				double avg   = 0;
				double min   = 2500;
				int    n     = 0;

				for (int y = 0; y < h; y++) {
					if (data[i].r[y] >= 0 && data[j].l[y] >= 0) {
						delta = data[i].r[y] + data[j].l[y];
						avg += delta;
						if (delta < min)
						min = delta;

						n++;
					}
				}

				if (n > 0) {
					avg /= n;

					delta = desired_avg - avg;

					if (delta < desired_min - min) {
						delta = ceil(desired_min - min);

						if (i == 'T' && islower(j) && !(flags & FONT_ALL_CAPS))
						delta += 1;
					}
				}
				else {
					delta = 0;
				}

				kern[i][j] = (char) delta;
			}
		}
	}

	delete [] data;
}

// +--------------------------------------------------------------------+

int
Font::CharWidth(char c) const
{
	if (flags & FONT_ALL_CAPS)
	if (islower(c))
	c = toupper(c);

	int result = 0;

	if (c >= PIPE_NBSP && c <= ARROW_RIGHT)
	result = pipe_width;

	else if (c < 0 || isspace(c))
	result = spacewidth;

	else
	result = glyph[c].width + interspace;

	return result;
}

int
Font::SpaceWidth() const
{
	return spacewidth;
}

int
Font::KernWidth(char a, char b) const
{
	if (flags & FONT_ALL_CAPS) {
		if (islower(a))   a = toupper(a);
		if (islower(b))   b = toupper(b);
	}

	return kern[a][b];
}

void
Font::SetKern(char a, char b, int k)
{
	if (k < -100 || k > 100)
	return;

	if (flags & FONT_ALL_CAPS) {
		if (islower(a))   a = toupper(a);
		if (islower(b))   b = toupper(b);
	}

	kern[a][b] = (char) k;
}

// +--------------------------------------------------------------------+

int
Font::StringWidth(const char* str, int len) const
{
	int result = 0;

	if (!str)
	return result;

	if (!len)
	len = strlen(str);

	const char* c = str;
	for (int i = 0; i < len; i++) {
		if (isspace(*c) && (*c < PIPE_NBSP || *c > ARROW_RIGHT))
		result += spacewidth;
		else {
			int cc = *c;
			if (flags & FONT_ALL_CAPS)
			if (islower(cc))
			cc = toupper(cc);

			int k  = 0;
			if (i < len-1)
			k = kern[cc][str[i+1]];

			result += glyph[cc].width + interspace + k;
		}
		c++;
	}

	return result;
}

// +--------------------------------------------------------------------+

void
Font::DrawText(const char* text, int count, Rect& text_rect, DWORD flags, Bitmap* tgt)
{
	Rect clip_rect = text_rect;

	if (clip_rect.w < 1 || clip_rect.h < 1)
	return;

	tgt_bitmap = tgt;

	if (text && text[0]) {
		if (count < 1)
		count = strlen(text);

		// single line:
		if (flags & DT_SINGLELINE) {
			DrawTextSingle(text, count, text_rect, clip_rect, flags);
		}

		// multi-line with word wrap:
		else if (flags & DT_WORDBREAK) {
			DrawTextWrap(text, count, text_rect, clip_rect, flags);
		}

		// multi-line with clip:
		else {
			DrawTextMulti(text, count, text_rect, clip_rect, flags);
		}
	}
	else {
		caret_x = text_rect.x + 2;
		caret_y = text_rect.y + 2;
	}

	// if calc only, update the rectangle:
	if (flags & DT_CALCRECT) {
		text_rect.h = clip_rect.h;
		text_rect.w = clip_rect.w;
	}

	// otherwise, draw caret if requested:
	else if (caret_index >= 0 && caret_y >= text_rect.y && caret_y <= text_rect.y + text_rect.h) {//caret_y + height < text_rect.y + text_rect.h) {
		Video* video = Video::GetInstance();

		if (video && (GetRealTime() / 500) & 1) {
			float v[4];
			v[0] = (float) (caret_x + 1);
			v[1] = (float) (caret_y);
			v[2] = (float) (caret_x + 1);
			v[3] = (float) (caret_y + height);

			video->DrawScreenLines(1, v, color, blend);
		}

		caret_index = -1;
	}

	tgt_bitmap = 0;
}

// +--------------------------------------------------------------------+

static int find_next_word_start(const char* text, int index)
{
	// step through intra-word space:
	while (text[index] && isspace(text[index]) && text[index] != '\n')
	index++;

	return index;
}

static int find_next_word_end(const char* text, int index)
{
	if (index < 0)
	return index;

	// check for leading newline:
	if (text[index] == '\n')
	return index;

	// step through intra-word space:
	while (text[index] && isspace(text[index]))
	index++;

	// step through word:
	while (text[index] && !isspace(text[index]))
	index++;

	return index-1;
}

// +--------------------------------------------------------------------+

void
Font::DrawTextSingle(const char* text, int count, const Rect& text_rect, Rect& clip_rect, DWORD flags)
{
	// parse the format flags:
	bool nodraw    = (flags & DT_CALCRECT)  ?true:false;

	int  align  = DT_LEFT;
	if (flags & DT_RIGHT)
	align = DT_RIGHT;
	else if (flags & DT_CENTER)
	align = DT_CENTER;   

	int max_width = 0;

	int valign = DT_TOP;
	if (flags & DT_BOTTOM)        valign = DT_BOTTOM;
	else if (flags & DT_VCENTER)  valign = DT_VCENTER;

	int xoffset = 0;
	int yoffset = 0;

	int length = StringWidth(text, count);
	if (length < text_rect.w) {
		switch (align) {
		default:
		case DT_LEFT:     break;
		case DT_RIGHT:    xoffset = text_rect.w - length;              break;
		case DT_CENTER:   xoffset = (text_rect.w - length)/2;          break;
		}
	}

	if (Height() < text_rect.h) {
		switch (valign) {
		default:
		case DT_TOP:      break;
		case DT_BOTTOM:   yoffset = text_rect.h - Height();      break;
		case DT_VCENTER:  yoffset = (text_rect.h - Height())/2;  break;
		}
	}

	max_width = length;

	// if calc only, update the rectangle:
	if (nodraw) {
		clip_rect.h = Height();
		clip_rect.w = max_width;
	}

	// otherwise, draw the string now:
	else {
		int x1 = text_rect.x + xoffset;
		int y1 = text_rect.y + yoffset;

		DrawString(text, count, x1, y1, text_rect);
	}

	if (caret_index >= 0 && caret_index <= count) {
		caret_x = text_rect.x + xoffset;
		caret_y = text_rect.y + yoffset;

		if (caret_index > 0)
		caret_x += StringWidth(text, caret_index);
	}

	else {
		caret_x = text_rect.x + 0;
		caret_y = text_rect.y + 0;
	}
}

// +--------------------------------------------------------------------+

void
Font::DrawTextWrap(const char* text, int count, const Rect& text_rect, Rect& clip_rect, DWORD flags)
{
	// parse the format flags:
	bool nodraw    = (flags & DT_CALCRECT)  ?true:false;

	int  align  = DT_LEFT;
	if (flags & DT_RIGHT)
	align = DT_RIGHT;
	else if (flags & DT_CENTER)
	align = DT_CENTER;   

	int nlines           = 0;
	int max_width        = 0;

	int line_start       = 0;
	int line_count       = 0;
	int count_remaining  = count;
	int curr_word_end    = -1;
	int next_word_end    = 0;
	int eol_index        = 0;

	int xoffset          = 0;
	int yoffset          = 0;

	caret_x = -1;
	caret_y = -1;

	// repeat for each line of text:
	while (count_remaining > 0) {
		int length = 0;
		
		// find the end of the last whole word that fits on the line:
		for (;;) {
			next_word_end = find_next_word_end(text, curr_word_end+1);

			if (next_word_end < 0 || next_word_end == curr_word_end)
			break;

			if (text[next_word_end] == '\n') {
				eol_index = curr_word_end = next_word_end;
				break;
			}
			
			int word_len = next_word_end - line_start + 1;

			length = StringWidth(text+line_start, word_len);
			
			if (length < text_rect.w) {
				curr_word_end = next_word_end;
				
				// check for a newline in the next block of white space:
				eol_index = 0;
				const char* eol = &text[curr_word_end+1];
				while (*eol && isspace(*eol) && *eol != '\n')
				eol++;

				if (*eol == '\n') {
					eol_index = eol - text;
					break;
				}
			}
			else
			break;
		}

		line_count = curr_word_end - line_start + 1;

		if (line_count > 0) {
			length = StringWidth(text+line_start, line_count);
		}

		// there was a single word longer than the entire line:
		else {
			line_count = next_word_end - line_start + 1;
			length = StringWidth(text+line_start, line_count);
			curr_word_end = next_word_end;
		}

		xoffset = 0;
		if (length < text_rect.w) {
			switch (align) {
			default:
			case DT_LEFT:     break;
			case DT_RIGHT:    xoffset = text_rect.w - length;              break;
			case DT_CENTER:   xoffset = (text_rect.w - length)/2;          break;
			}
		}
		
		if (length > max_width) max_width = length;

		if (eol_index > 0)
		curr_word_end = eol_index;

		int next_line_start = find_next_word_start(text, curr_word_end+1);

		if (length > 0 && !nodraw) {
			int x1 = text_rect.x + xoffset;
			int y1 = text_rect.y + yoffset;

			DrawString(text+line_start, line_count, x1, y1, text_rect);

			if (caret_index == line_start) {
				caret_x = x1 - 2;
				caret_y = y1;
			}
			else if (caret_index > line_start && caret_index < next_line_start) {
				caret_x = text_rect.x + xoffset + StringWidth(text+line_start, caret_index-line_start) - 2;
				caret_y = text_rect.y + yoffset;
			}
			else if (caret_index == count) {
				if (text[count-1] == '\n') {
					caret_x = x1 - 2;
					caret_y = y1 + height;
				}
				else {
					caret_x = text_rect.x + xoffset + StringWidth(text+line_start, caret_index-line_start) - 2;
					caret_y = text_rect.y + yoffset;
				}
			}
		}
		
		nlines++;
		yoffset += Height();
		if (eol_index > 0)
		curr_word_end = eol_index;
		line_start = find_next_word_start(text, curr_word_end+1);
		count_remaining = count - line_start;
	}

	// if calc only, update the rectangle:
	if (nodraw) {
		clip_rect.h = nlines * Height();
		clip_rect.w = max_width;
	}
}

// +--------------------------------------------------------------------+

void
Font::DrawTextMulti(const char* text, int count, const Rect& text_rect, Rect& clip_rect, DWORD flags)
{
	// parse the format flags:
	bool nodraw    = (flags & DT_CALCRECT)  ?true:false;

	int  align  = DT_LEFT;
	if (flags & DT_RIGHT)
	align = DT_RIGHT;
	else if (flags & DT_CENTER)
	align = DT_CENTER;   

	int max_width        = 0;
	int line_start       = 0;
	int count_remaining  = count;

	int xoffset = 0;
	int yoffset = 0;
	int nlines  = 0;

	// repeat for each line of text:
	while (count_remaining > 0) {
		int length     = 0;
		int line_count = 0;
		
		// find the end of line:
		while (line_count < count_remaining) {
			char c = text[line_start+line_count];
			if (!c || c == '\n')
			break;

			line_count++;
		}

		if (line_count > 0) {
			length = StringWidth(text+line_start, line_count);
		}

		xoffset = 0;
		if (length < text_rect.w) {
			switch (align) {
			default:
			case DT_LEFT:     break;
			case DT_RIGHT:    xoffset = text_rect.w - length;              break;
			case DT_CENTER:   xoffset = (text_rect.w - length)/2;          break;
			}
		}
		
		if (length > max_width) max_width = length;

		if (length && !nodraw) {
			int x1 = text_rect.x + xoffset;
			int y1 = text_rect.y + yoffset;

			DrawString(text+line_start, line_count, x1, y1, text_rect);
		}
		
		nlines++;
		yoffset += Height();

		if (line_start+line_count+1 < count) {
			line_start = find_next_word_start(text, line_start+line_count+1);
			count_remaining = count - line_start;
		}
		else {
			count_remaining = 0;
		}
	}

	// if calc only, update the rectangle:
	if (nodraw) {
		clip_rect.h = nlines * Height();
		clip_rect.w = max_width;
	}
}

// +--------------------------------------------------------------------+

int
Font::DrawString(const char* str, int len, int x1, int y1, const Rect& clip, Bitmap* tgt)
{
	Video*   video = Video::GetInstance();
	int      count = 0;
	int      maxw  = clip.w;
	int      maxh  = clip.h;

	if (len < 1 || !video)
	return count;

	// vertical clip
	if ((y1 < clip.y) || (y1 > clip.y + clip.h))
	return count;

	// RENDER TO BITMAP

	if (!tgt)
	tgt = tgt_bitmap;

	if (tgt) {
		for (int i = 0; i < len; i++) {
			char c = str[i];

			if ((flags & FONT_ALL_CAPS) && islower(c))
			c = toupper(c);
			
			int cw = glyph[c].width + interspace;
			int ch = height;
			int k  = 0;

			if (i < len-1)
			k = kern[c][str[i+1]];
			
			// horizontal clip:
			if (x1 < clip.x) {
				if (isspace(c) && (c < PIPE_NBSP || c > ARROW_RIGHT)) {
					x1   += spacewidth;
					maxw -= spacewidth;
				}
				else {
					x1   += cw+k;
					maxw -= cw+k;
				}
			}
			else if (x1+cw > clip.x+clip.w) {
				return count;
			}
			else {
				if (isspace(c) && (c < PIPE_NBSP || c > ARROW_RIGHT)) {
					x1   += spacewidth;
					maxw -= spacewidth;
				}
				else {
					int sx = GlyphLocationX(c);
					int sy = GlyphLocationY(c);

					Color* srcpix = bitmap.HiPixels();
					Color* dstpix = tgt->HiPixels();
					if (srcpix && dstpix) {
						int    spitch = bitmap.Width();
						int    dpitch = tgt->Width();

						Color* dst    = dstpix + (y1*dpitch) + x1;
						Color* src    = srcpix + (sy*spitch) + sx;

						for (int i = 0; i < ch; i++) {
							Color* ps = src;
							Color* pd = dst;

							for (int n = 0; n < cw; n++) {
								DWORD alpha = ps->Alpha();
								if (alpha) {
									*pd = color.dim(alpha / 240.0);
								}
								ps++;
								pd++;
							}

							dst += dpitch;
							src += spitch;
						}
					}
					else {
						// this probably won't work...
						tgt->BitBlt(x1, y1, bitmap, sx, sy, cw, ch, true);
					}

					x1   += cw + k;
					maxw -= cw + k;
				}
				
				count++;
			}
		}
		return count;
	}

	// RENDER TO VIDEO

	// allocate verts, if necessary
	int nverts = 4*len;
	if (!vset) {
		vset = new(__FILE__,__LINE__) VertexSet(nverts);

		if (!vset)
		return false;

		vset->space = VertexSet::SCREEN_SPACE;

		for (int v = 0; v < vset->nverts; v++) {
			vset->s_loc[v].z = 0.0f;
			vset->rw[v]      = 1.0f;
		}
	}
	else if (vset->nverts < nverts) {
		vset->Resize(nverts);

		for (int v = 0; v < vset->nverts; v++) {
			vset->s_loc[v].z = 0.0f;
			vset->rw[v]      = 1.0f;
		}
	}

	if (vset->nverts < nverts)
	return count;

	if (alpha < 1)
	color.SetAlpha((BYTE) (alpha * 255.0f));
	else
	color.SetAlpha(255);

	for (int i = 0; i < len; i++) {
		char c = str[i];

		if ((flags & FONT_ALL_CAPS) && islower(c))
		c = toupper(c);
		
		int cw = glyph[c].width + interspace;
		int k  = 0;

		if (i < len-1)
		k = kern[c][str[i+1]];
		
		// horizontal clip:
		if (x1 < clip.x) {
			if (isspace(c) && (c < PIPE_NBSP || c > ARROW_RIGHT)) {
				x1   += spacewidth;
				maxw -= spacewidth;
			}
			else {
				x1   += cw+k;
				maxw -= cw+k;
			}
		}
		else if (x1+cw > clip.x+clip.w) {
			break;
		}
		else {
			if (isspace(c) && (c < PIPE_NBSP || c > ARROW_RIGHT)) {
				x1   += spacewidth;
				maxw -= spacewidth;
			}
			else {
				// create four verts for this character:
				int    v      = count*4;
				double char_x = GlyphLocationX(c);
				double char_y = GlyphLocationY(c);
				double char_w = glyph[c].width;
				double char_h = height;

				if (y1 + char_h > clip.y + clip.h) {
					char_h = clip.y + clip.h - y1;
				}

				vset->s_loc[v+0].x = (float) (x1 - 0.5);
				vset->s_loc[v+0].y = (float) (y1 - 0.5);
				vset->tu[v+0]      = (float) (char_x / 256);
				vset->tv[v+0]      = (float) (char_y / 256);
				vset->diffuse[v+0] = color.Value();

				vset->s_loc[v+1].x = (float) (x1 + char_w - 0.5);
				vset->s_loc[v+1].y = (float) (y1 - 0.5);
				vset->tu[v+1]      = (float) (char_x / 256 + char_w / 256);
				vset->tv[v+1]      = (float) (char_y / 256);
				vset->diffuse[v+1] = color.Value();

				vset->s_loc[v+2].x = (float) (x1 + char_w - 0.5);
				vset->s_loc[v+2].y = (float) (y1 + char_h - 0.5);
				vset->tu[v+2]      = (float) (char_x / 256 + char_w / 256);
				vset->tv[v+2]      = (float) (char_y / 256 + char_h / 256);
				vset->diffuse[v+2] = color.Value();

				vset->s_loc[v+3].x = (float) (x1 - 0.5);
				vset->s_loc[v+3].y = (float) (y1 + char_h - 0.5);
				vset->tu[v+3]      = (float) (char_x / 256);
				vset->tv[v+3]      = (float) (char_y / 256 + char_h / 256);
				vset->diffuse[v+3] = color.Value();

				x1   += cw + k;
				maxw -= cw + k;

				count++;
			}
		}
	}

	if (count) {
		// this small hack is an optimization to reduce the 
		// size of vertex buffer needed for font rendering:

		int old_nverts = vset->nverts;
		vset->nverts   = 4 * count;

		// create a larger poly array, if necessary:
		if (count > npolys) {
			if (polys)
			delete [] polys;

			npolys = count;
			polys  = new(__FILE__,__LINE__) Poly[npolys];
			Poly*  p     = polys;
			int    index = 0;

			for (int i = 0; i < npolys; i++) {
				p->nverts      = 4;
				p->vertex_set  = vset;
				p->material    = material;
				p->verts[0]    = index++;
				p->verts[1]    = index++;
				p->verts[2]    = index++;
				p->verts[3]    = index++;

				p++;
			}
		}

		video->DrawScreenPolys(count, polys, blend);

		// remember to restore the proper size of the vertex set:
		vset->nverts = old_nverts;
	}

	return count;
}

