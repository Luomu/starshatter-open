/*****************************************************************************
	OpenGL Renderer for Starshatter
	Fabian "The E" Woltermann

******************************************************************************/

#include "VideoOGL.h"

void
VideoOGLError(const char* msg)
{
	Print("   VideoOGL: %s. [%s]\n", msg, glGetError());
}

VideoOGL::VideoOGL(const HWND& window, VideoSettings* vs) {
	int PixelFormat;
	PIXELFORMATDESCRIPTOR pfd_test;

	Print("\n********************************\n");
	Print("*   OpenGL Renderer		       *\n");
	Print("********************************\n\n");

	memset(&GL_pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	memset(&pfd_test, 0, sizeof(PIXELFORMATDESCRIPTOR));

	GL_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	GL_pfd.nVersion = 1;
	GL_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	GL_pfd.iPixelType = PFD_TYPE_RGBA;
	GL_pfd.cColorBits = vs->GetDepth();
	switch (vs->GetPixelFormat()) {
		case VideoMode::FMT_NONE: {
			GL_pfd.cRedBits = GL_pfd.cBlueBits = GL_pfd.cGreenBits = GL_pfd.cAlphaBits = 0;
			break;
		}
		case VideoMode::FMT_R5G5B5: {
			GL_pfd.cRedBits = 5;
			GL_pfd.cGreenBits = 5;
			GL_pfd.cBlueBits = 5;
			GL_pfd.cAlphaBits = 0;
			break;
		}
		case VideoMode::FMT_R5G6B5: {
			GL_pfd.cRedBits = 5;
			GL_pfd.cGreenBits = 6;
			GL_pfd.cBlueBits = 5;
			GL_pfd.cAlphaBits = 0;
			break;
		}
		case VideoMode::FMT_R8G8B8: {
			GL_pfd.cRedBits = 8;
			GL_pfd.cGreenBits = 8;
			GL_pfd.cBlueBits = 8;
			GL_pfd.cAlphaBits = 0;
			break;
		}
		case VideoMode::FMT_X8R8G8B8: {
			GL_pfd.cRedBits = 8;
			GL_pfd.cGreenBits = 8;
			GL_pfd.cBlueBits = 8;
			GL_pfd.cAlphaBits = 8;
			break;
		}
	}
	GL_pfd.cDepthBits = (GL_pfd.cColorBits == 32) ? 24 : 16;

	GL_device_context = GetDC(window);

	if (!GL_device_context) {
		VideoOGLError("Unable to get device context for OpenGL");
		return;
	}
	
	PixelFormat = ChoosePixelFormat(GL_device_context, &GL_pfd);

	if (!PixelFormat) {
		VideoOGLError("Unable to get pixel format for OpenGL");
		return;
	} else {
		DescribePixelFormat(GL_device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd_test);
	}

	if ( !SetPixelFormat(GL_device_context, PixelFormat, &GL_pfd) ) {
		VideoOGLError("Unable to set pixel format for OpenGL W32!");
		return;
	}


	int attriblist[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 1, WGL_CONTEXT_CORE_PROFILE_BIT_ARB};
	GL_render_context = wglCreateContextAttribsARB(GL_device_context, GL_render_context, attriblist);

	if (!GL_render_context) {
		VideoOGLError("Unable to create render context!");
		return;
	}

	// Now let's see what we actually have to work with
	DescribePixelFormat(GL_device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &GL_pfd);

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaximumSupportedTexSize);
	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &MaximumSupportedElementVertices);
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &MaximumSupportedElementIndices);
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &NumTextureUnits);

	Print("OpenGL Implementation info:\n");
	Print("    OpenGL Vendor    : %s\n", glGetString(GL_VENDOR));
	Print("    OpenGL Renderer  : %s\n", glGetString(GL_RENDERER));
	Print("    OpenGL Version	: %s\n", glGetString(GL_VERSION));
	Print("    Using R%dG%dB%d pixel format, %d bpp\n", GL_pfd.cRedBits, GL_pfd.cGreenBits, GL_pfd.cBlueBits, GL_pfd.cDepthBits);
	Print("    Max texture size supported:  %dx%d\n", MaximumSupportedTexSize, MaximumSupportedTexSize);
	Print("    Max number of vertices: %d\n", MaximumSupportedElementVertices); 
	Print("    Max number of element indices: %d\n", MaximumSupportedElementIndices);
	Print("    Texture units available: %d\n", NumTextureUnits);
	Print("    GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	wglMakeCurrent(GL_device_context, GL_render_context);

	glewInit();
}