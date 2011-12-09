/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         PCX.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	PCX image file loader
*/


#include "MemDebug.h"
#include "PCX.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// +--------------------------------------------------------------------+

#define MAX_SIZE (4*1024*1024)

enum { BYTEMODE, RUNMODE };

// +--------------------------------------------------------------------+

PcxImage::PcxImage()
: width(0), height(0), bitmap(0), himap(0), imagebytes(0)
{ }

PcxImage::PcxImage(short w, short h, unsigned char* bits, unsigned char* colors)
: bitmap(0), himap(0)
{
	hdr.manufacturer = 10;   // Always set to 10
	hdr.version = 5;         // Always 5 for 256-color files
	hdr.encoding = 1;        // Always set to 1
	hdr.bits_per_pixel = 8;  // Should be 8 for 256-color files
	hdr.xmin = 0;
	hdr.xmax = w-1;
	hdr.ymin = 0;
	hdr.ymax = h-1;
	hdr.hres = 0x48;
	hdr.vres = 0x48;

	hdr.palette16[ 0] = (unsigned char) 0x00;
	hdr.palette16[ 1] = (unsigned char) 0x00;
	hdr.palette16[ 2] = (unsigned char) 0x00;
	hdr.palette16[ 3] = (unsigned char) 0x80;
	hdr.palette16[ 4] = (unsigned char) 0x00;
	hdr.palette16[ 5] = (unsigned char) 0x00;
	hdr.palette16[ 6] = (unsigned char) 0x00;
	hdr.palette16[ 7] = (unsigned char) 0x80;
	hdr.palette16[ 8] = (unsigned char) 0x00;
	hdr.palette16[ 9] = (unsigned char) 0x80;
	hdr.palette16[10] = (unsigned char) 0x80;
	hdr.palette16[11] = (unsigned char) 0x00;
	hdr.palette16[12] = (unsigned char) 0x00;
	hdr.palette16[13] = (unsigned char) 0x00;
	hdr.palette16[14] = (unsigned char) 0x80;
	hdr.palette16[15] = (unsigned char) 0x80;

	hdr.palette16[16] = (unsigned char) 0x00;
	hdr.palette16[17] = (unsigned char) 0x80;
	hdr.palette16[18] = (unsigned char) 0x00;
	hdr.palette16[19] = (unsigned char) 0x80;
	hdr.palette16[20] = (unsigned char) 0x80;
	hdr.palette16[21] = (unsigned char) 0xC0;
	hdr.palette16[22] = (unsigned char) 0xC0;
	hdr.palette16[23] = (unsigned char) 0xC0;
	hdr.palette16[24] = (unsigned char) 0xC0;
	hdr.palette16[25] = (unsigned char) 0xDC;
	hdr.palette16[26] = (unsigned char) 0xC0;
	hdr.palette16[27] = (unsigned char) 0xA6;
	hdr.palette16[28] = (unsigned char) 0xCA;
	hdr.palette16[29] = (unsigned char) 0xF0;
	hdr.palette16[30] = (unsigned char) 0x33;
	hdr.palette16[31] = (unsigned char) 0x2B;

	hdr.palette16[32] = (unsigned char) 0x1F;
	hdr.palette16[33] = (unsigned char) 0x2B;
	hdr.palette16[34] = (unsigned char) 0x23;
	hdr.palette16[35] = (unsigned char) 0x1B;
	hdr.palette16[36] = (unsigned char) 0x5F;
	hdr.palette16[37] = (unsigned char) 0x5F;
	hdr.palette16[38] = (unsigned char) 0x5F;
	hdr.palette16[39] = (unsigned char) 0x2F;
	hdr.palette16[40] = (unsigned char) 0x2F;
	hdr.palette16[41] = (unsigned char) 0x2F;
	hdr.palette16[42] = (unsigned char) 0x27;
	hdr.palette16[43] = (unsigned char) 0x27;
	hdr.palette16[44] = (unsigned char) 0x27;
	hdr.palette16[45] = (unsigned char) 0x1F;
	hdr.palette16[46] = (unsigned char) 0x1F;
	hdr.palette16[47] = (unsigned char) 0x1F;

	hdr.reserved = 0;        // Reserved for future use
	hdr.color_planes = 1;    // Color planes
	hdr.bytes_per_line = w;  // Number of bytes in 1 line of pixels
	hdr.palette_type = 1;    // Should be 1 for color palette

	for (unsigned int i = 0; i < 58; i++)
	hdr.filler[i] = 0;

	width      = w;
	height     = h;
	imagebytes = width * height;

	bitmap = new(__FILE__,__LINE__) unsigned char [imagebytes];

	if (bitmap) {
		for (unsigned long i = 0; i < imagebytes; i++)
		bitmap[i] = bits[i];

		unsigned char* p = pal;
		for (int i = 0; i < 256; i++) {   
			*p++ = *colors++;
			*p++ = *colors++;
			*p++ = *colors++;
			colors++;
		}
	}
}

PcxImage::PcxImage(short w, short h, unsigned long* hibits)
: bitmap(0), himap(0)
{
	hdr.manufacturer = 10;   // Always set to 10
	hdr.version = 5;         // Always 5 for true color files
	hdr.encoding = 1;        // Always set to 1
	hdr.bits_per_pixel = 8;  // Should be 8 for true color files
	hdr.xmin = 0;
	hdr.xmax = w-1;
	hdr.ymin = 0;
	hdr.ymax = h-1;
	hdr.hres = 0x48;
	hdr.vres = 0x48;

	hdr.palette16[ 0] = (unsigned char) 0x00;
	hdr.palette16[ 1] = (unsigned char) 0x00;
	hdr.palette16[ 2] = (unsigned char) 0x00;
	hdr.palette16[ 3] = (unsigned char) 0x80;
	hdr.palette16[ 4] = (unsigned char) 0x00;
	hdr.palette16[ 5] = (unsigned char) 0x00;
	hdr.palette16[ 6] = (unsigned char) 0x00;
	hdr.palette16[ 7] = (unsigned char) 0x80;
	hdr.palette16[ 8] = (unsigned char) 0x00;
	hdr.palette16[ 9] = (unsigned char) 0x80;
	hdr.palette16[10] = (unsigned char) 0x80;
	hdr.palette16[11] = (unsigned char) 0x00;
	hdr.palette16[12] = (unsigned char) 0x00;
	hdr.palette16[13] = (unsigned char) 0x00;
	hdr.palette16[14] = (unsigned char) 0x80;
	hdr.palette16[15] = (unsigned char) 0x80;

	hdr.palette16[16] = (unsigned char) 0x00;
	hdr.palette16[17] = (unsigned char) 0x80;
	hdr.palette16[18] = (unsigned char) 0x00;
	hdr.palette16[19] = (unsigned char) 0x80;
	hdr.palette16[20] = (unsigned char) 0x80;
	hdr.palette16[21] = (unsigned char) 0xC0;
	hdr.palette16[22] = (unsigned char) 0xC0;
	hdr.palette16[23] = (unsigned char) 0xC0;
	hdr.palette16[24] = (unsigned char) 0xC0;
	hdr.palette16[25] = (unsigned char) 0xDC;
	hdr.palette16[26] = (unsigned char) 0xC0;
	hdr.palette16[27] = (unsigned char) 0xA6;
	hdr.palette16[28] = (unsigned char) 0xCA;
	hdr.palette16[29] = (unsigned char) 0xF0;
	hdr.palette16[30] = (unsigned char) 0x33;
	hdr.palette16[31] = (unsigned char) 0x2B;

	hdr.palette16[32] = (unsigned char) 0x1F;
	hdr.palette16[33] = (unsigned char) 0x2B;
	hdr.palette16[34] = (unsigned char) 0x23;
	hdr.palette16[35] = (unsigned char) 0x1B;
	hdr.palette16[36] = (unsigned char) 0x5F;
	hdr.palette16[37] = (unsigned char) 0x5F;
	hdr.palette16[38] = (unsigned char) 0x5F;
	hdr.palette16[39] = (unsigned char) 0x2F;
	hdr.palette16[40] = (unsigned char) 0x2F;
	hdr.palette16[41] = (unsigned char) 0x2F;
	hdr.palette16[42] = (unsigned char) 0x27;
	hdr.palette16[43] = (unsigned char) 0x27;
	hdr.palette16[44] = (unsigned char) 0x27;
	hdr.palette16[45] = (unsigned char) 0x1F;
	hdr.palette16[46] = (unsigned char) 0x1F;
	hdr.palette16[47] = (unsigned char) 0x1F;

	hdr.reserved = 0;        // Reserved for future use
	hdr.color_planes = 3;    // Color planes
	hdr.bytes_per_line = w;  // Number of bytes in 1 line of pixels
	hdr.palette_type = 1;    // Should be 1 for color palette

	for (unsigned int i = 0; i < 58; i++)
	hdr.filler[i] = 0;

	width      = w;
	height     = h;
	imagebytes = width * height;

	himap = new(__FILE__,__LINE__) unsigned long [imagebytes];

	if (himap) {
		for (unsigned long i = 0; i < imagebytes; i++)
		himap[i] = hibits[i];
	}
}

PcxImage::~PcxImage()
{
	delete [] bitmap;
	delete [] himap;
}

// +--------------------------------------------------------------------+

int PcxImage::Load(char *filename)
{
	unsigned long i;
	short  mode=BYTEMODE, bytecount;
	unsigned char abyte, *p;
	FILE *f;

	fopen_s(&f, filename,"rb");
	if (f == NULL)
	return PCX_NOFILE;

	fread(&hdr, sizeof(PcxHeader), 1, f);

	// read 256 color PCX file
	if (hdr.color_planes == 1) {
		width      = 1 + hdr.xmax - hdr.xmin;
		height     = 1 + hdr.ymax - hdr.ymin;
		imagebytes = width * height;

		if (imagebytes > MAX_SIZE)
		return PCX_TOOBIG;

		// get palette from pcx file
		fseek(f,-768L,SEEK_END);
		fread(pal,768,1,f);

		// now go back and read the pixel data:
		fseek(f,sizeof(PcxHeader),SEEK_SET);

		delete [] himap;  himap = 0;
		delete [] bitmap; bitmap = 0;

		himap = new(__FILE__,__LINE__) unsigned long [imagebytes];
		if (himap == NULL)
		return PCX_NOMEM;

		// force alpha channel to 255
		memset(himap, 0xff, imagebytes*4);

		unsigned long* pix = himap;
		for (i=0; i<imagebytes; i++) {
			if (mode == BYTEMODE) {
				abyte = fgetc(f);

				if (abyte > 0xbf) {
					bytecount = abyte & 0x3f;
					abyte = fgetc(f);
					if (--bytecount > 0)
					mode = RUNMODE;
				}
			}
			else if (--bytecount == 0) {
				mode = BYTEMODE;
			}

			*pix++ = 0xff000000 | (pal[3*abyte] << 16) | (pal[3*abyte+1] << 8) | (pal[3*abyte+2]);
		}
	}

	// read 24-bit (true COLOR) PCX file
	else {
		width      = 1 + hdr.xmax - hdr.xmin;
		height     = 1 + hdr.ymax - hdr.ymin;
		imagebytes = width * height;

		if (imagebytes > MAX_SIZE)
		return PCX_TOOBIG;

		delete [] himap;  himap = 0;
		delete [] bitmap; bitmap = 0;

		himap = new(__FILE__,__LINE__) unsigned long [imagebytes];
		if (himap == NULL)
		return PCX_NOMEM;

		// force alpha channel to 255
		memset(himap, 0xff, imagebytes*4);

		for (int row = 0; row < height; row++) {
			// RED, GREEN, BLUE
			for (int plane = 2; plane >= 0; plane--) {
				p = ((unsigned char*) himap) + width*row*4 + plane;
				for (int col = 0; col < width; col++) {
					if (mode == BYTEMODE) {
						abyte = fgetc(f);

						if (abyte > 0xbf) {
							bytecount = abyte & 0x3f;
							abyte = fgetc(f);
							if (--bytecount > 0)
							mode = RUNMODE;
						}
					}
					else if (--bytecount == 0) {
						mode = BYTEMODE;
					}

					*p = abyte;
					p += 4;
				}
			}
		}
	}

	fclose(f);
	return PCX_OK;
}

// +--------------------------------------------------------------------+

int PcxImage::LoadBuffer(unsigned char* buf, int len)
{
	unsigned long i;
	short  mode=BYTEMODE, bytecount;
	unsigned char abyte, *p;
	unsigned char* fp;

	if (buf == NULL)
	return PCX_NOFILE;

	fp = buf;
	memcpy(&hdr, buf, sizeof(PcxHeader));
	fp += sizeof(PcxHeader);

	// read 256 color PCX file
	if (hdr.color_planes == 1) {
		width      = 1 + hdr.xmax - hdr.xmin;
		height     = 1 + hdr.ymax - hdr.ymin;
		imagebytes = width * height;

		if (imagebytes > MAX_SIZE)
		return PCX_TOOBIG;

		// get palette from end of pcx file
		memcpy(pal,buf+len-768,768);

		delete [] himap;  himap = 0;
		delete [] bitmap; bitmap = 0;

		himap = new(__FILE__,__LINE__) unsigned long [imagebytes];
		if (himap == NULL)
		return PCX_NOMEM;

		memset(himap, 0, imagebytes*4);

		unsigned long* pix = himap;
		for (i=0; i<imagebytes; i++) {
			if (mode == BYTEMODE) {
				abyte = *fp++;

				if (abyte > 0xbf) {
					bytecount = abyte & 0x3f;
					abyte = *fp++;
					if (--bytecount > 0)
					mode = RUNMODE;
				}
			}
			else if (--bytecount == 0) {
				mode = BYTEMODE;
			}

			*pix++ = 0xff000000 | (pal[3*abyte] << 16) | (pal[3*abyte+1] << 8) | (pal[3*abyte+2]);
		}
	}

	// read 24-bit (true COLOR) PCX file
	else {
		width      = 1 + hdr.xmax - hdr.xmin;
		height     = 1 + hdr.ymax - hdr.ymin;
		imagebytes = width * height;

		if (imagebytes > MAX_SIZE)
		return PCX_TOOBIG;

		delete [] himap;  himap = 0;
		delete [] bitmap; bitmap = 0;

		himap = new(__FILE__,__LINE__) unsigned long [imagebytes];
		if (himap == NULL)
		return PCX_NOMEM;

		memset(himap, 0, imagebytes*4);

		for (int row = 0; row < height; row++) {
			// RED, GREEN, BLUE
			for (int plane = 2; plane >= 0; plane--) {
				p = ((unsigned char*) himap) + width*row*4 + plane;
				for (int col = 0; col < width; col++) {
					if (mode == BYTEMODE) {
						abyte = *fp++;

						if (abyte > 0xbf) {
							bytecount = abyte & 0x3f;
							abyte = *fp++;
							if (--bytecount > 0)
							mode = RUNMODE;
						}
					}
					else if (--bytecount == 0) {
						mode = BYTEMODE;
					}

					*p = abyte;
					p += 4;
				}
			}
		}
	}

	return PCX_OK;
}

// +--------------------------------------------------------------------+

int PcxImage::Save(char *filename)
{
	short  mode=BYTEMODE;
	FILE *f;

	fopen_s(&f, filename,"wb");
	if (f == NULL)
	return PCX_NOFILE;

	fwrite(&hdr, sizeof(PcxHeader), 1, f);

	if (hdr.color_planes == 1) {
		unsigned char *p    = bitmap;
		unsigned long total = imagebytes;
		unsigned long row   = 0;
		unsigned char palette_marker = 12;

		while (total) {
			unsigned char* start = p;
			unsigned char  count = 0;

			while (*start == *p && count < 0x3f && row < width) {
				p++;
				count++;
				row++;
			}

			if (count > 1 || *start > 0xbf) {
				unsigned char b[2];
				b[0] = 0xc0 | count;
				b[1] = *start;
				fwrite(b, 2, 1, f);
			}
			else {
				fwrite(start, 1, 1, f);
			}

			total -= count;
			
			if (row == width)
			row = 0;
		}

		fwrite(&palette_marker,1,1,f);
		fwrite(pal,768,1,f);
	}

	// write 24-bit (TRUE COLOR) PCX file
	else {
		for (int row = 0; row < height; row++) {
			for (int plane = 2; plane >= 0; plane--) {
				unsigned long  col   = 0;
				unsigned char* p     = ((unsigned char*) himap) + width*row*4 + plane;
				
				while (col < width) {
					unsigned char* start = p;
					unsigned char  count = 0;

					while (*start == *p && count < 0x3f && col < width) {
						p += 4;
						count++;
						col++;
					}
					
					if (count > 1 || *start > 0xbf) {
						unsigned char b[2];
						b[0] = 0xc0 | count;
						b[1] = *start;
						fwrite(b, 2, 1, f);
					}
					else {
						fwrite(start, 1, 1, f);
					}
				}
			}
		}
	}

	fclose(f);
	return PCX_OK;                // return success
}

