/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         VideoSettings.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Video Settings class
*/

#ifndef VideoSettings_h
#define VideoSettings_h

#include "Types.h"
#include "Video.h"

// +--------------------------------------------------------------------+

struct VideoMode
{
	enum Format {
		FMT_NONE       =  0,
		FMT_R5G5B5     = 24,
		FMT_R5G6B5     = 23,
		FMT_R8G8B8     = 20,
		FMT_X8R8G8B8   = 22
	};

	VideoMode()                                : width(0), height(0), refresh(0), format(0) { }
	VideoMode(int w, int h, Format f, int r=0) : width(w), height(h), refresh(r), format(f) { }

	int operator == (const VideoMode& m)  const { return m.width  == width  &&
		m.height == height &&
		m.format == format;   }
	int operator != (const VideoMode& m)  const { return m.width  != width  ||
		m.height != height ||
		m.format != format;   }

	const char* GetDescription() const;

	int               width;
	int               height;
	int               refresh;
	int               format;
};

// +--------------------------------------------------------------------+

struct VideoDeviceInfo
{
	VideoDeviceInfo();
	~VideoDeviceInfo();

	int               vertex_processing;
	int               depth_buffer_bits;
	int               adapter_index;
	int               device_index;
	DWORD             device_type;
	DWORD             depth_stencil_format;
	DWORD             back_buffer_format;
	DWORD             multisample_type;
	DWORD             multisample_qual;
	char              adapter_desc[128];
	char              device_desc[128];
};

// +--------------------------------------------------------------------+

class VideoSettings
{
public:
	enum VertexProcessing {
		VTX_SOFTWARE,
		VTX_MIXED,
		VTX_HARDWARE,
		VTX_PURE
	};

	VideoSettings();
	~VideoSettings();

	// accessor methods

	bool              IsWindowed()            const;
	bool              UseEffects()            const;
	int               GetWidth()              const;
	int               GetHeight()             const;
	int               GetDepth()              const;
	int               GetPixSize()            const;
	int               GetRefreshRate()        const;

	const char*       GetModeDescription()    const;

	int               GetVertexProcessing()   const;
	int               GetDepthBufferBits()    const;
	int               GetAdapterIndex()       const;
	int               GetDeviceIndex()        const;
	DWORD             GetDeviceType()         const;
	DWORD             GetDepthStencilFormat() const;
	DWORD             GetBackBufferFormat()   const;
	const char*       GetAdapterDesc()        const;
	const char*       GetDeviceDesc()         const;

	// properties

	bool              is_windowed;
	bool              use_effects;
	VideoMode         fullscreen_mode;
	VideoMode         windowed_mode;
	int               window_width;
	int               window_height;
	VideoDeviceInfo   fullscreen_device;
	VideoDeviceInfo   windowed_device;

	// feature set

	bool              shadows;
	bool              bumpmaps;
	bool              specmaps;
	int               max_detail;
	DWORD             enable_vs;
	DWORD             enable_ps;
	float             depth_bias;
};

#endif VideoSettings_h

