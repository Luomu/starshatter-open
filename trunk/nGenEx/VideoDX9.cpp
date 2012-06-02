/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         VideoDX9.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Direct3D Video class for DirectX 9
*/

#include "MemDebug.h"
#include "VideoDX9.h"
#include "VideoDX9Enum.h"
#include "VideoDX9VertexBuffer.h"
#include "TexDX9.h"
#include "TexCubeDX9.h"
#include "Camera.h"
#include "Color.h"
#include "DataLoader.h"
#include "Polygon.h"
#include "Light.h"
#include "Solid.h"

// +--------------------------------------------------------------------+

void                 Print(const char* msg, ...);
char*                D3DErrStr(HRESULT dderr);
void                 VideoDX9Error(const char* msg, HRESULT dderr);
static TexCacheDX9*  texcache = 0;
static TexCubeDX9*   environment_cube = 0;
static bool          surface_has_tangent_data = false;
static Light*        main_light;
static Light*        back_light;
static D3DXMATRIX    matrixWorld;
static D3DXMATRIX    matrixView;
static D3DXMATRIX    matrixProj;
static D3DXMATRIX    matrixWorldInverse;

extern int           VD3D_describe_things;

#ifndef RELEASE
#define RELEASE(x) if (x) { x->Release(); x=NULL; }
#endif

#ifndef F2DW
#define F2DW(x) (*(DWORD*)(&x))
#endif

#ifndef DW2I
#define DW2I(x) (*(int*)(&x))
#endif

// +--------------------------------------------------------------------+

typedef HRESULT (WINAPI * LPDDCE)(GUID FAR *, LPVOID *, REFIID , IUnknown FAR *);

static D3DMATRIX identity_matrix = {
	FLOAT(1.0), FLOAT(0.0), FLOAT(0.0), FLOAT(0.0),
	FLOAT(0.0), FLOAT(1.0), FLOAT(0.0), FLOAT(0.0),
	FLOAT(0.0), FLOAT(0.0), FLOAT(1.0), FLOAT(0.0),
	FLOAT(0.0), FLOAT(0.0), FLOAT(0.0), FLOAT(1.0)
};

// +--------------------------------------------------------------------+

List<Model> model_clients;

class VideoDX9SurfaceData : public VideoPrivateData
{
public:
	VideoDX9SurfaceData(Model* m) : model(m), vertex_buffer(0), index_buffer(0) {
		if (!model_clients.contains(model))
		model_clients.append(model);
	}

	virtual ~VideoDX9SurfaceData() { 
		model_clients.remove(model);

		delete vertex_buffer;
		delete index_buffer;
	}

	enum { TYPE = 9001 };
	virtual int GetType()   const { return TYPE; }

	Model*                  model;
	VideoDX9VertexBuffer*   vertex_buffer;
	VideoDX9IndexBuffer*    index_buffer;
};

class VideoDX9SegmentData : public VideoPrivateData
{
public:
	VideoDX9SegmentData() : first_vert(0), num_verts(0), first_index(0), num_tris(0) { }
	virtual ~VideoDX9SegmentData() { }

	enum { TYPE = 9002 };
	virtual int GetType()   const { return TYPE; }

	int      first_vert;
	int      num_verts;
	int      first_index;
	int      num_tris;
};

// +--------------------------------------------------------------------+

static int d3dstate_table[] = {
	D3DRS_FILLMODE,            // FILL_MODE
	D3DRS_SHADEMODE,           // SHADE_MODE
	D3DRS_LIGHTING,            // LIGHTING_ENABLE
	D3DRS_ZENABLE,             // Z_ENABLE
	D3DRS_ZWRITEENABLE,        // Z_WRITE_ENABLE
	D3DRS_DEPTHBIAS,           // Z_BIAS
	0,                         // TEXTURE_FILTER
	D3DRS_DITHERENABLE,        // DITHER_ENABLE
	D3DRS_SPECULARENABLE,      // SPECULAR_ENABLE
	D3DRS_FOGENABLE,           // FOG_ENABLE
	D3DRS_FOGCOLOR,            // FOG_COLOR
	D3DRS_FOGDENSITY,          // FOG_DENSITY
	D3DRS_STENCILENABLE,       // STENCIL_ENABLE
	0x11111111,                // TEXTURE_WRAP (special case)
	0                          // LIGHTING_PASS
};

static const int NUM_SCREEN_VERTS   = 1024;
static const int NUM_SCREEN_INDICES = NUM_SCREEN_VERTS * 2;

// +--------------------------------------------------------------------+

struct VideoDX9ScreenVertex
{
	FLOAT sx, sy, sz, rhw;
	DWORD diffuse;
	FLOAT tu, tv;

	static DWORD FVF;
};

DWORD VideoDX9ScreenVertex::FVF     = D3DFVF_XYZRHW   | 
D3DFVF_DIFFUSE  | 
D3DFVF_TEX1;

struct VideoDX9NormalVertex
{
	FLOAT x,  y,  z;
	FLOAT nx, ny, nz;
	FLOAT t0u, t0v;
	FLOAT t1u, t1v;
	FLOAT tx, ty, tz;
	FLOAT bx, by, bz;

	static DWORD FVF;
};

DWORD VideoDX9NormalVertex::FVF     = 0;

// Global Vertex Declaration shared by shaders
D3DVERTEXELEMENT9 videoDX9NormalVertexElements[] = 
{
	{ 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
	{ 0, 24, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{ 0, 32, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
	{ 0, 40, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  2 },
	{ 0, 52, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 3 },
	D3DDECL_END()
};

struct VideoDX9SolidVertex
{
	FLOAT x,  y,  z;
	FLOAT nx, ny, nz;
	FLOAT tu, tv;

	static DWORD FVF;
};

DWORD VideoDX9SolidVertex::FVF      = D3DFVF_XYZ      | 
D3DFVF_NORMAL   | 
D3DFVF_TEX1     |
D3DFVF_TEXCOORDSIZE2(0);

struct VideoDX9LuminousVertex
{
	FLOAT x,  y,  z;
	DWORD diffuse;
	FLOAT tu, tv;

	static DWORD FVF;
};

DWORD VideoDX9LuminousVertex::FVF   = D3DFVF_XYZ      | 
D3DFVF_DIFFUSE  | 
D3DFVF_TEX1     |
D3DFVF_TEXCOORDSIZE2(0);

struct VideoDX9DetailVertex
{
	FLOAT x,  y,  z;
	DWORD diffuse;
	DWORD specular;
	FLOAT tu,  tv;
	FLOAT tu1, tv1;

	static DWORD FVF;
};

DWORD VideoDX9DetailVertex::FVF     = D3DFVF_XYZ      | 
D3DFVF_DIFFUSE  |
D3DFVF_SPECULAR |
D3DFVF_TEX2;

struct VideoDX9LineVertex
{
	FLOAT x,  y,  z;
	DWORD diffuse;

	static DWORD FVF;
};

DWORD VideoDX9LineVertex::FVF       = D3DFVF_XYZ      | 
D3DFVF_DIFFUSE;

enum {
	DX9_STRATEGY_NONE,
	DX9_STRATEGY_SIMPLE,
	DX9_STRATEGY_GLOW,
	DX9_STRATEGY_SPECMAP,
	DX9_STRATEGY_EMISSIVE,
	DX9_STRATEGY_SPEC_EMISSIVE,
	DX9_STRATEGY_BLEND,
	DX9_STRATEGY_BLEND_DETAIL
};

// +--------------------------------------------------------------------+

static VideoDX9* video_dx9_instance = 0;

VideoDX9::VideoDX9(const HWND& window, VideoSettings* vs)
: width(0), height(0), bpp(0), hwnd(window), surface(0),
d3d(0), d3ddevice(0), device_lost(false), fade(0),
zdepth(0), gamma(128), num_verts(0), first_vert(0),
current_texture(0), screen_vbuf(0), screen_ibuf(0),
font_verts(0), font_indices(0), font_nverts(0),
nlights(0), use_material(0), d3dx_font(0),
segment_material(0), strategy(0), passes(0),
screen_line_verts(0), line_verts(0),
vertex_declaration(0),
magic_fx(0), magic_fx_code(0), magic_fx_code_len(0)
{
	video_dx9_instance = this;

	Print("\n********************************\n");
	Print("*   Direct 3D version 9        *\n");
	Print("********************************\n\n");

	status = VIDEO_ERR;
	HRESULT err = E_OUTOFMEMORY;

	d3d      = Direct3DCreate9(D3D_SDK_VERSION);
	dx9enum  = new(__FILE__,__LINE__) VideoDX9Enum(d3d);

	if (d3d && dx9enum) {
		if (vs) {
			dx9enum->req_fullscreen    = vs->is_windowed ? false : true;
			dx9enum->req_windowed      = vs->is_windowed ? true  : false;
			dx9enum->min_stencil_bits  = vs->shadows ? 8 : 0;
			dx9enum->uses_depth_buffer = true;
		}
		else {
			dx9enum->req_fullscreen    = video_settings.is_windowed ? false : true;
			dx9enum->req_windowed      = video_settings.is_windowed ? true  : false;
			dx9enum->min_stencil_bits  = video_settings.shadows ? 8 : 0;
			dx9enum->uses_depth_buffer = true;
		}

		err = dx9enum->Enumerate();

		if (FAILED(err)) {
			VideoDX9Error("(ctor) could not enumerate dx9 properties", err);
			delete dx9enum;
			return;
		}
	}
	else {
		VideoDX9Error("(ctor) could not create enumerator", err);
		return;
	}

	SetVideoSettings(vs);

	if (video_settings.is_windowed)
	dx9enum->SuggestWindowSettings(&video_settings);
	else
	dx9enum->SuggestFullscreenSettings(&video_settings);

	SetupParams();

	if (VD3D_describe_things > 2) {
		Print("\nD3DPRESENT_PARAMETERS:\n");
		Print("   BackBufferWidth:       %d\n", d3dparams.BackBufferWidth);
		Print("   BackBufferHeight:      %d\n", d3dparams.BackBufferHeight);
		Print("   BackBufferCount:       %d\n", d3dparams.BackBufferCount);
		Print("   BackBufferFormat:      %s\n", VideoDX9DisplayMode::D3DFormatToString(d3dparams.BackBufferFormat));
		Print("   Multisample Type:      %d\n", d3dparams.MultiSampleType);
		Print("   Multisample Qual:      %d\n", d3dparams.MultiSampleQuality);
		Print("   Swap Effect:           %d\n", d3dparams.SwapEffect);
		Print("   Device Window:         %08X\n", d3dparams.hDeviceWindow);
		Print("   Windowed:              %s\n", d3dparams.Windowed ? "true" : "false");
		Print("   Enable Depth/Stencil:  %s\n", d3dparams.EnableAutoDepthStencil ? "true" : "false");
		Print("   Depth/Stencil Format:  %s\n", VideoDX9DisplayMode::D3DFormatToString(d3dparams.AutoDepthStencilFormat));
		Print("   Flags:                 %08X\n", d3dparams.Flags);
		Print("   Fullscreen Refresh:    %d Hz\n", d3dparams.FullScreen_RefreshRateInHz);

		switch (d3dparams.PresentationInterval) {
		case D3DPRESENT_INTERVAL_IMMEDIATE:
			Print("   Present Interval:      IMMEDIATE\n");
			break;

		case D3DPRESENT_INTERVAL_DEFAULT:
			Print("   Present Interval:      DEFAULT\n");
			break;

		case D3DPRESENT_INTERVAL_ONE:
			Print("   Present Interval:      ONE\n");
			break;

		case D3DPRESENT_INTERVAL_TWO:
			Print("   Present Interval:      TWO\n");
			break;

		case D3DPRESENT_INTERVAL_THREE:
			Print("   Present Interval:      THREE\n");
			break;

		case D3DPRESENT_INTERVAL_FOUR:
			Print("   Present Interval:      FOUR\n");
			break;

		default:
			Print("   Present Interval:      Unknown (%d)\n", d3dparams.PresentationInterval);
			break;
		}

		Print("\n");
	}

	Print("   Creating Video Device for HWND = %08x\n", window);

	err = d3d->CreateDevice(D3DADAPTER_DEFAULT,
	D3DDEVTYPE_HAL,
	window,
	D3DCREATE_HARDWARE_VERTEXPROCESSING,
	&d3dparams,
	&d3ddevice);

	if (FAILED(err)) {
		VideoDX9Error("(ctor) could not create device", err);
		return;
	}

	width                         = video_settings.GetWidth();
	height                        = video_settings.GetHeight();
	bpp                           = video_settings.GetDepth();

	shadow_enabled                = vs->shadows;
	bump_enabled                  = vs->bumpmaps;
	spec_enabled                  = vs->specmaps;

	render_state[FILL_MODE]       = FILL_SOLID;
	render_state[SHADE_MODE]      = SHADE_GOURAUD;
	render_state[Z_ENABLE]        = false;
	render_state[Z_WRITE_ENABLE]  = false;
	render_state[Z_BIAS]          = 0;
	render_state[TEXTURE_FILTER]  = FILTER_LINEAR;
	render_state[DITHER_ENABLE]   = false;
	render_state[SPECULAR_ENABLE] = true;
	render_state[FOG_ENABLE]      = false;
	render_state[FOG_COLOR]       = 0;
	render_state[FOG_DENSITY]     = 0;
	render_state[STENCIL_ENABLE]  = false;
	render_state[TEXTURE_WRAP]    = true;
	render_state[LIGHTING_PASS]   = 0;

	ZeroMemory(&rect, sizeof(rect));

	if (!texcache)
	texcache = new(__FILE__,__LINE__) TexCacheDX9(this);

	if (texcache)
	texcache->count++;

	if (VD3D_describe_things > 0) {
		DWORD vmf = VidMemFree() / (1024 * 1024);
		Print("   Available Texture Memory: %d MB\n\n", vmf);
	}

	if (CreateBuffers()) {
		d3ddevice->SetRenderState(D3DRS_ALPHATESTENABLE,   false);
		d3ddevice->SetRenderState(D3DRS_ZFUNC,             D3DCMP_LESSEQUAL);

		d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP,    D3DTOP_MODULATE);
		d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,  D3DTA_TEXTURE);
		d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG2,  D3DTA_DIFFUSE);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		status = VIDEO_OK;
	}

	ZeroMemory(font_name, 64);
	font_size = 0;
	font_bold = false;
	font_ital = false;
}

// +--------------------------------------------------------------------+

VideoDX9::~VideoDX9()
{
	DestroyBuffers();

	texcache->count--;
	if (!texcache->count) {
		delete texcache;
		texcache = 0;
	}

	delete environment_cube;
	delete dx9enum;

	RELEASE(d3dx_font);
	RELEASE(d3ddevice);
	RELEASE(d3d);

	if (magic_fx_code)
	delete [] magic_fx_code;

	Print("   VideoDX9: shutdown\n");
	video_dx9_instance = 0;
}

IDirect3DDevice9*  
VideoDX9::GetD3DDevice9()
{
	if (video_dx9_instance)
	return video_dx9_instance->d3ddevice;

	return 0;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::SetupParams()
{
	if (!dx9enum || dx9enum->NumAdapters() < 1) {
		status = VIDEO_ERR;
		return false;
	}

	int adapter_index = video_settings.GetAdapterIndex();

	if (adapter_index < 0 || adapter_index >= dx9enum->NumAdapters()) {
		::Print("WARNING: VideoDX9 could not select adapter %d (max=%d)\n",
		adapter_index, dx9enum->NumAdapters());

		adapter_index = 0;
	}

	dx9enum->SelectAdapter(adapter_index);

	d3dparams.Windowed                  = video_settings.IsWindowed();
	d3dparams.BackBufferCount           = 2;
	d3dparams.MultiSampleType           = D3DMULTISAMPLE_NONE;
	d3dparams.MultiSampleQuality        = 0;
	d3dparams.SwapEffect                = D3DSWAPEFFECT_DISCARD;
	d3dparams.EnableAutoDepthStencil    = dx9enum->uses_depth_buffer;
	d3dparams.hDeviceWindow             = hwnd;

	if (dx9enum->uses_depth_buffer) {
		d3dparams.Flags                  = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		d3dparams.AutoDepthStencilFormat = (D3DFORMAT) video_settings.GetDepthStencilFormat();
	}
	else {
		d3dparams.Flags                  = 0;
	}

	d3dparams.Flags |= D3DPRESENTFLAG_DEVICECLIP;

	if (video_settings.IsWindowed()) {
		d3dparams.BackBufferWidth        = video_settings.window_width;
		d3dparams.BackBufferHeight       = video_settings.window_height;
		d3dparams.BackBufferFormat       = (D3DFORMAT) video_settings.GetBackBufferFormat();
		d3dparams.FullScreen_RefreshRateInHz = 0;
		d3dparams.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	else {
		d3dparams.BackBufferWidth        = video_settings.GetWidth();
		d3dparams.BackBufferHeight       = video_settings.GetHeight();
		d3dparams.BackBufferFormat       = (D3DFORMAT) video_settings.GetBackBufferFormat();
		d3dparams.FullScreen_RefreshRateInHz = video_settings.GetRefreshRate();
		d3dparams.PresentationInterval   = D3DPRESENT_INTERVAL_DEFAULT;
	}

	return true;
}


bool
VideoDX9::IsModeSupported(int w, int h, int b) const
{
	if (dx9enum)
	return dx9enum->IsModeSupported(w, h, b);

	return false;
}

bool
VideoDX9::SetVideoSettings(const VideoSettings* vs)
{
	// custom video settings:
	if (vs) {
		if (vs != &video_settings)
		CopyMemory(&video_settings, vs, sizeof(VideoSettings));
	}

	// default video settings:
	else {
		ZeroMemory(&video_settings, sizeof(VideoSettings));

		video_settings.fullscreen_mode.width   = 800;
		video_settings.fullscreen_mode.height  = 600;
		video_settings.fullscreen_mode.format  = VideoMode::FMT_X8R8G8B8;
	}

	return true;
}

bool
VideoDX9::Reset(const VideoSettings* vs)
{
	if (!d3ddevice || !SetVideoSettings(vs)) {
		status = VIDEO_ERR;
		return false;
	}

	bool using_x_font = (d3dx_font != 0);

	RELEASE(d3dx_font);
	InvalidateCache();
	DestroyBuffers();
	SetupParams();

	HRESULT hr = d3ddevice->Reset(&d3dparams);

	if (FAILED(hr)) {
		VideoDX9Error("could not reset d3d device", hr);
		status = VIDEO_ERR;
		return false;
	}

	// Store render target surface desc
	IDirect3DSurface9* back_buffer;
	d3ddevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
	back_buffer->GetDesc(&back_buffer_desc);
	RELEASE(back_buffer);

	width          = video_settings.GetWidth();
	height         = video_settings.GetHeight();
	bpp            = video_settings.GetDepth();

	shadow_enabled = vs->shadows;
	bump_enabled   = vs->bumpmaps;
	spec_enabled   = vs->specmaps;


	if (CreateBuffers()) {
		d3ddevice->SetRenderState(D3DRS_ALPHATESTENABLE,   false);
		d3ddevice->SetRenderState(D3DRS_ZFUNC,             D3DCMP_LESSEQUAL);

		d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP,    D3DTOP_MODULATE);
		d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,  D3DTA_TEXTURE);
		d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG2,  D3DTA_DIFFUSE);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		D3DVIEWPORT9 view;
		
		hr = d3ddevice->GetViewport(&view);
		if (SUCCEEDED(hr)) {
			rect.x = view.X;
			rect.y = view.Y;
			rect.w = view.Width;
			rect.h = view.Height;
		}

		if (using_x_font)
		UseXFont(font_name, font_size, font_bold, font_ital);

		status = VIDEO_OK;
	}

	return status == VIDEO_OK;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::CreateBuffers()
{
	if (d3ddevice) {
		UINT     vertex_size = sizeof(VideoDX9ScreenVertex);
		UINT     index_size  = sizeof(WORD);

		if (!screen_vbuf) {
			screen_vbuf = new(__FILE__,__LINE__) VideoDX9VertexBuffer(
			this,
			NUM_SCREEN_VERTS,
			vertex_size,
			VideoDX9ScreenVertex::FVF,
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY);
		}

		if (!screen_ibuf) {
			screen_ibuf = new(__FILE__,__LINE__) VideoDX9IndexBuffer(
			this,
			NUM_SCREEN_INDICES,
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY);
		}

		screen_line_verts = new(__FILE__,__LINE__) VideoDX9ScreenVertex[256];
		line_verts        = new(__FILE__,__LINE__) VideoDX9LineVertex[512];

		// create effects:
		LPD3DXBUFFER   code_buffer          = 0;
		DataLoader*    loader               = DataLoader::GetLoader();
		HRESULT        hr                   = E_FAIL;

		hr = d3ddevice->CreateVertexDeclaration(videoDX9NormalVertexElements,
		&vertex_declaration);

		// The E - We want to load our shader from the standard filesystem by default, to allow for better modding.
		if (video_settings.use_effects && !magic_fx_code)  {
			FILE* f;
			::fopen_s(&f, "magic.fx", "rb");

			if (f) {
				::fseek(f, 0, SEEK_END);
				magic_fx_code_len = ftell(f);
				::fseek(f, 0, SEEK_SET);

				magic_fx_code = new(__FILE__,__LINE__) BYTE[magic_fx_code_len+1];
				if (magic_fx_code) {
					::fread(magic_fx_code, magic_fx_code_len, 1, f);
					magic_fx_code[magic_fx_code_len] = 0;
				}
				::fclose(f);
			} else if (loader) {
				magic_fx_code_len = loader->LoadBuffer("magic.fx", magic_fx_code, true, true);
			}
		}

		if (video_settings.use_effects && magic_fx_code && magic_fx_code_len) {
			hr = D3DXCreateEffect(d3ddevice,
			magic_fx_code,
			magic_fx_code_len,
			0, 0, 0, 0,
			&magic_fx,
			&code_buffer);

			if (code_buffer) {
				::Print("ERROR - Failed to compile 'magic.fx'\n");
				::Print((const char*) code_buffer->GetBufferPointer());
				::Print("\n\n");
				RELEASE(code_buffer);
			}
		}
	}

	return screen_vbuf && screen_ibuf;
}

bool
VideoDX9::DestroyBuffers()
{
	if (line_verts) {
		delete line_verts;
		line_verts = 0;
	}

	if (screen_line_verts) {
		delete screen_line_verts;
		screen_line_verts = 0;
	}

	if (screen_vbuf) {
		delete screen_vbuf;
		screen_vbuf = 0;
	}

	if (screen_ibuf) {
		delete screen_ibuf;
		screen_ibuf = 0;
	}

	if (font_verts) {
		delete [] font_verts;
		font_verts = 0;
	}

	if (font_indices) {
		delete [] font_indices;
		font_indices = 0;
	}

	font_nverts = 0;

	RELEASE(vertex_declaration);
	RELEASE(magic_fx);

	return true;
}

// +--------------------------------------------------------------------+

DWORD
VideoDX9::VidMemFree() const
{
	UINT result = 0;

	if (d3ddevice)
	result = d3ddevice->GetAvailableTextureMem();

	return result;
}

int
VideoDX9::MaxTexSize() const
{
	if (d3d && dx9enum && dx9enum->GetAdapterInfo()) {
		VideoDX9DeviceInfo* dev_info = dx9enum->GetDeviceInfo(video_settings.GetDeviceType());

		if (dev_info) {
			return (int) dev_info->caps.MaxTextureWidth;
		}
	}

	return 0;
}

int
VideoDX9::MaxTexAspect() const
{
	if (d3d && dx9enum && dx9enum->GetAdapterInfo()) {
		VideoDX9DeviceInfo* dev_info = dx9enum->GetDeviceInfo(video_settings.GetDeviceType());

		if (dev_info) {
			return (int) dev_info->caps.MaxTextureAspectRatio;
		}
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
VideoDX9::RecoverSurfaces()
{
	Print("VideoDX9::RecoverSurfaces()\n");

	HRESULT hr = D3D_OK;

	surface = 0;

	hr = d3ddevice->TestCooperativeLevel();

	if (hr == D3DERR_DEVICELOST) {
		// This means that some app took exclusive mode access
		// we need to sit in a loop till we get back to the right mode.
		Print("D3DERR_DEVICELOST\n");

		do {
			Sleep(500);
			hr = d3ddevice->TestCooperativeLevel();
		} while (hr == D3DERR_DEVICELOST);
	}

	if (hr == D3DERR_DEVICENOTRESET) {
		if (Reset(&video_settings))
		hr = S_OK;
	}

	if (SUCCEEDED(hr)) {
		Print("* Invalidating Texture Cache\n");
		// Re-fill the contents of textures which just got restored:
		InvalidateCache();

		device_lost = false;
	}

	Print("* Vid Mem Free: %8d\n", VidMemFree());
	Print("* Recover Surfaces Complete.\n\n");
}

// +--------------------------------------------------------------------+

bool
VideoDX9::SetBackgroundColor(Color c)
{
	background = c;
	return true;
}

//-----------------------------------------------------------------------------
// RampValue
//
// The gamma function with inputs in [0,255], scaled to a range with the
// default range appropriate for D3DGAMMARAMP.
//
inline WORD
RampValue(UINT i, double recip_gamma, double fade)
{
	return (WORD) (65535.0 * fade * pow((double)i/255.f, recip_gamma));
}

//-----------------------------------------------------------------------------
// ReciprocalGamma
//
// Given a gamma corrected i in [0,255], return 1/gamma
//
inline float
ReciprocalGamma(UINT i)
{
	return logf(i/255.f)/logf(0.5f);
}

//-----------------------------------------------------------------------------
// GammaValue
//
// Given a gamma corrected color channel value in [0,255], return the gamma.
//
inline float
GammaValue(UINT i)
{
	return logf(0.5f)/logf(i/255.f);
}

bool
VideoDX9::SetGammaLevel(int g)
{
	HRESULT  hr = E_FAIL;
	double   f  = Color::GetFade();

	if (gamma != g || fade != f) {
		if (d3ddevice) {
			//::Print("VideoDX9 - SetGammaLevel(%d) fade = %f\n", g, f);

			// compute 1/gamma
			float recip_gray = ReciprocalGamma(g);
			
			// compute i**(1/gamma) for all i and scale to range
			for (UINT i = 0; i < 256; i++) {
				int val = RampValue(i, recip_gray, f);

				gamma_ramp.red[i]   = val;
				gamma_ramp.green[i] = val;
				gamma_ramp.blue[i]  = val;
			}

			d3ddevice->SetGammaRamp(0, D3DSGR_NO_CALIBRATION, &gamma_ramp);
			hr = D3D_OK;
		}

		gamma = g;
		fade  = f;
	}

	return SUCCEEDED(hr);
}

bool
VideoDX9::SetObjTransform(const Matrix& orient, const Point& loc)
{
	HRESULT   hr = E_FAIL;

	if (d3ddevice) {
		D3DMATRIX world_matrix;
		CreateD3DMatrix(world_matrix, orient, loc);
		hr = d3ddevice->SetTransform(D3DTS_WORLD, &world_matrix);

		matrixWorld = world_matrix;
		D3DXMatrixInverse(&matrixWorldInverse, 0, &matrixWorld);
	}

	return SUCCEEDED(hr);
}

// +--------------------------------------------------------------------+

bool
VideoDX9::ClearAll()
{
	HRESULT err;

	err = d3ddevice->Clear(0,
	NULL,
	D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER,
	background.Value(),
	1.0f,
	0);

	if (FAILED(err)) {
		static int report = 10;
		if (report > 0) {
			VideoDX9Error("Failed to clear device", err);
			report--;
		}
	}

	return true;
}

bool
VideoDX9::ClearDepthBuffer()
{
	HRESULT err;

	err = d3ddevice->Clear(0,
	NULL,
	D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER,
	0,
	1.0f,
	0);

	if (FAILED(err)) {
		static int report = 10;
		if (report > 0) {
			VideoDX9Error("Failed to clear depth buffer", err);
			report--;
		}
	}

	return true;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::Present()
{
	// Show the frame on the primary surface.
	HRESULT err = d3ddevice->Present( NULL, NULL, NULL, NULL );

	if (FAILED(err)) {
		if (err == D3DERR_DEVICELOST) {
			device_lost = true;
		}

		else {
			static int report = 10;
			if (report > 0) {
				VideoDX9Error("Could not present frame", err);
				report--;
			}
		}
	}

	return true;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::Pause()
{
	return true;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::Resume()
{
	return true;
}

// +--------------------------------------------------------------------+

void
VideoDX9::PreloadSurface(Surface* s)
{
	if (s)
	PrepareSurface(s);
}

void
VideoDX9::PreloadTexture(Bitmap* tex)
{
	if (texcache && tex)
	texcache->FindTexture(tex);
}

void
VideoDX9::InvalidateCache()
{
	ListIter<Model> iter = model_clients;
	while (++iter) {
		// remove each model from the list...
		Model* model = iter.removeItem();

		// ...so that the buffer destructor doesn't
		// do it and mess up the iterator.
		model->DeletePrivateData();
	}

	if (texcache)
	texcache->InvalidateCache();
}

// +--------------------------------------------------------------------+

void
VideoDX9::CreateD3DMatrix(D3DMATRIX& result, const Matrix& m, const Point& p)
{
	result._11  = (float) m.elem[0][0];
	result._12  = (float) m.elem[1][0];
	result._13  = (float) m.elem[2][0];
	result._14  = 0.0f;

	result._21  = (float) m.elem[0][1];
	result._22  = (float) m.elem[1][1];
	result._23  = (float) m.elem[2][1];
	result._24  = 0.0f;

	result._31  = (float) m.elem[0][2];
	result._32  = (float) m.elem[1][2];
	result._33  = (float) m.elem[2][2];
	result._34  = 0.0f;

	result._41  = (float) p.x;
	result._42  = (float) p.y;
	result._43  = (float) p.z;
	result._44  = 1.0f;
}

void
VideoDX9::CreateD3DMatrix(D3DMATRIX& result, const Matrix& m, const Vec3& v)
{
	result._11  = (float) m.elem[0][0];
	result._12  = (float) m.elem[1][0];
	result._13  = (float) m.elem[2][0];
	result._14  = 0.0f;

	result._21  = (float) m.elem[0][1];
	result._22  = (float) m.elem[1][1];
	result._23  = (float) m.elem[2][1];
	result._24  = 0.0f;

	result._31  = (float) m.elem[0][2];
	result._32  = (float) m.elem[1][2];
	result._33  = (float) m.elem[2][2];
	result._34  = 0.0f;

	result._41  = v.x;
	result._42  = v.y;
	result._43  = v.z;
	result._44  = 1.0f;
}

void
VideoDX9::CreateD3DMaterial(D3DMATERIAL9& result, const Material& mtl)
{
	CopyMemory(&result.Diffuse,  &mtl.Kd, sizeof(D3DCOLORVALUE));
	CopyMemory(&result.Ambient,  &mtl.Ka, sizeof(D3DCOLORVALUE));
	CopyMemory(&result.Specular, &mtl.Ks, sizeof(D3DCOLORVALUE));
	CopyMemory(&result.Emissive, &mtl.Ke, sizeof(D3DCOLORVALUE));

	result.Power = mtl.power;
}


// +--------------------------------------------------------------------+

bool
VideoDX9::Capture(Bitmap& bmp)
{
	if (d3ddevice) {
		HRESULT hr = E_FAIL;
		LPDIRECT3DSURFACE9 pSurf=NULL, pTempSurf=NULL;
		D3DSURFACE_DESC desc;
		D3DDISPLAYMODE dm;

		// get display dimensions
		// this will be the dimensions of the front buffer
		hr = d3ddevice->GetDisplayMode(0, &dm);

		if (FAILED(hr))
		VideoDX9Error("VideoDX9::Capture - Can't get display mode!", hr);

		desc.Width = dm.Width;
		desc.Height = dm.Height;
		desc.Format = D3DFMT_A8R8G8B8;

		hr = d3ddevice->CreateOffscreenPlainSurface(
		desc.Width, 
		desc.Height, 
		desc.Format, 
		D3DPOOL_SYSTEMMEM, 
		&pTempSurf, 
		NULL);

		if (FAILED(hr)) {
			VideoDX9Error("VideoDX9::Capture - Cannot create offscreen buffer 1", hr);
			return false;
		}

		hr = d3ddevice->GetFrontBufferData(0, pTempSurf);

		if (FAILED(hr)) {
			RELEASE(pTempSurf);
			VideoDX9Error("VideoDX9::Capture - Can't get front buffer", hr);
			return false;
		}


		if (video_settings.IsWindowed()) {
			POINT pt={0, 0};
			RECT  srcRect;

			// capture only the client area of the screen:
			::GetClientRect(hwnd, &srcRect);
			::ClientToScreen(hwnd, (LPPOINT) &srcRect);
			srcRect.right  += srcRect.left;
			srcRect.bottom += srcRect.top;

			desc.Width  = srcRect.right - srcRect.left;
			desc.Height = srcRect.bottom - srcRect.top;
			desc.Format = D3DFMT_A8R8G8B8;   // this is what we get from the screen, so stick with it

			// NB we can't lock the back buffer direct because it's no created that way
			// and to do so hits performance, so copy to another surface
			// Must be the same format as the source surface
			hr = d3ddevice->CreateOffscreenPlainSurface(
			desc.Width, 
			desc.Height, 
			desc.Format, 
			D3DPOOL_DEFAULT, 
			&pSurf,
			NULL);

			if (FAILED(hr)) {
				RELEASE(pSurf);
				VideoDX9Error("VideoDX9::Capture - Cannot create offscreen buffer 2", hr);
				return false;
			}

			// Copy
			hr = d3ddevice->UpdateSurface(pTempSurf, &srcRect, pSurf, &pt);

			if (FAILED(hr)) {
				RELEASE(pTempSurf);
				RELEASE(pSurf);
				VideoDX9Error("VideoDX9::Capture - Cannot update surface", hr);
				return false;
			}

			RELEASE(pTempSurf);
			pTempSurf = pSurf;
			pSurf = NULL;
		}

		D3DLOCKED_RECT lockedRect;
		hr = pTempSurf->LockRect(&lockedRect, NULL, 
		D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);

		if (FAILED(hr)) {
			VideoDX9Error("VideoDX9::Capture - can't lock rect", hr);
			RELEASE(pTempSurf);
			return false;
		} 

		// Allocate color buffer
		DWORD*   buffer = new DWORD[desc.Width * desc.Height];
		BYTE*    src    = (BYTE*) lockedRect.pBits;
		BYTE*    dst    = (BYTE*) buffer;
		Color    clr;

		for (DWORD y = 0; y < desc.Height; y++) {
			BYTE *pRow = src;

			for (DWORD x = 0; x < desc.Width; x++) {
				switch(desc.Format) {
				case D3DFMT_R5G6B5:
					clr = Color::Unformat(*((WORD*) (pRow)));

					*dst++ = (BYTE) clr.Red();
					*dst++ = (BYTE) clr.Green();
					*dst++ = (BYTE) clr.Blue();
					*dst++ = 255;
					break;

				case D3DFMT_A8R8G8B8:
				case D3DFMT_X8R8G8B8:
					*dst++ = pRow[0]; // R
					*dst++ = pRow[1]; // G
					*dst++ = pRow[2]; // B
					*dst++ = 255;

					pRow += 4;
					break;

				case D3DFMT_R8G8B8:
					*dst++ = pRow[0]; // R
					*dst++ = pRow[1]; // G
					*dst++ = pRow[2]; // B
					*dst++ = 255;

					pRow += 3; 
					break;
				}

			}

			src += lockedRect.Pitch;
		}

		bmp.CopyHighColorImage(desc.Width, desc.Height, buffer);

		delete [] buffer;

		RELEASE(pTempSurf);
		RELEASE(pSurf);

		return SUCCEEDED(hr);
	}

	return false;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::GetWindowRect(Rect& r)
{
	if (d3ddevice && (rect.w < 1 || rect.h < 1)) {
		D3DVIEWPORT9 view;
		HRESULT hr = d3ddevice->GetViewport(&view);
		if (SUCCEEDED(hr)) {
			rect.x = view.X;
			rect.y = view.Y;
			rect.w = view.Width;
			rect.h = view.Height;
		}
	}

	r = rect;
	return true;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::SetWindowRect(const Rect& r)
{
	return SetViewport(r.x, r.y, r.w, r.h);
}

// +--------------------------------------------------------------------+

bool
VideoDX9::SetViewport(int x, int y, int w, int h)
{
	if (!d3d || !d3ddevice)
	return false;

	HRESULT hr;

	// set up the viewport according to args:
	D3DVIEWPORT9 view;

	view.X          = x;
	view.Y          = y;
	view.Width      = w;
	view.Height     = h;
	view.MinZ       = 0.0f;   
	view.MaxZ       = 1.0f;   

	hr = d3ddevice->SetViewport(&view);
	if (FAILED(hr)) {
		VideoDX9Error("could not initialize viewport", hr);
		return false;
	}

	// set up the render state:
	for (int i = FILL_MODE; i < TEXTURE_WRAP; i++) {
		if (d3dstate_table[i]) {
			d3ddevice->SetRenderState((D3DRENDERSTATETYPE) d3dstate_table[i], render_state[i]);
		}
	}

	d3ddevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	return true;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::SetAmbient(Color c)
{
	ambient = c;
	return true;
}

bool
VideoDX9::SetLights(const List<Light>& lights)
{
	if (d3ddevice) {
		main_light = 0;
		back_light = 0;

		ListIter<Light> iter  = (List<Light>&) lights;
		int             index = -1;

		while (++iter) {
			Light* light = iter.value();

			if (light->IsActive()) {
				D3DLIGHT9 d3d_light;
				ZeroMemory(&d3d_light, sizeof(d3d_light));
				d3d_light.Type = (D3DLIGHTTYPE) light->Type();

				if (light->Type() == Light::LIGHT_DIRECTIONAL) {
					d3d_light.Direction.x = (float) (-light->Location().x);
					d3d_light.Direction.y = (float) (-light->Location().y);
					d3d_light.Direction.z = (float) (-light->Location().z);

					if (d3d_light.Direction.x == 0 &&
							d3d_light.Direction.y == 0 &&
							d3d_light.Direction.z == 0) {

						d3d_light.Direction.y = -1;
					}

					if (light->CastsShadow()) {
						if (!main_light || light->Intensity() > main_light->Intensity())
						main_light = light;
					}
					else if (!back_light) {
						back_light = light;
					}
				}
				else {
					d3d_light.Position.x  = (float) ( light->Location().x);
					d3d_light.Position.y  = (float) ( light->Location().y);
					d3d_light.Position.z  = (float) ( light->Location().z);
				}

				float r = (light->GetColor().Red()   / 255.0f) * light->Intensity();
				float g = (light->GetColor().Green() / 255.0f) * light->Intensity();
				float b = (light->GetColor().Blue()  / 255.0f) * light->Intensity();

				d3d_light.Diffuse.r  = r;
				d3d_light.Diffuse.g  = g;
				d3d_light.Diffuse.b  = b;

				d3d_light.Specular.r = r;
				d3d_light.Specular.g = g;
				d3d_light.Specular.b = b;

				d3d_light.Range        = light->Intensity() * 10.0f;
				d3d_light.Attenuation0 = 0.1f;
				d3d_light.Attenuation1 = 0.7f;
				d3d_light.Attenuation2 = 0.0f;

				index++;
				d3ddevice->SetLight(index, &d3d_light);
				d3ddevice->LightEnable(index, TRUE);
			}
		}

		// turn off any unused lights from before:
		while (nlights > index+1) {
			d3ddevice->LightEnable(--nlights, FALSE);
		}

		nlights = index + 1;

		return true;
	}

	return false;
}

bool
VideoDX9::SetCamera(const Camera* cam)
{
	if (d3ddevice) {
		camera = cam;

		D3DMATRIX m;
		CreateD3DMatrix(m, cam->Orientation(), cam->Pos());
		d3ddevice->SetTransform(D3DTS_VIEW, &m);
		matrixView = m;

		return true;
	}

	return false;
}

bool
VideoDX9::SetProjection(float fov, float znear, float zfar, DWORD type)
{
	if (d3ddevice && zfar > znear) {
		D3DMATRIX   m;
		float       h, w, Q;

		double width  = (float) (rect.w);
		double height = (float) (rect.h);
		ZeroMemory(&m, sizeof(m));

		/***
	*** PERSPECTIVE PROJECTION:
	***/ 

		if (type == PROJECTION_PERSPECTIVE) {
			double xscale = width  / fov;
			double yscale = height / fov;

			double maxscale = xscale;
			if (yscale > xscale) maxscale = yscale;

			double xangle = atan(fov/2 * maxscale/xscale);

			w = (float) (2/tan(xangle));   // 1/tan(x) == cot(x)
			h = (float) (w * width/height);
			Q = zfar/(zfar - znear);

			m._11 = w;
			m._22 = h;
			m._33 = Q;
			m._43 = -Q*znear;
			m._34 = 1;
		}

		/***
	*** ORTHOGONAL PROJECTION:
	***/

		else if (type == PROJECTION_ORTHOGONAL) {
			m._11 = (float) (fov/width);
			m._22 = (float) (fov/height);
			m._33 = (float) (1/(zfar-znear));
			m._43 = (float) (znear/(znear-zfar));
			m._44 = (float) (1);
		}

		else {
			return false;
		}

		d3ddevice->SetTransform(D3DTS_PROJECTION, &m);
		matrixProj = m;

		return true;
	}

	return false;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::SetEnvironment(Bitmap** faces)
{
	if (environment_cube && !faces) {
		delete environment_cube;
		environment_cube = 0;
		return true;
	}

	if (!environment_cube) {
		environment_cube = new(__FILE__,__LINE__) TexCubeDX9(this);
	}

	if (environment_cube) {
		bool ok = true;
		for (int i = 0; i < 6; i++)
		ok = ok && environment_cube->LoadTexture(faces[i], i);
		return ok;
	}

	return false;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::SetRenderState(RENDER_STATE state, DWORD value)
{
	if (!d3ddevice)
	return false;

	if (render_state[state] == value || d3dstate_table[state] == 0) {
		render_state[state] = value;
		return true;
	}

	HRESULT hr = E_FAIL;

	// special case for texture wrapping:
	if (state == TEXTURE_WRAP) {
		DWORD wrap = D3DTADDRESS_CLAMP;

		if (value)
		wrap = D3DTADDRESS_WRAP;

		hr = d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, wrap);
		hr = d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, wrap);
	}

	// special case for fog enable:
	else if (state == FOG_ENABLE) {
		if (value) {
			hr = d3ddevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_EXP);
			hr = d3ddevice->SetRenderState(D3DRS_FOGTABLEMODE,  D3DFOG_NONE);
		}

		hr = d3ddevice->SetRenderState(D3DRS_FOGENABLE, value);
	}

	// special case for z bias
	else if (state == Z_BIAS) {
		if (value) {
			FLOAT bias_scale = 1.0f;
			FLOAT depth_bias = (FLOAT) (DW2I(value) / -10000.0);

			hr = d3ddevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(bias_scale));
			hr = d3ddevice->SetRenderState(D3DRS_DEPTHBIAS,           F2DW(depth_bias));
		}
		else {
			hr = d3ddevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, 0);
			hr = d3ddevice->SetRenderState(D3DRS_DEPTHBIAS,           0);
		}
	}

	// set default z func along with z enable
	else if (state == Z_ENABLE) {
		hr = d3ddevice->SetRenderState(D3DRS_ZFUNC,     D3DCMP_LESSEQUAL);
		hr = d3ddevice->SetRenderState(D3DRS_ZENABLE,   value);
	}

	// all other render states:
	else {
		hr = d3ddevice->SetRenderState((D3DRENDERSTATETYPE) d3dstate_table[state], value);
	}

	if (FAILED(hr)) {
		VideoDX9Error("could not SetRenderState", hr);
		return false;
	}
	else {
		render_state[state] = value;
	}

	return true;
}

bool
VideoDX9::SetBlendType(int blend_type)
{
	if (blend_type == current_blend_state)
	return true;

	switch (blend_type) {
	default:
		// map misc blend types to SOLID
		// and fall through to that case

		blend_type = BLEND_SOLID;

	case BLEND_SOLID:
		d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE,     false);
		break;

	case BLEND_ALPHA:
		d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE,     true);
		d3ddevice->SetRenderState(D3DRS_SRCBLEND,             D3DBLEND_SRCALPHA);
		d3ddevice->SetRenderState(D3DRS_DESTBLEND,            D3DBLEND_INVSRCALPHA);
		break;

	case BLEND_ADDITIVE:
		d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE,     true);
		d3ddevice->SetRenderState(D3DRS_SRCBLEND,             D3DBLEND_ONE);
		d3ddevice->SetRenderState(D3DRS_DESTBLEND,            D3DBLEND_ONE);
		break;
	}

	current_blend_state = blend_type;
	return true;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::StartFrame()
{
	if (device_lost) {
		RecoverSurfaces();

		if (status != VIDEO_OK)
		return false;
	}

	stats.Clear();

	HRESULT err = 0;
	static int frame_number = 1;
	static int report_errs = 100;

	stats.nframe = frame_number;
	texcache->FrameNumber(frame_number++);

	// update gamma ramp for global fade value:
	SetGammaLevel(gamma);

	ClearAll();

	err = d3ddevice->BeginScene();

	if (FAILED(err)) {
		if (report_errs > 0) {
			report_errs--;
			VideoDX9Error("could not begin scene", err);
		}

		return false;
	}

	scene_active         = 1;
	current_blend_state  = -1;

	SetRenderState(LIGHTING_PASS,  0);
	SetRenderState(STENCIL_ENABLE, false);

	return true;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::EndFrame()
{
	HRESULT err = 0;

	if (scene_active) {
		err = d3ddevice->EndScene();

		if (FAILED(err)) {
			VideoDX9Error("could not end scene", err);
			return false;
		}
	}

	scene_active = 0;
	return true;
}

// +--------------------------------------------------------------------+

static DWORD ColorModulate(DWORD a, DWORD b)
{
	float a0 = (float) ((a    ) & 0xff)/255.0f;
	float a1 = (float) ((a>> 8) & 0xff)/255.0f;
	float a2 = (float) ((a>>16) & 0xff)/255.0f;
	float a3 = (float) ((a>>24) & 0xff)/255.0f;

	float b0 = (float) ((b    ) & 0xff)/255.0f;
	float b1 = (float) ((b>> 8) & 0xff)/255.0f;
	float b2 = (float) ((b>>16) & 0xff)/255.0f;
	float b3 = (float) ((b>>24) & 0xff)/255.0f;

	return (DWORD) ((BYTE)(a3*b3*255.0f) << 24) |
	((BYTE)(a2*b2*255.0f) << 16) |
	((BYTE)(a1*b1*255.0f) <<  8) |
	((BYTE)(a0*b0*255.0f));
}

// +--------------------------------------------------------------------+

bool
VideoDX9::PopulateScreenVerts(VertexSet* vset)
{
	if (!vset || !screen_vbuf)
	return false;

	num_verts = vset->nverts;

	VideoDX9ScreenVertex* v = (VideoDX9ScreenVertex*) screen_vbuf->Lock(num_verts);

	if (v) {
		first_vert = screen_vbuf->GetNextVert();

		for (int i = 0; i < num_verts; i++) {
			v->sx       = vset->s_loc[i].x;
			v->sy       = vset->s_loc[i].y;
			v->sz       = vset->s_loc[i].z;
			v->rhw      = vset->rw[i];

			v->diffuse  = vset->diffuse[i];

			v->tu       = vset->tu[i];
			v->tv       = vset->tv[i];

			v++;
		}

		screen_vbuf->Unlock();
		return true;
	}

	Print("   VideoDX9: could not lock screen vbuf for %d verts.\n", num_verts);
	return false;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::DrawPolys(int npolys, Poly* polys)
{
	bool result = false;

	if (d3ddevice && polys && npolys > 0) {
		// screen space polys:
		if (polys->vertex_set->space == VertexSet::SCREEN_SPACE)
		return DrawScreenPolys(npolys, polys);

		// world space polys:
		stats.ncalls++;

		VertexSet*           vset     = polys->vertex_set;
		int                  nverts   = vset->nverts;
		bool                 luminous = false;
		bool                 detail   = false;
		DWORD                fvf      = 0;
		void*                verts    = 0;
		int                  vsize    = 0;
		WORD*                indices  = new(__FILE__,__LINE__) WORD[npolys*6];

		if (polys->material) {
			luminous = polys->material->luminous;
		}

		if (vset->tu1 != 0) {
			VideoDX9DetailVertex* v = new(__FILE__,__LINE__) VideoDX9DetailVertex[nverts];
			verts = v;
			vsize = sizeof(VideoDX9DetailVertex);
			fvf   = VideoDX9DetailVertex::FVF;

			for (int i = 0; i < nverts; i++) {
				v->x        = vset->loc[i].x;
				v->y        = vset->loc[i].y;
				v->z        = vset->loc[i].z;

				v->diffuse  = vset->diffuse[i];
				v->specular = vset->specular[i];

				v->tu       = vset->tu[i];
				v->tv       = vset->tv[i];
				v->tu1      = vset->tu1[i];
				v->tv1      = vset->tv1[i];

				v++;
			}
		}

		else if (luminous) {
			VideoDX9LuminousVertex* v = new(__FILE__,__LINE__) VideoDX9LuminousVertex[nverts];
			verts = v;
			vsize = sizeof(VideoDX9LuminousVertex);
			fvf   = VideoDX9LuminousVertex::FVF;

			for (int i = 0; i < nverts; i++) {
				v->x        = vset->loc[i].x;
				v->y        = vset->loc[i].y;
				v->z        = vset->loc[i].z;

				v->diffuse  = vset->diffuse[i];

				v->tu       = vset->tu[i];
				v->tv       = vset->tv[i];

				v++;
			}
		}

		else {
			VideoDX9SolidVertex* v = new(__FILE__,__LINE__) VideoDX9SolidVertex[nverts];
			verts = v;
			vsize = sizeof(VideoDX9SolidVertex);
			fvf   = VideoDX9SolidVertex::FVF;

			for (int i = 0; i < nverts; i++) {
				v->x     = vset->loc[i].x;
				v->y     = vset->loc[i].y;
				v->z     = vset->loc[i].z;

				v->nx    = vset->nrm[i].x;
				v->ny    = vset->nrm[i].y;
				v->nz    = vset->nrm[i].z;

				v->tu    = vset->tu[i];
				v->tv    = vset->tv[i];

				v++;
			}
		}

		if (verts && indices) {
			HRESULT hr = E_FAIL;

			// fill index array
			int num_indices = 0;
			int num_tris    = 0;

			WORD* s = indices;
			Poly* p = polys;

			for (int i = 0; i < npolys; i++) {
				if (p->nverts == 3) {
					num_indices += 3;
					num_tris    += 1;

					*s++ = p->verts[0];
					*s++ = p->verts[1];
					*s++ = p->verts[2];
				}

				else if (p->nverts == 4) {
					num_indices += 6;
					num_tris    += 2;

					*s++ = p->verts[0];
					*s++ = p->verts[1];
					*s++ = p->verts[2];

					*s++ = p->verts[0];
					*s++ = p->verts[2];
					*s++ = p->verts[3];
				}

				p++;
			}

			hr = d3ddevice->SetTransform(D3DTS_WORLD, &identity_matrix);
			hr = d3ddevice->SetVertexShader(NULL);
			hr = d3ddevice->SetFVF(fvf);

			// send primitives to the device
			Material*            mtl      = polys->material;
			IDirect3DTexture9*   texture  = 0;

			if (mtl && texcache && mtl->tex_diffuse) {
				texture = texcache->FindTexture(mtl->tex_diffuse);
			}

			if (current_texture != texture) {
				hr = d3ddevice->SetTexture(0, texture);
				current_texture = texture;
			}

			if (mtl && texcache && mtl->tex_detail) {
				texture = texcache->FindTexture(mtl->tex_detail);
				hr = d3ddevice->SetTexture(1, texture);
			}

			if (mtl && !luminous) {
				D3DMATERIAL9 d3dmtl;
				CreateD3DMaterial(d3dmtl, *mtl);

				hr = d3ddevice->SetMaterial(&d3dmtl);
				hr = d3ddevice->SetRenderState(D3DRS_AMBIENT, ambient.Value());
			}

			// set render states and select buffers
			SetRenderState(FILL_MODE,        D3DFILL_SOLID);
			SetRenderState(LIGHTING_ENABLE,  luminous ? FALSE : TRUE);
			SetBlendType(mtl->blend);

			d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP,    D3DTOP_MODULATE);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG2,  D3DTA_DIFFUSE);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP,    D3DTOP_MODULATE);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG2,  D3DTA_DIFFUSE);

			d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP,    D3DTOP_DISABLE);


			hr = d3ddevice->DrawIndexedPrimitiveUP(
			D3DPT_TRIANGLELIST,
			0,
			nverts,
			num_tris,
			indices,
			D3DFMT_INDEX16,
			verts,
			vsize);

			if (FAILED(hr)) {
				static int report = 10;
				if (report-- > 0)
				VideoDX9Error("Could not draw 3D polys.", hr);
			}

			delete [] verts;
			delete [] indices;

			if (SUCCEEDED(hr)) {
				stats.nverts += nverts;
				stats.npolys += num_tris;
				result = true;
			}
		}
	}

	return result;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::DrawScreenPolys(int npolys, Poly* polys, int blend)
{
	bool     result = false;
	HRESULT  hr     = E_FAIL;

	if (d3ddevice && polys && npolys > 0 && screen_vbuf && screen_ibuf) {
		stats.ncalls++;

		// fill screen vertex buffer
		if (!PopulateScreenVerts(polys->vertex_set))
		return false;

		// fill screen index buffer
		int num_indices = 0;
		int num_tris    = 0;

		// count the number of indices needed for these polys
		for (int i = 0; i < npolys; i++) {
			Poly* p = polys + i;

			if (p->nverts == 3) {
				num_indices += 3;
				num_tris    += 1;
			}

			else if (p->nverts == 4) {
				num_indices += 6;
				num_tris    += 2;
			}
		}

		WORD* screen_indices = screen_ibuf->Lock(num_indices);
		int   first_index    = screen_ibuf->GetNextIndex();

		if (!screen_indices) {
			Print("   VideoDX9: could not lock screen ibuf for %d indices.\n", num_indices);
			return false;
		}

		// copy the indices into the locked index buffer
		WORD* s = screen_indices;
		Poly* p = polys;

		for (int i = 0; i < npolys; i++) {
			if (p->nverts == 3) {
				*s++ = p->verts[0] + first_vert;
				*s++ = p->verts[1] + first_vert;
				*s++ = p->verts[2] + first_vert;
			}
			else if (p->nverts == 4) {
				*s++ = p->verts[0] + first_vert;
				*s++ = p->verts[1] + first_vert;
				*s++ = p->verts[2] + first_vert;

				*s++ = p->verts[0] + first_vert;
				*s++ = p->verts[2] + first_vert;
				*s++ = p->verts[3] + first_vert;
			}

			p++;
		}

		screen_ibuf->Unlock();

		// set render states and select buffers
		SetRenderState(FILL_MODE,        D3DFILL_SOLID);
		SetRenderState(Z_ENABLE,         D3DZB_FALSE);
		SetRenderState(Z_WRITE_ENABLE,   D3DZB_FALSE);
		SetRenderState(LIGHTING_ENABLE,  FALSE);
		SetBlendType(blend);

		d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP,    D3DTOP_MODULATE);
		d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
		d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG2,  D3DTA_DIFFUSE);
		d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP,    D3DTOP_MODULATE);
		d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,  D3DTA_TEXTURE);
		d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG2,  D3DTA_DIFFUSE);

		d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP,    D3DTOP_DISABLE);

		// send primitives to the device
		Material*            mtl      = polys->material;
		IDirect3DTexture9*   texture  = 0;

		if (mtl && texcache && mtl->tex_diffuse) {
			texture = texcache->FindTexture(mtl->tex_diffuse);
		}

		if (current_texture != texture) {
			hr = d3ddevice->SetTexture(0, texture);
			current_texture = texture;
		}

		screen_vbuf->Select(0);
		screen_ibuf->Select();
		hr = d3ddevice->SetVertexShader(NULL);
		hr = d3ddevice->SetFVF(VideoDX9ScreenVertex::FVF);

		hr = d3ddevice->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		first_vert,
		num_verts,
		first_index,
		num_tris);

		if (FAILED(hr)) {
			static int report = 10;
			if (report-- > 0)
			VideoDX9Error("Could not draw screen polys.", hr);
		}
		else {
			stats.nverts += num_verts;
			stats.npolys += num_tris;
			result = true;
		}
	}

	return result;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::DrawSolid(Solid* s, DWORD blend_modes)
{
	bool     result = false;
	HRESULT  hr     = E_FAIL;

	if (d3ddevice && s && s->GetModel()) {
		Model* model   = s->GetModel();
		Matrix orient  = s->Orientation();
		orient.Transpose();

		D3DMATRIX world_matrix;
		CreateD3DMatrix(world_matrix, orient, s->Location());
		d3ddevice->SetTransform(D3DTS_WORLD, &world_matrix);
		matrixWorld = world_matrix;
		D3DXMatrixInverse(&matrixWorldInverse, 0, &matrixWorld);

		ListIter<Surface> surf_iter = model->GetSurfaces();
		while (++surf_iter) {
			Surface* surf = surf_iter.value();

			if (surf->IsHidden() || surf->IsSimplified())
			continue;

			if (PrepareSurface(surf)) {
				result = true;

				VideoDX9SurfaceData* surf_data = (VideoDX9SurfaceData*) surf->GetVideoPrivateData();
				surf_data->vertex_buffer->Select(0);
				surf_data->index_buffer->Select();

				ListIter<Segment> seg_iter = surf->GetSegments();
				while (++seg_iter) {
					Segment*    segment  = seg_iter.value();
					Material*   mtl      = segment->material;

					if (mtl && (blend_modes & mtl->blend)) {
						result = result && DrawSegment(segment);
					}
				}
			}
		}
	}

	surface_has_tangent_data = false;

	return result;
}

bool
VideoDX9::DrawSegment(Segment* segment)
{
	bool    result    = false;
	bool    detail    = false;
	bool    luminous  = false;
	HRESULT hr        = E_FAIL;

	if (segment && segment->video_data) {
		stats.ncalls++;

		VideoDX9SegmentData* seg_data = (VideoDX9SegmentData*) segment->video_data;
		int   first_vert     = seg_data->first_vert;
		int   num_verts      = seg_data->num_verts;
		int   first_index    = seg_data->first_index;
		int   num_tris       = seg_data->num_tris;

		if (segment->model)
		luminous = segment->model->IsLuminous();

		// set render states and select buffers
		d3ddevice->SetRenderState(D3DRS_AMBIENT, ambient.Value());

		Material* mtl = segment->material;

		if (use_material)
		mtl = use_material;

		if (segment->polys && segment->polys->vertex_set && segment->polys->vertex_set->tu1)
		detail = true;

		// send primitives to the device
		if (detail) {
			hr = d3ddevice->SetVertexShader(NULL);
			hr = d3ddevice->SetFVF(VideoDX9DetailVertex::FVF);
		}

		else if (luminous) {
			hr = d3ddevice->SetVertexShader(NULL);
			hr = d3ddevice->SetFVF(VideoDX9LuminousVertex::FVF);
		}

		else if (surface_has_tangent_data && vertex_declaration) {
			hr = d3ddevice->SetVertexDeclaration(vertex_declaration);
			hr = d3ddevice->SetVertexShader(NULL);
		}

		else {
			hr = d3ddevice->SetVertexShader(NULL);
			hr = d3ddevice->SetFVF(VideoDX9SolidVertex::FVF);
		}

		if (render_state[FILL_MODE] == FILL_WIREFRAME) {
			PrepareMaterial(mtl);
			SetupPass(0);

			for (int i = 0; i < segment->npolys; i++) {
				DrawPolyOutline(segment->polys + i);
			}
		}

		else if (luminous) {
			PrepareMaterial(mtl);

			SetRenderState(FILL_MODE,        D3DFILL_SOLID);
			SetRenderState(LIGHTING_ENABLE,  FALSE);
			SetBlendType(mtl->blend);

			for (int pass = 0; pass < passes; pass++) {
				SetupPass(pass);

				hr = d3ddevice->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST,
				0,
				first_vert,
				num_verts,
				first_index,
				num_tris);
			}

			d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		}

		else {
			PrepareMaterial(mtl);

			if (strategy == DX9_STRATEGY_GLOW && render_state[LIGHTING_PASS] > 0) {
				hr = 0;
			}

			else if (magic_fx && strategy < DX9_STRATEGY_BLEND && !detail) {
				DWORD vs_version     = 0;
				DWORD ps_version     = 0;
				bool  shaders_ok     = false;

				VideoDX9DeviceInfo* dev_info = dx9enum->GetDeviceInfo(video_settings.GetDeviceType());

				if (dev_info) {
					vs_version = video_settings.enable_vs ? dev_info->caps.VertexShaderVersion : 0;
					ps_version = video_settings.enable_ps ? dev_info->caps.PixelShaderVersion  : 0;

					if (vs_version >= D3DVS_VERSION(1,1))
					shaders_ok = true;
				}

				if (surface_has_tangent_data && vertex_declaration) {
					hr = d3ddevice->SetVertexShader(NULL);
					hr = d3ddevice->SetVertexDeclaration(vertex_declaration);
				}

				else {
					hr = d3ddevice->SetVertexShader(NULL);
					hr = d3ddevice->SetFVF(VideoDX9SolidVertex::FVF);
				}

				bool would_bump = !IsBumpMapEnabled() && mtl->bump > 0.5;

				bool will_bump  = IsBumpMapEnabled() &&	surface_has_tangent_data &&	shaders_ok && camera;

				bool do_pix     = will_bump && render_state[LIGHTING_PASS] == 0;
				bool do_bump    = will_bump && render_state[LIGHTING_PASS] >  0;

				D3DXMATRIX matrixWVP = matrixWorld * matrixView * matrixProj;

				D3DXVECTOR4 eyePos( (float) camera->Pos().x, (float) camera->Pos().y, (float) camera->Pos().z,	1.0f);

				D3DXVECTOR4 eyeObj;
				D3DXVec4Transform(&eyeObj, &eyePos, &matrixWorldInverse);

				D3DXVECTOR4 lightPos(100.0f, 100.0f, 100.0f, 1.0f);
				D3DXVECTOR4 lightColor(1,1,1,1);
				D3DXVECTOR4 ambientColor(0,0,0,1);

				ambientColor.x = ambient.fRed();
				ambientColor.y = ambient.fGreen();
				ambientColor.z = ambient.fBlue();

				if (main_light && (render_state[LIGHTING_PASS] > 0 || mtl->blend > Material::MTL_SOLID)) {
					lightPos.x = (float) main_light->Location().x;
					lightPos.y = (float) main_light->Location().y;
					lightPos.z = (float) main_light->Location().z;

					if (mtl->tex_bumpmap && do_bump)
						D3DXVec4Transform(&lightPos, &lightPos, &matrixWorldInverse);

					lightColor.x = main_light->GetColor().fRed()   * main_light->Intensity();
					lightColor.y = main_light->GetColor().fGreen() * main_light->Intensity();
					lightColor.z = main_light->GetColor().fBlue()  * main_light->Intensity();
					lightColor.w = 1.0f;
				}

				else if (back_light && render_state[LIGHTING_PASS] == 0) {
					lightPos.x = (float) back_light->Location().x;
					lightPos.y = (float) back_light->Location().y;
					lightPos.z = (float) back_light->Location().z;

					lightColor.x = back_light->GetColor().fRed()   * back_light->Intensity();
					lightColor.y = back_light->GetColor().fGreen() * back_light->Intensity();
					lightColor.z = back_light->GetColor().fBlue()  * back_light->Intensity();
					lightColor.w = 1.0f;
				}

				D3DXVECTOR4 lightDir = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f) - lightPos;
				D3DXVec4Normalize(&lightDir, &lightDir);

				magic_fx->SetMatrix("wvp",          &matrixWVP);
				magic_fx->SetMatrix("world",        &matrixWorld);
				magic_fx->SetMatrix("view",         &matrixView);
				magic_fx->SetMatrix("proj",         &matrixProj);
				magic_fx->SetMatrix("worldInv",     &matrixWorldInverse);

				magic_fx->SetVector("light1Pos",    &lightPos);
				magic_fx->SetVector("light1Dir",    &lightDir);
				magic_fx->SetVector("light1Color",  &lightColor);
				magic_fx->SetVector("ambientColor", &ambientColor);
				magic_fx->SetVector("eyeObj",       &eyeObj);

				FLOAT base_bias = (FLOAT) (DW2I(render_state[Z_BIAS]) / -10000.0);
				magic_fx->SetFloat("bias", base_bias + video_settings.depth_bias);

				ColorValue orig_ks = mtl->Ks;

				if (would_bump && mtl->specular_value >= 0.5)
					mtl->Ks = mtl->Ks * 0.3;

				magic_fx->SetValue("Ka", &mtl->Ka, sizeof(ColorValue));
				magic_fx->SetValue("Kd", &mtl->Kd, sizeof(ColorValue));
				magic_fx->SetValue("Ke", &mtl->Ke, sizeof(ColorValue));
				magic_fx->SetValue("Ks", &mtl->Ks, sizeof(ColorValue));
				magic_fx->SetFloat("Ns", mtl->power);

				if (would_bump && mtl->specular_value >= 0.5)
					mtl->Ks = orig_ks;

				if (mtl->tex_diffuse) {
					IDirect3DTexture9* texture = texcache->FindTexture(mtl->tex_diffuse);
					magic_fx->SetTexture("tex_d", texture);
				} else {
					magic_fx->SetTexture("tex_d", 0);
				}

				if (mtl->tex_emissive) {
					IDirect3DTexture9* texture = texcache->FindTexture(mtl->tex_emissive);
					magic_fx->SetTexture("tex_e", texture);
				} else {
					magic_fx->SetTexture("tex_e", 0);
				}

				if (mtl->tex_specular && IsSpecMapEnabled()) {
					IDirect3DTexture9* texture = texcache->FindTexture(mtl->tex_specular);
					magic_fx->SetTexture("tex_s", texture);
				} else {
					magic_fx->SetTexture("tex_s", 0);
				}

				if (mtl->tex_bumpmap && do_bump) {
					IDirect3DTexture9* texture = texcache->FindNormalMap(mtl->tex_bumpmap, mtl->bump);
					magic_fx->SetTexture("tex_n", texture);
					magic_fx->SetFloat("offsetAmp", mtl->bump / mtl->tex_bumpmap->Height());
				} else if (mtl->tex_bumpmap && !do_bump) {
					IDirect3DTexture9* texture = texcache->FindTexture(mtl->tex_bumpmap);
					magic_fx->SetTexture("tex_x", texture);
				} else {
					magic_fx->SetTexture("tex_x", 0);
				}

				const char* mtl_shader = mtl->GetShader(render_state[LIGHTING_PASS]);
				D3DXHANDLE  hnd_shader = 0;

				if (mtl_shader) {
					if (!strcmp(mtl_shader, "null"))
						return true;

					hnd_shader = magic_fx->GetTechniqueByName(mtl_shader);
				}

				if (hnd_shader) {
					hr = magic_fx->SetTechnique(hnd_shader);
				} else {
					if (will_bump) {
						if (mtl->tex_specular && IsSpecMapEnabled()) {
							if (mtl->tex_bumpmap && do_bump) {
								if (ps_version >= D3DPS_VERSION(2,0))
									hr = magic_fx->SetTechnique("BumpSpecMapPix");
								else
									hr = magic_fx->SetTechnique("BumpSpecMap");
							} else if (mtl->tex_emissive && render_state[LIGHTING_PASS] == 0) {
								if (ps_version >= D3DPS_VERSION(2,0))
									hr = magic_fx->SetTechnique("EmissiveSpecMapPix");
								else
									hr = magic_fx->SetTechnique("EmissiveSpecularTexture");
							} else {
								if (ps_version >= D3DPS_VERSION(2,0))
								hr = magic_fx->SetTechnique("SpecMapPix");
								else
								hr = magic_fx->SetTechnique("SpecularTexture");
							}
						} else {
							if (mtl->tex_bumpmap && do_bump) {
								if (ps_version >= D3DPS_VERSION(2,0))
									hr = magic_fx->SetTechnique("BumpMapPix");
								else
									hr = magic_fx->SetTechnique("BumpMap");
							} else if (mtl->tex_emissive && render_state[LIGHTING_PASS] == 0) {
								if (ps_version >= D3DPS_VERSION(2,0))
									hr = magic_fx->SetTechnique("EmissivePix");
								else
									hr = magic_fx->SetTechnique("EmissiveTexture");
							} else {
								if (ps_version >= D3DPS_VERSION(2,0))
									hr = magic_fx->SetTechnique("SimplePix");
								else
									hr = magic_fx->SetTechnique("SimpleTexture");
							}
						}
					}

					else if (texcache && mtl->tex_diffuse) {
						if (mtl->tex_specular && IsSpecMapEnabled()) {
							if (mtl->tex_emissive && render_state[LIGHTING_PASS] == 0) {
								hr = magic_fx->SetTechnique("EmissiveSpecularTexture");
							} else {
								hr = magic_fx->SetTechnique("SpecularTexture");
							}
						}

						else {
							if (mtl->tex_emissive && render_state[LIGHTING_PASS] == 0) {
								hr = magic_fx->SetTechnique("EmissiveTexture");
							} else {
								hr = magic_fx->SetTechnique("SimpleTexture");
							}
						}
					} else {
						hr = magic_fx->SetTechnique("SimpleMaterial");
					}
				}

				if (environment_cube != 0 && magic_fx->IsParameterUsed("env_cube", hnd_shader)) {
					D3DXMATRIX env_matrix;
					D3DXMatrixIdentity(&env_matrix);

					magic_fx->SetMatrix("env_matrix", &env_matrix);
					magic_fx->SetTexture("env_cube", environment_cube->GetTexture());
				}

				if (render_state[STENCIL_ENABLE]) {
					d3ddevice->SetRenderState(D3DRS_STENCILENABLE,  TRUE);
					d3ddevice->SetRenderState(D3DRS_STENCILREF,     0x01);
					d3ddevice->SetRenderState(D3DRS_STENCILFUNC,    D3DCMP_GREATER);
				} else {
					d3ddevice->SetRenderState(D3DRS_STENCILENABLE,  FALSE);
				}

				if (render_state[LIGHTING_PASS] > 0) {
					current_blend_state = 100;
					SetBlendType(BLEND_ADDITIVE);
					SetRenderState(Z_WRITE_ENABLE, FALSE);
				} else {
					current_blend_state = 100;
					SetBlendType(mtl->blend);
				}

				UINT nPasses = 0;

				hr = magic_fx->Begin(&nPasses, 0);

				for (UINT i = 0; i < nPasses; i++) {
					hr = magic_fx->BeginPass(i);

					hr = d3ddevice->DrawIndexedPrimitive(
					D3DPT_TRIANGLELIST,
					0,
					first_vert,
					num_verts,
					first_index,
					num_tris);

					hr = magic_fx->EndPass();
				}

				hr = magic_fx->End();
			} else {
				for (int pass = 0; pass < passes; pass++) {
					SetupPass(pass);

					hr = d3ddevice->DrawIndexedPrimitive(
					D3DPT_TRIANGLELIST,
					0,
					first_vert,
					num_verts,
					first_index,
					num_tris);

					if (detail) {
						hr = d3ddevice->SetVertexShader(NULL);
						hr = d3ddevice->SetFVF(VideoDX9DetailVertex::FVF);
					} else if (luminous) {
						hr = d3ddevice->SetVertexShader(NULL);
						hr = d3ddevice->SetFVF(VideoDX9LuminousVertex::FVF);
					} else if (surface_has_tangent_data && vertex_declaration) {
						hr = d3ddevice->SetVertexShader(NULL);
						hr = d3ddevice->SetVertexDeclaration(vertex_declaration);
					} else {
						hr = d3ddevice->SetVertexShader(NULL);
						hr = d3ddevice->SetFVF(VideoDX9SolidVertex::FVF);
					}
				}
			}
		}

		if (FAILED(hr)) {
			static int report = 10;
			if (report-- > 0)
				VideoDX9Error("Could not draw solid polys.", hr);
		} else {
			stats.nverts += num_verts;
			stats.npolys += num_tris;
			result = true;
		}
	}

	return result;
}

bool
VideoDX9::DrawPolyOutline(Poly* p)
{
	if (d3ddevice && p && p->nverts >= 3) {
		static VideoDX9LineVertex  verts[8];

		int         nlines = p->nverts;
		VertexSet*  vset   = p->vertex_set;
		WORD        index  = 0;
		Color       color  = Color::Black;
		HRESULT     hr     = E_FAIL;

		ZeroMemory(verts, sizeof(verts));

		if (p->material)
		color = p->material->Kd.ToColor();

		for (int i = 0; i < p->nverts; i++) {
			index = p->verts[i];

			verts[i].x        = vset->loc[index].x;
			verts[i].y        = vset->loc[index].y;
			verts[i].z        = vset->loc[index].z;
			verts[i].diffuse  = color.Value();
		}

		// last vertex, to close the loop
		index = p->verts[0];
		int i = p->nverts;

		verts[i].x        = vset->loc[index].x;
		verts[i].y        = vset->loc[index].y;
		verts[i].z        = vset->loc[index].z;
		verts[i].diffuse  = color.Value();

		current_texture = 0;

		hr = d3ddevice->SetVertexShader(NULL);
		hr = d3ddevice->SetFVF(VideoDX9LineVertex::FVF);
		hr = d3ddevice->SetTexture(0, 0);
		hr = d3ddevice->DrawPrimitiveUP(
		D3DPT_LINESTRIP,
		nlines,
		verts,
		sizeof(VideoDX9LineVertex));
		return true;
	}

	return false;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::DrawShadow(Solid* s, int nverts, Vec3* shadow_verts, bool visible)
{
	bool     result = false;
	HRESULT  hr     = E_FAIL;

	if (d3ddevice && s && nverts && shadow_verts && IsShadowEnabled()) {
		Matrix orient  = s->Orientation();
		orient.Transpose();

		D3DMATRIX world_matrix;
		CreateD3DMatrix(world_matrix, orient, s->Location());
		d3ddevice->SetTransform(D3DTS_WORLD, &world_matrix);
		matrixWorld = world_matrix;

		// show shadow outlines:
		if (visible) {
			static VideoDX9LineVertex  verts[4];

			d3ddevice->SetRenderState(D3DRS_ZENABLE,        TRUE);
			d3ddevice->SetRenderState(D3DRS_ZWRITEENABLE,   FALSE);
			d3ddevice->SetRenderState(D3DRS_ZFUNC,          D3DCMP_LESSEQUAL);
			d3ddevice->SetRenderState(D3DRS_SHADEMODE,      D3DSHADE_FLAT);
			d3ddevice->SetRenderState(D3DRS_LIGHTING,       FALSE);
			d3ddevice->SetVertexShader(NULL);
			d3ddevice->SetFVF(VideoDX9LineVertex::FVF);
			d3ddevice->SetTexture(0, 0);

			SetBlendType(BLEND_ALPHA);

			for (int i = 0; i < nverts; i+=3) {
				DWORD c = 0xa0ffff80;

				verts[0].x = shadow_verts[i+0].x;
				verts[0].y = shadow_verts[i+0].y;
				verts[0].z = shadow_verts[i+0].z;
				verts[0].diffuse = c;

				verts[1].x = shadow_verts[i+1].x;
				verts[1].y = shadow_verts[i+1].y;
				verts[1].z = shadow_verts[i+1].z;
				verts[1].diffuse = c;

				verts[2].x = shadow_verts[i+2].x;
				verts[2].y = shadow_verts[i+2].y;
				verts[2].z = shadow_verts[i+2].z;
				verts[2].diffuse = c;

				verts[3].x = shadow_verts[i+0].x;
				verts[3].y = shadow_verts[i+0].y;
				verts[3].z = shadow_verts[i+0].z;
				verts[3].diffuse = c;

				hr = d3ddevice->DrawPrimitiveUP(
				D3DPT_LINESTRIP,
				3,
				verts,
				sizeof(VideoDX9LineVertex));
			}

			// restore lighting state
			d3ddevice->SetRenderState(D3DRS_LIGHTING,    render_state[LIGHTING_ENABLE]);
		}

		// render shadows into stencil buffer:

		// Disable z-buffer writes (note: z-testing still occurs), and enable the
		// stencil-buffer
		d3ddevice->SetRenderState(D3DRS_ZENABLE,       TRUE);
		d3ddevice->SetRenderState(D3DRS_ZWRITEENABLE,  FALSE);
		d3ddevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);

		// Dont bother with interpolating color
		d3ddevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

		// Set up stencil compare fuction, reference value, and masks.
		// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
		// Note: since we set up the stencil-test to always pass, the STENCILFAIL
		// renderstate is really not needed.
		d3ddevice->SetRenderState(D3DRS_STENCILFUNC,  D3DCMP_ALWAYS );
		d3ddevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
		d3ddevice->SetRenderState(D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );

		// If z-test passes, inc/decrement stencil buffer value
		d3ddevice->SetRenderState(D3DRS_STENCILREF,       0x1 );
		d3ddevice->SetRenderState(D3DRS_STENCILMASK,      0xff );
		d3ddevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xff );
		d3ddevice->SetRenderState(D3DRS_STENCILPASS,      D3DSTENCILOP_INCR );

		// Make sure that no pixels get drawn to the frame buffer
		d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
		d3ddevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ZERO );
		d3ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE );

		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetFVF(D3DFVF_XYZ);

		// Draw front-side of shadow volume in stencil/z only
		hr = d3ddevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nverts/3, shadow_verts, sizeof(Vec3));

		// Now reverse cull order so back sides of shadow volume are written.
		d3ddevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW );

		// Decrement stencil buffer value
		d3ddevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR );

		// Draw back-side of shadow volume in stencil/z only
		hr = d3ddevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nverts/3, shadow_verts, sizeof(Vec3));

		// restore render states
		d3ddevice->SetRenderState(D3DRS_SHADEMODE,         D3DSHADE_GOURAUD);
		d3ddevice->SetRenderState(D3DRS_CULLMODE,          D3DCULL_CCW);
		d3ddevice->SetRenderState(D3DRS_ZWRITEENABLE,      TRUE);
		d3ddevice->SetRenderState(D3DRS_STENCILENABLE,     FALSE);

		// force restore of current blend type
		int type = current_blend_state;
		current_blend_state = 100;
		SetBlendType(type);

		result = SUCCEEDED(hr);
	}

	return result;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::DrawLines(int nlines, Vec3* points, Color c, int blend)
{
	bool result = false;

	if (d3ddevice && points && nlines > 0 && nlines <= 256) {
		stats.ncalls++;

		VideoDX9LineVertex* verts = line_verts;

		if (verts) {
			HRESULT hr = E_FAIL;

			for (int i = 0; i < 2*nlines; i++) {
				VideoDX9LineVertex*  v = verts + i;
				Vec3*                p = points + i;

				v->x        = p->x;
				v->y        = p->y;
				v->z        = p->z;
				v->diffuse  = c.Value();
			}

			hr = d3ddevice->SetTransform(D3DTS_WORLD, &identity_matrix);
			hr = d3ddevice->SetVertexShader(NULL);
			hr = d3ddevice->SetFVF(VideoDX9LineVertex::FVF);

			DWORD old_lighting = render_state[LIGHTING_ENABLE];

			// untextured lines:
			if (current_texture) {
				d3ddevice->SetTexture(0, 0);
				current_texture = 0;
			}

			SetRenderState(LIGHTING_ENABLE,  FALSE);
			SetBlendType(blend);

			hr = d3ddevice->DrawPrimitiveUP(
			D3DPT_LINELIST,
			nlines,
			verts,
			sizeof(VideoDX9LineVertex));

			if (FAILED(hr)) {
				static int report = 10;
				if (report-- > 0)
				VideoDX9Error("Could not draw 3D lines.", hr);
			}

			SetRenderState(LIGHTING_ENABLE,  old_lighting);

			if (SUCCEEDED(hr)) {
				stats.nverts += 2*nlines;
				stats.nlines += nlines;
				result = true;
			}
		}
	}

	return result;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::DrawScreenLines(int nlines, float* points, Color c, int blend)
{
	bool result = false;

	if (d3ddevice && points && nlines > 0 && nlines <= 64) {
		stats.ncalls++;

		VideoDX9ScreenVertex* verts = screen_line_verts;

		if (verts) {
			HRESULT hr = E_FAIL;

			for (int i = 0; i < 2*nlines; i++) {
				VideoDX9ScreenVertex* v = verts + i;

				v->sx       = points[2*i + 0];
				v->sy       = points[2*i + 1];
				v->sz       = 0.0f;
				v->rhw      = 1.0f;
				v->diffuse  = c.Value();
				v->tu       = 0.0f;
				v->tv       = 0.0f;
			}

			hr = d3ddevice->SetVertexShader(NULL);
			hr = d3ddevice->SetFVF(VideoDX9ScreenVertex::FVF);

			if (FAILED(hr)) {
				static int report = 10;
				if (report-- > 0)
				VideoDX9Error("Could not set FVF for screen lines.", hr);
			}

			else {
				if (current_texture != 0) {
					hr = d3ddevice->SetTexture(0, 0);
					current_texture = 0;
				}

				SetRenderState(FILL_MODE,        D3DFILL_SOLID);
				SetRenderState(Z_ENABLE,         D3DZB_FALSE);
				SetRenderState(LIGHTING_ENABLE,  FALSE);
				SetBlendType(blend);

				hr = d3ddevice->DrawPrimitiveUP(
				D3DPT_LINELIST,
				nlines,
				verts,
				sizeof(VideoDX9ScreenVertex));

				if (FAILED(hr)) {
					static int report = 10;
					if (report-- > 0)
					VideoDX9Error("Could not draw screen lines.", hr);
				}
			}

			if (SUCCEEDED(hr)) {
				stats.nverts += 2*nlines;
				stats.nlines += nlines;
				result = true;
			}
		}
	}

	return result;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::DrawPoints(VertexSet* vset)
{
	if (vset && vset->nverts) {
		HRESULT hr = E_FAIL;

		int                  nverts = vset->nverts;
		VideoDX9LineVertex*  verts  = new(__FILE__,__LINE__) VideoDX9LineVertex[nverts];

		if (verts) {
			for (int i = 0; i < nverts; i++) {
				VideoDX9LineVertex*  v = verts + i;
				Vec3*                p = vset->loc + i;

				v->x        = p->x;
				v->y        = p->y;
				v->z        = p->z;
				v->diffuse  = vset->diffuse[i];
			}

			SetRenderState(LIGHTING_ENABLE,  FALSE);

			hr = d3ddevice->SetTransform(D3DTS_WORLD, &identity_matrix);
			hr = d3ddevice->SetVertexShader(NULL);
			hr = d3ddevice->SetFVF(VideoDX9LineVertex::FVF);
			hr = d3ddevice->SetTexture(0, 0);
			hr = d3ddevice->DrawPrimitiveUP(
			D3DPT_POINTLIST,
			nverts,
			verts,
			sizeof(VideoDX9LineVertex));

			delete [] verts;
			return true;
		}
	}

	return false;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::UseMaterial(Material* m)
{
	use_material = m;
	return true;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::UseXFont(const char* name, int size, bool bold, bool ital)
{
	if (d3ddevice && name && *name && size > 4) {
		RELEASE(d3dx_font);

		strcpy_s(font_name, name);
		font_size = size;
		font_bold = bold;
		font_ital = ital;

		HRESULT  hr  = E_FAIL;
		HDC      hdc = GetDC(NULL);
		int      nLogPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);

		ReleaseDC(NULL, hdc);

		int nHeight = -size * nLogPixelsY / 72;

		hr = D3DXCreateFont(d3ddevice,                  // D3D device
		nHeight,                     // Height
		0,                           // Width
		bold ? FW_BOLD : FW_NORMAL,  // Weight
		1,                           // MipLevels, 0 = autogen mipmaps
		ital,                        // Italic
		DEFAULT_CHARSET,             // CharSet
		OUT_DEFAULT_PRECIS,          // OutputPrecision
		DEFAULT_QUALITY,             // Quality
		DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
		name,                        // pFaceName
		&d3dx_font);                 // ppFont

		if (SUCCEEDED(hr)) {
			return true;
		}
	}

	RELEASE(d3dx_font);
	return false;
}

bool
VideoDX9::DrawText(const char* text, int count, const Rect& rect, DWORD format, Color c)
{
	if (d3ddevice && text && *text && d3dx_font) {
		RECT r;
		r.left   = rect.x;
		r.top    = rect.y;
		r.right  = rect.x + rect.w;
		r.bottom = rect.y + rect.h;

		d3dx_font->DrawText(0, text, count, &r, format, c.Value());
	}

	return false;
}

// +--------------------------------------------------------------------+

bool
VideoDX9::PrepareSurface(Surface* surf)
{
	if (surf) {
		int   nverts      = surf->NumVerts();
		int   nindices    = surf->NumIndices();
		bool  detail      = surf->GetVertexSet()->tu1 != 0;
		bool  luminous    = false;
		DWORD dynamic     = 0;

		if (surf->GetModel()) {
			luminous = surf->GetModel()->IsLuminous();
			dynamic  = surf->GetModel()->IsDynamic() ? D3DUSAGE_DYNAMIC : 0;
		}

		surface_has_tangent_data = !luminous && (surf->GetVertexSet()->tangent && surf->GetVertexSet()->binormal);

		VideoDX9SurfaceData* surf_data = (VideoDX9SurfaceData*) surf->GetVideoPrivateData();

		if (!surf_data) {
			surf_data   = new(__FILE__,__LINE__) VideoDX9SurfaceData(surf->GetModel());

			surface_has_tangent_data = false;

			if (surf->GetVertexSet()->tangent && surf->GetVertexSet()->binormal) {
				surface_has_tangent_data = true;
				surf_data->vertex_buffer = new(__FILE__,__LINE__) VideoDX9VertexBuffer(
				this,
				nverts,
				sizeof(VideoDX9NormalVertex),
				0, // not an FVF vertex buffer
				dynamic | D3DUSAGE_WRITEONLY);
			}

			else if (detail) {
				surf_data->vertex_buffer = new(__FILE__,__LINE__) VideoDX9VertexBuffer(
				this,
				nverts,
				sizeof(VideoDX9DetailVertex),
				VideoDX9DetailVertex::FVF,
				dynamic | D3DUSAGE_WRITEONLY);
			}

			else if (luminous) {
				surf_data->vertex_buffer = new(__FILE__,__LINE__) VideoDX9VertexBuffer(
				this,
				nverts,
				sizeof(VideoDX9LuminousVertex),
				VideoDX9LuminousVertex::FVF,
				dynamic | D3DUSAGE_WRITEONLY);
			}

			else {
				surf_data->vertex_buffer = new(__FILE__,__LINE__) VideoDX9VertexBuffer(
				this,
				nverts,
				sizeof(VideoDX9SolidVertex),
				VideoDX9SolidVertex::FVF,
				dynamic | D3DUSAGE_WRITEONLY);
			}

			surf_data->index_buffer = new(__FILE__,__LINE__) VideoDX9IndexBuffer(
			this,
			nindices,
			dynamic | D3DUSAGE_WRITEONLY);

			if (!surf_data->vertex_buffer || !surf_data->index_buffer) {
				Print("VideoDX9: Unable to prepare surface '%s'\n", surf->Name());
				delete surf_data;
				return false;
			}

			surf->SetVideoPrivateData(surf_data);
		}

		if (surf_data && !surf_data->IsValid()) {
			if (detail) {
				VideoDX9DetailVertex* v = (VideoDX9DetailVertex*) surf_data->vertex_buffer->Lock(nverts);

				if (v) {
					const VertexSet* vset = surf->GetVertexSet();
					for (int i = 0; i < nverts; i++) {
						v->x        = vset->loc[i].x;
						v->y        = vset->loc[i].y;
						v->z        = vset->loc[i].z;

						v->diffuse  = vset->diffuse[i];
						v->specular = vset->specular[i];

						v->tu       = vset->tu[i];
						v->tv       = vset->tv[i];
						v->tu1      = vset->tu1[i];
						v->tv1      = vset->tv1[i];

						v++;
					}

					surf_data->vertex_buffer->Unlock();
				}
			}

			else if (luminous) {
				VideoDX9LuminousVertex* v = (VideoDX9LuminousVertex*) surf_data->vertex_buffer->Lock(nverts);

				if (v) {
					const VertexSet* vset = surf->GetVertexSet();
					for (int i = 0; i < nverts; i++) {
						v->x        = vset->loc[i].x;
						v->y        = vset->loc[i].y;
						v->z        = vset->loc[i].z;

						v->diffuse  = vset->diffuse[i];

						v->tu       = vset->tu[i];
						v->tv       = vset->tv[i];

						v++;
					}

					surf_data->vertex_buffer->Unlock();
				}
			}

			else if (surface_has_tangent_data) {
				VideoDX9NormalVertex* v = (VideoDX9NormalVertex*) surf_data->vertex_buffer->Lock(nverts);

				if (v) {
					const VertexSet* vset = surf->GetVertexSet();
					for (int i = 0; i < nverts; i++) {
						v->x        = vset->loc[i].x;
						v->y        = vset->loc[i].y;
						v->z        = vset->loc[i].z;

						v->nx       = vset->nrm[i].x;
						v->ny       = vset->nrm[i].y;
						v->nz       = vset->nrm[i].z;

						v->t0u      = vset->tu[i];
						v->t0v      = vset->tv[i];
						v->t1u      = vset->tu[i];
						v->t1v      = vset->tv[i];

						v->tx       = vset->tangent[i].x;
						v->ty       = vset->tangent[i].y;
						v->tz       = vset->tangent[i].z;

						v->bx       = vset->binormal[i].x;
						v->by       = vset->binormal[i].y;
						v->bz       = vset->binormal[i].z;

						v++;
					}

					surf_data->vertex_buffer->Unlock();
				}
			}

			else {
				VideoDX9SolidVertex* v = (VideoDX9SolidVertex*) surf_data->vertex_buffer->Lock(nverts);

				if (v) {
					const VertexSet* vset = surf->GetVertexSet();
					for (int i = 0; i < nverts; i++) {
						v->x        = vset->loc[i].x;
						v->y        = vset->loc[i].y;
						v->z        = vset->loc[i].z;

						v->nx       = vset->nrm[i].x;
						v->ny       = vset->nrm[i].y;
						v->nz       = vset->nrm[i].z;

						v->tu       = vset->tu[i];
						v->tv       = vset->tv[i];

						v++;
					}

					surf_data->vertex_buffer->Unlock();
				}
			}

			WORD* indices = surf_data->index_buffer->Lock(nindices);

			if (indices) {
				// copy the indices into the locked index buffer
				WORD* s = indices;
				Poly* p = surf->GetPolys();

				for (int i = 0; i < surf->NumPolys(); i++) {
					if (p->nverts == 3) {
						*s++ = p->verts[0];
						*s++ = p->verts[2];
						*s++ = p->verts[1];
					}
					else if (p->nverts == 4) {
						*s++ = p->verts[0];
						*s++ = p->verts[2];
						*s++ = p->verts[1];

						*s++ = p->verts[0];
						*s++ = p->verts[3];
						*s++ = p->verts[2];
					}

					p++;
				}

				surf_data->index_buffer->Unlock();
			}

			surf_data->Validate();
		}

		int first_index = 0;

		ListIter<Segment> seg_iter = surf->GetSegments();
		while (++seg_iter) {
			Segment* segment = seg_iter.value();

			if (!segment->video_data) {
				VideoDX9SegmentData* seg_data = new(__FILE__,__LINE__) VideoDX9SegmentData;

				int num_tris = 0;
				for (int i = 0; i < segment->npolys; i++)
				num_tris += segment->polys[i].nverts-2;

				seg_data->first_vert    = 0;
				seg_data->num_verts     = surf->NumVerts();
				seg_data->first_index   = first_index;
				seg_data->num_tris      = num_tris;

				segment->video_data = seg_data;

				first_index += num_tris * 3;
			}
		}
	}

	return true;
}

// +--------------------------------------------------------------------+

int
VideoDX9::PrepareMaterial(Material* m)
{
	segment_material  = m;
	strategy          = 0;
	passes            = 0;

	if (m) {
		int   max_stages     = 1;
		int   max_textures   = 1;
		bool  multiply_add   = false;
		bool  dotproduct3    = false;
		bool  vertexshader   = false;
		bool  pixelshader    = false;
		DWORD vs_version     = 0;
		DWORD ps_version     = 0;

		VideoDX9DeviceInfo* dev_info = dx9enum->GetDeviceInfo(video_settings.GetDeviceType());

		if (dev_info) {
			max_stages   = (int)  dev_info->caps.MaxTextureBlendStages;
			max_textures = (int)  dev_info->caps.MaxSimultaneousTextures;
			multiply_add =       (dev_info->caps.TextureOpCaps & D3DTEXOPCAPS_MULTIPLYADD) ? true : false;
			dotproduct3  =       (dev_info->caps.TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3) ? true : false;

			vs_version   = video_settings.enable_vs ? dev_info->caps.VertexShaderVersion : 0;
			ps_version   = video_settings.enable_ps ? dev_info->caps.PixelShaderVersion  : 0;

			vertexshader = vs_version >= D3DVS_VERSION(1,1);
			pixelshader  = ps_version >= D3DPS_VERSION(2,0);
		}

		strategy = DX9_STRATEGY_SIMPLE;
		passes   = 1;

		if (m->tex_alternate) {
			if (m->tex_detail && max_textures > 2 && max_stages > 4)
			strategy = DX9_STRATEGY_BLEND_DETAIL;

			else if (max_textures > 1 && max_stages > 3)
			strategy = DX9_STRATEGY_BLEND;
		}

		else if (m->tex_emissive && (!m->tex_diffuse || m->tex_diffuse == m->tex_emissive)) {
			strategy = DX9_STRATEGY_GLOW;
		}

		else if (IsSpecMapEnabled() && m->tex_specular && !m->tex_emissive) {
			strategy = DX9_STRATEGY_SPECMAP;

			if (max_textures < 2 || max_stages < 2 || !multiply_add)
			passes = 2;
		}

		else if ((!IsSpecMapEnabled() || !m->tex_specular) && m->tex_emissive) {
			strategy = DX9_STRATEGY_EMISSIVE;

			if (max_textures < 2 || max_stages < 2)
			passes = 2;
		}

		else if (IsSpecMapEnabled() && m->tex_specular && m->tex_emissive) {
			strategy = DX9_STRATEGY_SPEC_EMISSIVE;

			if (max_textures < 2 || max_stages < 2)
			passes = 3;

			else if (max_textures < 3 || max_stages < 3 || !multiply_add)
			passes = 2;
		}
	}

	return passes;
}

bool
VideoDX9::SetupPass(int pass)
{
	if (pass < 0 || pass >= passes)
	return false;

	if (pass == 0) {
		D3DMATERIAL9         d3dmtl;
		IDirect3DTexture9*   texture_0 = 0;
		IDirect3DTexture9*   texture_1 = 0;
		IDirect3DTexture9*   texture_2 = 0;
		Bitmap*              tex_bmp_0 = 0;
		Bitmap*              tex_bmp_1 = 0;
		Bitmap*              tex_bmp_2 = 0;
		ColorValue           orig_spec = segment_material->Ks;
		HRESULT              hr        = E_FAIL;

		if (segment_material->tex_specular && passes > 1)
		segment_material->Ks = Color::Black;

		CreateD3DMaterial(d3dmtl, *segment_material);
		segment_material->Ks = orig_spec;

		hr = d3ddevice->SetMaterial(&d3dmtl);

		if (strategy == DX9_STRATEGY_SIMPLE) {
			tex_bmp_0 = segment_material->tex_diffuse;
		}

		else if (strategy == DX9_STRATEGY_BLEND) {
			tex_bmp_0 = segment_material->tex_diffuse;
			tex_bmp_1 = segment_material->tex_alternate;
		}

		else if (strategy == DX9_STRATEGY_BLEND_DETAIL) {
			tex_bmp_0 = segment_material->tex_diffuse;
			tex_bmp_1 = segment_material->tex_alternate;
			tex_bmp_2 = segment_material->tex_detail;
		}

		else if (strategy == DX9_STRATEGY_SPECMAP) {
			if (passes == 1) {
				tex_bmp_0 = segment_material->tex_diffuse;
				tex_bmp_1 = segment_material->tex_specular;
			}
			else {
				tex_bmp_0 = segment_material->tex_diffuse;
			}
		}

		else if (strategy == DX9_STRATEGY_EMISSIVE      && passes == 1 ||
				strategy == DX9_STRATEGY_SPEC_EMISSIVE && passes == 2) {
			if (segment_material->tex_diffuse) {
				tex_bmp_0 = segment_material->tex_diffuse;
				tex_bmp_1 = segment_material->tex_emissive;
			}
			else {
				tex_bmp_0 = segment_material->tex_emissive;
			}
		}

		else {
			tex_bmp_0 = segment_material->tex_emissive;
		}

		if (texcache && tex_bmp_0) {
			texture_0 = texcache->FindTexture(tex_bmp_0);

			hr = d3ddevice->SetTexture(0, texture_0);
			current_texture = texture_0;

			if (tex_bmp_1) {
				texture_1 = texcache->FindTexture(tex_bmp_1);
				hr = d3ddevice->SetTexture(1, texture_1);

				if (tex_bmp_2) {
					texture_2 = texcache->FindTexture(tex_bmp_2);
					hr = d3ddevice->SetTexture(2, texture_2);
				}
			}
		}
		else {
			hr = d3ddevice->SetTexture(0, 0);
			current_texture = 0;
		}

		SetBlendType(segment_material->blend);

		if (texture_0 && texture_1 && strategy == DX9_STRATEGY_BLEND) {
			d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,  D3DTA_DIFFUSE);

			d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP,    D3DTOP_BLENDCURRENTALPHA);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG1,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG2,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAARG1,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0); 

			d3ddevice->SetTextureStageState(2, D3DTSS_COLOROP,    D3DTOP_MODULATE);
			d3ddevice->SetTextureStageState(2, D3DTSS_COLORARG1,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(2, D3DTSS_COLORARG2,  D3DTA_DIFFUSE);
			d3ddevice->SetTextureStageState(2, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(2, D3DTSS_ALPHAARG1,  D3DTA_CURRENT);

			d3ddevice->SetTextureStageState(3, D3DTSS_COLOROP,    D3DTOP_DISABLE);
		}

		else if (texture_0 && texture_1 && texture_2 && strategy == DX9_STRATEGY_BLEND_DETAIL) {
			d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,  D3DTA_DIFFUSE);
			d3ddevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0); 

			d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP,    D3DTOP_BLENDCURRENTALPHA);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG1,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG2,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAARG1,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0); 

			d3ddevice->SetTextureStageState(2, D3DTSS_COLOROP,    D3DTOP_BLENDTEXTUREALPHA);
			d3ddevice->SetTextureStageState(2, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(2, D3DTSS_COLORARG2,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(2, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(2, D3DTSS_ALPHAARG1,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 1); 

			d3ddevice->SetTextureStageState(3, D3DTSS_COLOROP,    D3DTOP_MODULATE);
			d3ddevice->SetTextureStageState(3, D3DTSS_COLORARG1,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(3, D3DTSS_COLORARG2,  D3DTA_DIFFUSE);
			d3ddevice->SetTextureStageState(3, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(3, D3DTSS_ALPHAARG1,  D3DTA_CURRENT);
		}

		else if (texture_0 && strategy == DX9_STRATEGY_GLOW) {
			d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG2,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,  D3DTA_TEXTURE);
		}

		else if (texture_0) {
			d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP,    D3DTOP_MODULATE);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG2,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,  D3DTA_TEXTURE);
		}

		if (texture_1 && strategy == DX9_STRATEGY_SPECMAP) {
			d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			d3ddevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP,    D3DTOP_MULTIPLYADD);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG2,  D3DTA_SPECULAR);
			d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAARG1,  D3DTA_TEXTURE);
		}

		else if (texture_1 && strategy == DX9_STRATEGY_EMISSIVE) {
			d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			d3ddevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP,    D3DTOP_ADD);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG2,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAARG1,  D3DTA_TEXTURE);
		}

		else if (texture_1 && strategy == DX9_STRATEGY_SPEC_EMISSIVE) {
			d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			d3ddevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP,    D3DTOP_ADD);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG2,  D3DTA_CURRENT);
			d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAARG1,  D3DTA_TEXTURE);
		}

		else if (strategy < DX9_STRATEGY_BLEND) {
			d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP,    D3DTOP_DISABLE);
		}
	}

	else if (pass == 1) {
		D3DMATERIAL9         d3dmtl;
		IDirect3DTexture9*   texture  = 0;
		Bitmap*              tex_bmp  = 0;
		ColorValue           orig_Ka  = segment_material->Ka;
		ColorValue           orig_Kd  = segment_material->Kd;
		HRESULT              hr       = E_FAIL;

		if (segment_material->tex_specular) {
			segment_material->Ka = Color::Black;
			segment_material->Kd = Color::Black;
		}

		CreateD3DMaterial(d3dmtl, *segment_material);

		segment_material->Ka = orig_Ka;
		segment_material->Kd = orig_Kd;

		hr = d3ddevice->SetMaterial(&d3dmtl);

		if (strategy == DX9_STRATEGY_SPECMAP ||
				strategy == DX9_STRATEGY_SPEC_EMISSIVE) {
			tex_bmp = segment_material->tex_specular;
			d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP,    D3DTOP_MODULATE);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG2,  D3DTA_SPECULAR);
		}

		else if (strategy == DX9_STRATEGY_EMISSIVE) {
			tex_bmp = segment_material->tex_emissive;
			d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP,    D3DTOP_SELECTARG1);
			d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1,  D3DTA_TEXTURE);
		}

		d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP,    D3DTOP_DISABLE);

		if (texcache && tex_bmp) {
			texture = texcache->FindTexture(tex_bmp);
			hr = d3ddevice->SetTexture(0, texture);
			current_texture = texture;

			SetBlendType(BLEND_ADDITIVE);
		}
	}

	if (render_state[STENCIL_ENABLE]) {
		d3ddevice->SetRenderState(D3DRS_STENCILENABLE,  TRUE);
		d3ddevice->SetRenderState(D3DRS_STENCILREF,     0x01);
		d3ddevice->SetRenderState(D3DRS_STENCILFUNC,    D3DCMP_GREATER);
	}
	else {
		d3ddevice->SetRenderState(D3DRS_STENCILENABLE,  FALSE);
	}

	if (render_state[LIGHTING_PASS] > 0) {
		SetBlendType(BLEND_ADDITIVE);
		SetRenderState(Z_WRITE_ENABLE, FALSE);
	}

	return true;
}


// +--------------------------------------------------------------------+

void
VideoDX9Error(const char* msg, HRESULT err)
{
	Print("   VideoDX9: %s. [%s]\n", msg, D3DErrStr(err));
}

char* D3DErrStr(HRESULT hr)
{
	static char errstrbuf[128];

	switch (hr) {
	default:
		sprintf_s(errstrbuf, "Unrecognized error value = %08x.", hr);
		return errstrbuf;

	case D3D_OK:
		return "No error.";

	case D3DERR_WRONGTEXTUREFORMAT:
		return "Wrong texture format.";

	case D3DERR_UNSUPPORTEDCOLOROPERATION:
		return "Unsupported color operation.";

	case D3DERR_UNSUPPORTEDCOLORARG:
		return "Unsupported color argument.";

	case D3DERR_UNSUPPORTEDALPHAOPERATION:
		return "Unsupported alpha operation.";

	case D3DERR_UNSUPPORTEDALPHAARG:
		return "Unsupported alpha argument.";

	case D3DERR_TOOMANYOPERATIONS:
		return "Too many operations.";

	case D3DERR_CONFLICTINGTEXTUREFILTER:
		return "Conflicting texture filter.";

	case D3DERR_UNSUPPORTEDFACTORVALUE:
		return "Unsupported factor value.";

	case D3DERR_CONFLICTINGRENDERSTATE:
		return "Conflicting render state.";

	case D3DERR_UNSUPPORTEDTEXTUREFILTER:
		return "Unsupported texture filter.";

	case D3DERR_CONFLICTINGTEXTUREPALETTE:
		return "Conflicting texture palette.";

	case D3DERR_DRIVERINTERNALERROR:
		return "Driver internal error.";


	case D3DERR_NOTFOUND:
		return "Resource was not found.";

	case D3DERR_MOREDATA:
		return "More data?";

	case D3DERR_DEVICELOST:
		return "Device lost.";

	case D3DERR_DEVICENOTRESET:
		return "Device is not reset.";

	case D3DERR_NOTAVAILABLE:
		return "Not available.";

	case D3DERR_OUTOFVIDEOMEMORY:
		return "Out of video memory.";

	case E_OUTOFMEMORY:
		return "Out of system memory.";

	case D3DERR_INVALIDDEVICE:
		return "Invalid device selection.";

	case D3DERR_INVALIDCALL:
		return "Invalid call or parameter.";

	case D3DERR_DRIVERINVALIDCALL:
		return "Driver invalid call.";

	case D3DERR_WASSTILLDRAWING:
		return "The device was still drawing.";

	case D3DOK_NOAUTOGEN:
		return "Autogeneration is not supported by this device.";

	}
}

