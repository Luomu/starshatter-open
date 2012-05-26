/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         VideoDX9.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Direct3D and Direct3D Video classes for DirectX 7 
*/

#ifndef VideoDX9_h
#define VideoDX9_h

#include "Video.h"
#include "VideoSettings.h"
#include "List.h"

// +--------------------------------------------------------------------+

class  VideoDX9;
class  VideoDX9Enum;
class  VideoDX9VertexBuffer;
class  VideoDX9IndexBuffer;
struct VideoDX9ScreenVertex;
class  Surface;
class  Segment;

struct VideoDX9ScreenVertex;
struct VideoDX9SolidVertex;
struct VideoDX9LuminousVertex;
struct VideoDX9LineVertex;


// +--------------------------------------------------------------------+

class VideoDX9 : public Video
{
public:
	VideoDX9(const HWND& window, VideoSettings* vs);
	virtual ~VideoDX9();

	virtual const VideoSettings* 
	GetVideoSettings() const { return &video_settings; }
	virtual bool   SetVideoSettings(const VideoSettings* vs);

	virtual bool   SetBackgroundColor(Color c);
	virtual bool   SetGammaLevel(int g);
	virtual bool   SetObjTransform(const Matrix& o, const Point& l);

	virtual bool   SetupParams();
	virtual bool   Reset(const VideoSettings* vs);

	virtual bool   StartFrame();
	virtual bool   EndFrame();

	virtual int    Width()     const { return width;   }
	virtual int    Height()    const { return height;  }
	virtual int    Depth()     const { return bpp;     }

	virtual void   RecoverSurfaces();

	virtual bool   ClearAll();
	virtual bool   ClearDepthBuffer();
	virtual bool   Present();
	virtual bool   Pause();
	virtual bool   Resume();

	virtual IDirect3D9*        Direct3D()     const { return d3d; }
	virtual IDirect3DDevice9*  D3DDevice()    const { return d3ddevice; }
	static  IDirect3DDevice9*  GetD3DDevice9();

	virtual bool   IsModeSupported(int width, int height, int bpp) const;
	virtual bool   IsHardware()               const { return true;   }
	virtual int    ZDepth()                   const { return zdepth; }
	virtual DWORD  VidMemFree()               const;
	virtual int    D3DLevel()                 const { return 9;      }
	virtual int    MaxTexSize()               const;
	virtual int    MaxTexAspect()             const;
	virtual int    GammaLevel()               const { return gamma;  }

	virtual bool   Capture(Bitmap& bmp);
	virtual bool   GetWindowRect(Rect& r);
	virtual bool   SetWindowRect(const Rect& r);
	virtual bool   SetViewport(int x, int y, int w, int h);
	virtual bool   SetCamera(const Camera* cam);
	virtual bool   SetEnvironment(Bitmap** faces);
	virtual bool   SetAmbient(Color c);
	virtual bool   SetLights(const List<Light>& lights);
	virtual bool   SetProjection(float fov, 
	float znear=1.0f, 
	float zfar=1.0e6f, 
	DWORD type=PROJECTION_PERSPECTIVE);
	virtual bool   SetRenderState(RENDER_STATE state, DWORD value);
	virtual bool   SetBlendType(int blend_type);

	virtual bool   DrawPolys(int npolys, Poly* p);
	virtual bool   DrawScreenPolys(int npolys, Poly* p, int blend=0);
	virtual bool   DrawSolid(Solid* s, DWORD blend_modes=0xf);
	virtual bool   DrawShadow(Solid* s, int nverts, Vec3* verts, bool vis=false);
	virtual bool   DrawLines(int nlines, Vec3* v, Color c, int blend=0);
	virtual bool   DrawScreenLines(int nlines, float* v, Color c, int blend=0);
	virtual bool   DrawPoints(VertexSet* v);
	virtual bool   DrawPolyOutline(Poly* p);
	virtual bool   UseMaterial(Material* m);

	virtual bool   UseXFont(const char* name, int size, bool b, bool i);
	virtual bool   DrawText(const char* text, int count, const Rect& rect,
	DWORD format, Color c);

	virtual void   PreloadTexture(Bitmap* bmp);
	virtual void   PreloadSurface(Surface* s);
	virtual void   InvalidateCache();

	static  void   CreateD3DMatrix(D3DMATRIX& result, const Matrix& m, const Point& p);
	static  void   CreateD3DMatrix(D3DMATRIX& result, const Matrix& m, const Vec3&  v);
	static  void   CreateD3DMaterial(D3DMATERIAL9& result, const Material& mtl);

private:
	bool           CreateBuffers();
	bool           DestroyBuffers();
	bool           PopulateScreenVerts(VertexSet* vset);
	bool           PrepareSurface(Surface* s);
	bool           DrawSegment(Segment* s);

	int            PrepareMaterial(Material* m);
	bool           SetupPass(int n);

	HWND                    hwnd;
	int                     width;
	int                     height;
	int                     bpp;
	int                     gamma;
	int                     zdepth;
	Color                   background;

	VideoDX9Enum*           dx9enum;
	VideoSettings           video_settings;

	IDirect3D9*             d3d;
	IDirect3DDevice9*       d3ddevice;
	D3DPRESENT_PARAMETERS   d3dparams;
	D3DSURFACE_DESC         back_buffer_desc;
	bool                    device_lost;

	BYTE*                   surface;

	DWORD                   texture_format[3];
	D3DGAMMARAMP            gamma_ramp;
	double                  fade;

	Rect                    rect;

	IDirect3DVertexDeclaration9*  vertex_declaration;
	ID3DXEffect*                  magic_fx;
	BYTE*                         magic_fx_code;
	int                           magic_fx_code_len;

	IDirect3DTexture9*      current_texture;
	int                     current_blend_state;
	int                     scene_active;
	DWORD                   render_state[RENDER_STATE_MAX];
	Material*               use_material;

	Material*               segment_material;
	int                     strategy;
	int                     passes;

	ID3DXFont*              d3dx_font;
	char                    font_name[64];
	int                     font_size;
	bool                    font_bold;
	bool                    font_ital;

	Color                   ambient;
	int                     nlights;

	int                     first_vert;
	int                     num_verts;

	VideoDX9VertexBuffer*   screen_vbuf;
	VideoDX9IndexBuffer*    screen_ibuf;
	VideoDX9ScreenVertex*   font_verts;
	WORD*                   font_indices;
	int                     font_nverts;

	VideoDX9ScreenVertex*   screen_line_verts;
	VideoDX9LineVertex*     line_verts;
};

#endif VideoDX9_h

