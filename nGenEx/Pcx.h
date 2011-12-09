/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         PCX.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	PCX image file loader
*/

#ifndef PCX_H
#define PCX_H

// +--------------------------------------------------------------------+

enum { PCX_OK, PCX_NOMEM, PCX_TOOBIG, PCX_NOFILE };

struct PcxHeader
{
	char  manufacturer;    // Always set to 10
	char  version;         // Always 5 for 256-color files
	char  encoding;        // Always set to 1
	char  bits_per_pixel;  // Should be 8 for 256-color files
	short xmin,ymin;       // Coordinates for top left corner
	short xmax,ymax;       // Width and height of image
	short hres;            // Horizontal resolution of image
	short vres;            // Vertical resolution of image
	char  palette16[48];   // EGA palette; not used for 256-color files
	char  reserved;        // Reserved for future use
	char  color_planes;    // Color planes
	short bytes_per_line;  // Number of bytes in 1 line of pixels
	short palette_type;    // Should be 2 for color palette
	char  filler[58];      // Nothing but junk
};

// +--------------------------------------------------------------------+

struct PcxImage
{
	static const char* TYPENAME() { return "PcxImage"; }

	PcxImage(short w, short h, unsigned long* hibits);
	PcxImage(short w, short h, unsigned char* bits, unsigned char* colors);

	PcxImage();
	~PcxImage();

	int Load(char *filename);
	int Save(char *filename);

	int LoadBuffer(unsigned char* buf, int len);

	PcxHeader      hdr;
	unsigned char* bitmap;
	unsigned long* himap;
	unsigned char  pal[768];
	unsigned long  imagebytes;
	unsigned short width, height;
};

// +--------------------------------------------------------------------+


#endif
