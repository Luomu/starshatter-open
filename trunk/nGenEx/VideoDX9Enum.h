/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         VideoDX9Enum.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Direct3D and Direct3D Video classes for DirectX 7 
*/

#ifndef VideoDX9Enum_h
#define VideoDX9Enum_h

#include <d3d9.h>

#include "Video.h"
#include "ArrayList.h"
#include "List.h"

// +--------------------------------------------------------------------+

class  VideoDX9Enum;
struct VideoDX9DisplayMode;
struct VideoDX9AdapterInfo;
struct VideoDX9DeviceInfo;
struct VideoDX9DSMSConflict;
struct VideoDX9DeviceCombo;
class  VideoSettings;

// +--------------------------------------------------------------------+

class VideoDX9Enum
{
public:
	enum VP_TYPE {
		SOFTWARE_VP,
		MIXED_VP,
		HARDWARE_VP,
		PURE_HARDWARE_VP
	};

	VideoDX9Enum(IDirect3D9* d3d9);
	~VideoDX9Enum();

	void     SetDirect3D9(IDirect3D9* d3d9);

	HRESULT  Enumerate();
	bool     SuggestWindowSettings(VideoSettings* vs);
	bool     SuggestFullscreenSettings(VideoSettings* vs);

	int                  NumAdapters()  const { return adapter_info_list.size(); }
	void                 SelectAdapter(int index);
	VideoDX9AdapterInfo* GetAdapterInfo();
	VideoDX9DeviceInfo*  GetDeviceInfo(DWORD devtype);

	bool     IsModeSupported(int width, int height, int bpp) const;

	UINT     min_width;
	UINT     min_height;
	UINT     min_color_bits;
	UINT     min_alpha_bits;
	UINT     min_depth_bits;
	UINT     min_stencil_bits;

	bool     uses_depth_buffer;
	bool     uses_mixed_vp;
	bool     req_windowed;
	bool     req_fullscreen;

private:
	HRESULT  EnumerateDevices(VideoDX9AdapterInfo* adapter_info, ArrayList& adapter_format_list);
	HRESULT  EnumerateDeviceCombos(VideoDX9DeviceInfo* device_info, ArrayList& adapter_format_list);

	void     BuildDepthStencilFormatList(VideoDX9DeviceCombo* device_combo);
	void     BuildMultiSampleTypeList(VideoDX9DeviceCombo* device_combo);
	void     BuildDSMSConflictList(VideoDX9DeviceCombo* device_combo);
	void     BuildVertexProcessingTypeList(VideoDX9DeviceInfo* device_info, VideoDX9DeviceCombo* device_combo);
	void     BuildPresentIntervalList(VideoDX9DeviceInfo* device_info, VideoDX9DeviceCombo* device_combo);

	IDirect3D9*                d3d;
	ArrayList                  allowed_adapter_format_list;

	List<VideoDX9AdapterInfo>  adapter_info_list;
	int                        adapter_index;
};


// +--------------------------------------------------------------------+

struct VideoDX9AdapterInfo
{
	static const char* TYPENAME() { return "VideoDX9AdapterInfo"; }

	VideoDX9AdapterInfo();
	~VideoDX9AdapterInfo();

	const char* GetDescription() const;

	int                        adapter_ordinal;
	D3DADAPTER_IDENTIFIER9     adapter_identifier;
	List<VideoDX9DisplayMode>  display_mode_list;
	List<VideoDX9DeviceInfo>   device_info_list;
};


// +--------------------------------------------------------------------+

struct VideoDX9DisplayMode
{
	static const char* TYPENAME() { return "VideoDX9DisplayMode"; }

	VideoDX9DisplayMode();
	VideoDX9DisplayMode(const VideoDX9DisplayMode& m);
	VideoDX9DisplayMode(const D3DDISPLAYMODE& m);

	int operator<(const VideoDX9DisplayMode& m) const;
	int operator<=(const VideoDX9DisplayMode& m) const;

	const char*          GetDescription() const;
	static const char*   D3DFormatToString(D3DFORMAT format);

	UINT        width;
	UINT        height;
	UINT        refresh;
	D3DFORMAT   format;
};


// +--------------------------------------------------------------------+

struct VideoDX9DeviceInfo
{
	static const char* TYPENAME() { return "VideoDX9DeviceInfo"; }

	VideoDX9DeviceInfo();
	~VideoDX9DeviceInfo();

	int                        adapter_ordinal;
	D3DDEVTYPE                 device_type;
	D3DCAPS9                   caps;
	List<VideoDX9DeviceCombo>  device_combo_list;
};


// +--------------------------------------------------------------------+

struct VideoDX9FormatConflict
{
	static const char* TYPENAME() { return "VideoDX9FormatConflict"; }

	D3DFORMAT                  ds_format;
	D3DMULTISAMPLE_TYPE        multisample_type;
};


// +--------------------------------------------------------------------+

struct VideoDX9DeviceCombo
{
	VideoDX9DeviceCombo();
	~VideoDX9DeviceCombo();

	int                  adapter_ordinal;
	D3DDEVTYPE           device_type;
	D3DFORMAT            adapter_format;
	D3DFORMAT            back_buffer_format;
	bool                 is_windowed;

	ArrayList            vertex_processing_list;
	ArrayList            depth_stencil_fmt_list;
	ArrayList            multisample_type_list;
	ArrayList            multisample_qual_list;
	ArrayList            present_interval_list;

	List<VideoDX9FormatConflict> conflict_list;
};

// +--------------------------------------------------------------------+

#endif VideoDX9Enum_h

