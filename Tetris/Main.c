#include "Bitmap.h"
#include "Config.h"
#include "Console.h"
#include "Dvar.h"
#include "Error.h"
#include "Globals.h"
#include "IO.h"
#include "Matrix.h"
#include "Menu.h"
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

float **projection;

void Render();

//Windows calls this when the main window receives a window message
LRESULT CALLBACK windowproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_DESTROY:
		g_running = false;
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_SIZE:
		glViewport(0, 0, LOWORD(lparam), HIWORD(lparam));
		Mat3Ortho(projection, LOWORD(lparam), HIWORD(lparam));
		Render();
		break;

	case WM_KEYDOWN:
		switch (wparam) {
		case VK_UP:
			ActiveMenu_ChangeSelection(1);
			break;
		case VK_DOWN:
			ActiveMenu_ChangeSelection(-1);
			break;
		case VK_RETURN:
			ActiveMenu_Select();
			break;
		}

		break;

	case WM_KEYUP:
		switch (wparam) {
		case VK_OEM_3: //~
			ConsoleOpen();
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

HDC device_context;
GLuint shader;
GLuint textshader;

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd_str, int cmd_show) {
	projection = Mat3Alloc();

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

	device_context = GetDC(window);

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

	int pfd_id = ChoosePixelFormat(device_context, &pfd);
	SetPixelFormat(device_context, pfd_id, &pfd);

	HGLRC glcontext = wglCreateContext(device_context);
	wglMakeCurrent(device_context, glcontext);

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

	shader = CreateShaderProgram(frag_src, vert_src);
	textshader = CreateShaderProgram(text_frag_src, vert_src);

	InitTimer();

	InitCommonControls();
	ConsoleInit();
	SettingsInit();

	ConsolePrint("Running config.cfg...\n");
	RunConfig("config.cfg");
	ConsolePrint("Done!\n");

	//
	//Start Game
	//Show the window, run message loop, and call frames 
	//
	CreateMainMenu();
	ShowWindow(window, cmd_show);
	g_running = true;

	MSG msg;
	while (g_running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		StartTimer();

		Render();
		g_delta = GetDeltaTime();
	}

	//
	//Close Game
	//
	Font_Free(g_font);
	Font_Free(g_menu_font);
	FreeDvars();

	return 0;
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	UseGLProgram(textshader);
	ShaderSetUniformMat3(textshader, "u_projection", projection);
	Menus_Render();

	SwapBuffers(device_context);
}
