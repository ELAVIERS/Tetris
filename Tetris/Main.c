#include "Bitmap.h"
#include "Error.h"
#include "Resource.h"
#include "Shader.h"
#include "Text.h"
#include "Texture.h"
#include "Timing.h"
#include <GL/glew.h>
#include <stdbool.h>
#include <Windows.h>

/*
	Main.c
	Entry point

	Does all of the important stuff, y'know
*/

bool running = true;

void Render();

//Windows calls this when the main window receives a window message
LRESULT CALLBACK windowproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_DESTROY:
		running = false;
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_SIZE:
		glViewport(0, 0, LOWORD(lparam), HIWORD(lparam));
		Render();
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

//Temporary stuff
void Test_Init();
void Test_Render();

HDC g_devicecontext;
GLuint g_shader;
GLuint g_textshader;
float g_deltaseconds;
float g_runtime;

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd_str, int cmd_show) {
	//
	//Window initialisation
	//Register the window class and create the window
	//
	WNDCLASSEXA windowclass = {
		sizeof(WNDCLASSEXA),
		0,
		windowproc,
		0,
		0,
		instance,
		LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON)),
		LoadCursor(NULL, IDC_ARROW),
		0,
		NULL,
		"tetris_window",
		LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON))
	};
	RegisterClassExA(&windowclass);

	HWND window = CreateWindowA("tetris_window", "", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, instance, NULL);

	g_devicecontext = GetDC(window);

	//
	//OpenGL initialisation
	//Find and set the pixel format, set the opengl context, general opengl init
	//
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,							//cColorBits
		0, 0, 0, 0, 0, 0, 0, 0,		//*
		0, 0, 0, 0, 0,				//*
		32,							//cDepthBits
		0,							//*
		0,							//*
		0,							//Ignored
		0,							//*
		0,							//*	
		0,							//*
		0							//* - Not relevant for finding PFD
	};

	int pfd_id = ChoosePixelFormat(g_devicecontext, &pfd);
	SetPixelFormat(g_devicecontext, pfd_id, &pfd);

	HGLRC glcontext = wglCreateContext(g_devicecontext);
	wglMakeCurrent(g_devicecontext, glcontext);

	glewInit();
	glClearColor(0.f, .2f, 0.f, 1.f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//
	//Other
	//
	//Obtain shader source strings from resource
	char *frag_src =		LoadStringResource(instance, SHADER_FRAG);
	char *vert_src =		LoadStringResource(instance, SHADER_VERT);
	char *text_frag_src = LoadStringResource(instance, SHADER_TEXTFRAG);

	if (frag_src == NULL || vert_src == NULL || text_frag_src == NULL) {
		ErrorMessage("Would you care to explain as to why the shaders don't exist mate?");
		return 0;
	}

	g_shader = CreateShaderProgram(frag_src, vert_src);
	g_textshader = CreateShaderProgram(text_frag_src, vert_src);

	InitTimer();

	Test_Init();


	//
	//Start Game
	//Show the window, run message loop, and call frames 
	//
	ShowWindow(window, cmd_show);

	MSG msg;
	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		StartTimer();
		g_runtime += g_deltaseconds;

		Render();
		g_deltaseconds = GetDeltaTime();
	}

	return 0;
}

void Render() {
	glClearColor(0.f, 0.f, 0.f, 1.f);

	Test_Render();

	SwapBuffers(g_devicecontext);
}

//
//TEMPORARY THINGS
//
Text test_text;
GLuint test_texture;

void Test_Init() {
	test_text = CreateText();

	SetTextString(&test_text, "AYY LMAO");

	Bitmap bmp = LoadBMP("Textures/Font.bmp");
	test_texture = TextureFromBMP(&bmp, true);
}

//Render test vao and texture
void Test_Render() {
	glUseProgram(g_textshader);
	glBindTexture(GL_TEXTURE_2D, test_texture);
	RenderText(&test_text);
}
