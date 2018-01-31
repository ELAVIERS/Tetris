#include "Bitmap.h"
#include "Config.h"
#include "Console.h"
#include "Dvar.h"
#include "Error.h"
#include "IO.h"
#include "Resource.h"
#include "Settings.h"
#include "Shader.h"
#include "Text.h"
#include "Texture.h"
#include "Timing.h"
#include <CommCtrl.h>
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

	case WM_KEYUP:
		switch (wparam) {
		case VK_OEM_3: //~
			ConsoleOpen();
			break;
		case VK_RETURN:
			SettingsOpen();
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

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

	HWND window = CreateWindowA("tetris_window", "Tetris", WS_OVERLAPPEDWINDOW,
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
	char *frag_src =		LoadStringResource(instance, ID_SHADER_FRAG);
	char *vert_src =		LoadStringResource(instance, ID_SHADER_VERT);
	char *text_frag_src =	LoadStringResource(instance, ID_SHADER_TEXTFRAG);

	if (frag_src == NULL || vert_src == NULL || text_frag_src == NULL) {
		ErrorMessage("Would you care to explain as to why the shaders don't exist mate?");
		return 0;
	}

	g_shader = CreateShaderProgram(frag_src, vert_src);
	g_textshader = CreateShaderProgram(text_frag_src, vert_src);

	InitTimer();

	InitCommonControls();
	ConsoleInit();
	SettingsInit();

	RunConfig("config.cfg");

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

	//
	//Close Game
	//
	FreeDvars();

	return 0;
}

void Render() {
	glClearColor(0.f, 0.f, 0.f, 1.f);

	SwapBuffers(g_devicecontext);
}

/*
//
//TEMPORARY THINGS
//
Text test_text;
GLuint test_texture;

void Test_Init() {
	test_text = CreateText();

	//Yeah... no.
	//SetTextString(&test_text, "TETRIS IS A TILE - MATCHING PUZZLE VIDEO GAME, ORIGINALLY DESIGNED AND PROGRAMMED BY RUSSIAN GAME DESIGNER ALEXEY PAJITNOV.[1] IT WAS RELEASED ON JUNE 6, 1984, [2] WHILE HE WAS WORKING FOR THE DORODNITSYN COMPUTING CENTRE OF THE ACADEMY OF SCIENCE OF THE SOVIET UNION IN MOSCOW.[3] HE DERIVED ITS NAME FROM THE GREEK NUMERICAL PREFIX TETRA - (ALL OF THE GAME'S PIECES CONTAIN FOUR SEGMENTS) AND TENNIS, PAJITNOV'S FAVORITE SPORT.[4][5]");

	SetTextString(&test_text, "AYY LMAO");

	Bitmap bmp = LoadBMP("Textures/Font.bmp");
	test_texture = TextureFromBMP(&bmp, true);
}
*/
