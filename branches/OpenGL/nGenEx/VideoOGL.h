/*****************************************************************************
	OpenGL Renderer for Starshatter
	Fabian "The E" Woltermann

******************************************************************************/

#ifndef VideoOGL_h
#define VideoOGL_h

#include "Video.h"
#include "VideoSettings.h"

#define GLEW_STATIC
#include <WinDef.h>
#include <windows.h>
#include <windowsx.h>
#include <direct.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/GL.h>
#include <GL/wglext.h>
#include <GL/glext.h>


class VideoOGL : public Video {
	VideoOGL(const HWND& window, VideoSettings* vs);
	virtual ~VideoOGL();

private:
	HDC GL_device_context;
	HGLRC GL_render_context;
	PIXELFORMATDESCRIPTOR GL_pfd;

	int MaximumSupportedTexSize;
	int MaximumSupportedElementVertices;
	int MaximumSupportedElementIndices;
	int NumTextureUnits;
};

#endif