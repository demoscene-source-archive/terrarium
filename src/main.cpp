// custom build and feature flags
#ifdef DEBUG
	#define OPENGL_DEBUG        1
	#define FULLSCREEN          0
	#define DESPERATE           0
	#define BREAK_COMPATIBILITY 0
#else
	#define OPENGL_DEBUG        0
	#define FULLSCREEN          1
	#define DESPERATE           0
	#define BREAK_COMPATIBILITY 0
#endif
 
   
#define POST_PASS    0
#define USE_MIPMAPS  1
#define USE_AUDIO    1
#define NO_UNIFORMS  1
#define WRITEWAV	 0
#define WGS			64

#include "definitions.h"
#if OPENGL_DEBUG
	#include "debug.h"
#else
#define CHECK_ERRORS() ;
#endif
        
      
#include "glext.h"
         
// static allocation saves a few bytes
#pragma data_seg(".pids")
static int pidMain;
static int pidPost;
// static HDC hDC;
           
static const char csSrc2[] =
#include "shaders/fragment.glsl.h"
;


static const char *strs[] = {
	"glCreateShaderProgramv",
	"glUseProgram",
	"glUniform1i",
	"glDispatchCompute",
	"glBindBuffer",
	"glBufferStorage",
	"glTexStorage2D",
	"glBindImageTexture",
	};

#define NUMFUNCIONES (sizeof(strs)/sizeof(strs[0]))

#define oglCreateShaderProgramv	      ((PFNGLCREATESHADERPROGRAMVPROC)myglfunc[0])
#define oglUseProgram									((PFNGLUSEPROGRAMPROC)myglfunc[1])
#define oglUniform1i									((PFNGLUNIFORM1IPROC)myglfunc[2])
#define oglDispatchCompute						((PFNGLDISPATCHCOMPUTEPROC)myglfunc[3])
#define oglBindBuffer								((PFNGLBINDBUFFERPROC)myglfunc[4])
#define oglBufferStorage	((PFNGLBUFFERSTORAGEPROC)myglfunc[5])
#define oglTexStorage2D   ((PFNGLTEXSTORAGE2DPROC)myglfunc[6])
#define oglBindImageTexture   ((PFNGLBINDIMAGETEXTUREPROC)myglfunc[7])


#ifndef EDITOR_CONTROLS
#pragma code_seg(".main")
void entrypoint(void)
#else
#include "editor.h"
#include "song.h"
int __cdecl main(int argc, char* argv[])
#endif
{
	// initialize window
	#if FULLSCREEN
		ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);
		ShowCursor(0);
		const HDC hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0));
	#else
		#ifdef EDITOR_CONTROLS
			HWND window = CreateWindow("static", 0, WS_POPUP | WS_VISIBLE, 0, 0, XRES, YRES, 0, 0, 0, 0);
			HDC hDC = GetDC(window);
		#else
			// you can create a pseudo fullscreen window by similarly enabling the WS_MAXIMIZE flag as above
			// in which case you can replace the resolution parameters with 0s and save a couple bytes
			// this only works if the resolution is set to the display device's native resolution
			HDC hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE, 0, 0, XRES, YRES, 0, 0, 0, 0));
		#endif
	#endif  

	// initalize opengl context
	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));
	
	void *myglfunc[NUMFUNCIONES];

	for (int i = 0; i < NUMFUNCIONES; i++)
	{
#ifdef WIN32
		myglfunc[i] = wglGetProcAddress(strs[i]);
#endif
#ifdef LINUX
		myglfunc[i] = glXGetProcAddress((const unsigned char *)strs[i]);
#endif
	}

	static const char* shp2 = csSrc2;

	// create and compile shader programs
	oglUseProgram(oglCreateShaderProgramv(GL_COMPUTE_SHADER, 1, &shp2));

#if WRITEWAV
	{
		_4klang_render(lpSoundBuffer);

		HANDLE hf = CreateFile("waveout.raw",
			GENERIC_READ | GENERIC_WRITE,
			(DWORD)0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL);
		DWORD dwTmp;

		if (!WriteFile(hf, lpSoundBuffer, sizeof(lpSoundBuffer),
			(LPDWORD)&dwTmp, NULL))
		{
		}

		ExitProcess(0);
	}
#endif

	// initialize sound
#ifndef EDITOR_CONTROLS
#if USE_AUDIO
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_4klang_render, lpSoundBuffer, 0, 0);
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL);
	waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
	waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));
#endif
#else
	Leviathan::Editor editor = Leviathan::Editor();
	editor.updateShaders(&pidMain, &pidPost, true);

	((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(pidMain);

	// absolute path always works here
	// relative path works only when not ran from visual studio directly
	Leviathan::Song track(L"audio.wav");
	track.play();
	double position = 0.0;
#endif

	// http://web.archive.org/web/20010208190716/http://nehe.gamedev.net/tutorials/lesson13.asp

	// WINGDIAPI HFONT   WINAPI CreateFontA( _In_ int cHeight, _In_ int cWidth, _In_ int cEscapement, _In_ int cOrientation, _In_ int cWeight, _In_ DWORD bItalic,
	// _In_ DWORD bUnderline, _In_ DWORD bStrikeOut, _In_ DWORD iCharSet, _In_ DWORD iOutPrecision, _In_ DWORD iClipPrecision,
	//	_In_ DWORD iQuality, _In_ DWORD iPitchAndFamily, _In_opt_ LPCSTR pszFaceName);

	SelectObject(hDC, CreateFont(260, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Bahnschrift"));
	wglUseFontBitmaps(hDC, 0, 256, 0);

	glGetError();
	CHECK_ERRORS();

	glClear(GL_COLOR_BUFFER_BIT);
	
	{
		glRasterPos2s(-1, 0);
		glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
		static const char str[] = "terrarium";
		glCallLists(sizeof(str) - 1, GL_UNSIGNED_BYTE, str);
	}

	{
		glRasterPos2s(-1, 0);
		glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
		static const char str[] = "noby & fizzer";
		glCallLists(sizeof(str) - 1, GL_UNSIGNED_BYTE, str);
	}

	{
		glRasterPos2s(-1, 0);
		glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
		static const char str[] = "eos 2019";
		glCallLists(sizeof(str) - 1, GL_UNSIGNED_BYTE, str);
	}

	glBindTexture(GL_TEXTURE_2D, 3);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, XRES, YRES, 0);


	glRasterPos2s(-1, -1);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	GLuint buf = 1;

	oglBindBuffer(GL_PIXEL_UNPACK_BUFFER, buf);
	oglBindBuffer(GL_PIXEL_PACK_BUFFER, buf);
	oglBufferStorage(GL_PIXEL_UNPACK_BUFFER, XRES*YRES * 4, 0, 0);

	glBindTexture(GL_TEXTURE_2D, 1);
	oglTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, XRES, YRES);

	CHECK_ERRORS();

	glBindTexture(GL_TEXTURE_2D, 2);
	oglTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, XRES, YRES);

	oglBindImageTexture(0, 1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	oglBindImageTexture(1, 2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	CHECK_ERRORS();

	// main loop
	do
	{
#ifdef EDITOR_CONTROLS
		editor.beginFrame(timeGetTime());
#endif

#if !(DESPERATE)
		// do minimal message handling so windows doesn't kill your application
		// not always strictly necessary but increases compatibility and reliability a lot
		// normally you'd pass an msg struct as the first argument but it's just an
		// output parameter and the implementation presumably does a NULL check
		PeekMessage(0, 0, 0, 0, PM_REMOVE);
#endif

#if 1

		// render with the primary shader
		//((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(pidMain);
#ifndef EDITOR_CONTROLS
	// if you don't have an audio system figure some other way to pass time to your shader
#if USE_AUDIO
		waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
		// it is possible to upload your vars as vertex color attribute (gl_Color) to save one function import
		auto tt = GLint(MMTime.u.sample);
		oglUniform1i(0, tt);
#endif
#else 
		position = track.getTime();
		auto tt = GLint(position*SAMPLE_RATE);
		oglUniform1i(0, tt);
#endif

		oglDispatchCompute(WGS, WGS, 1);

		CHECK_ERRORS();

		oglUniform1i(0, -tt);
		glBindTexture(GL_TEXTURE_2D, 3);
		oglDispatchCompute((XRES + 7) / 8, (YRES + 7) / 8, 1);

		CHECK_ERRORS();

		glBindTexture(GL_TEXTURE_2D, 2);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glDrawPixels(XRES, YRES, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		CHECK_ERRORS();
#endif


		SwapBuffers(hDC);

		// handle functionality of the editor
#ifdef EDITOR_CONTROLS
		editor.endFrame(timeGetTime());
		position = editor.handleEvents(window, hDC, &track, position);
		editor.printFrameStatistics();
		editor.updateShaders(&pidMain, &pidPost);
		((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(pidMain);
#endif

	} while (!GetAsyncKeyState(VK_ESCAPE)
#if USE_AUDIO
		&& MMTime.u.sample < MAX_SAMPLES
#endif
		);

	ExitProcess(0);
}
