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
#include <GL/wglew.h>
#include <stdbool.h>
#include <stdio.h>
#include <Windows.h>

/*
	Main.c
	Entry point

	Does all of the important stuff, y'know
*/

Mat3 projection;

void Frame(), Render(), FullscreenToggle();

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
		g_width = LOWORD(lparam);
		g_height = HIWORD(lparam);

		glViewport(0, 0, LOWORD(lparam), HIWORD(lparam));
		Mat3Ortho(projection, LOWORD(lparam), HIWORD(lparam));

		TEMP_UpdateBoardSize();

		Render();
		break;

	case WM_KEYDOWN:
		switch (wparam) {
		case VK_F11:
			FullscreenToggle();
			break;
		}

		if (g_menu_active) {
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
		}
		else {
			switch (wparam) {
			case VK_UP:
				while (BoardInputDown(g_board));
				break;
			case VK_DOWN:
				BoardInputDown(g_board);
				break;
			case VK_LEFT:
				BoardInputLeft(g_board);
				break;
			case VK_RIGHT:
				BoardInputRight(g_board);
				break;
			case 'Z':
				BoardInputCCW(g_board);
				break;
			case 'X':
				BoardInputCW(g_board);
				break;

			case VK_ESCAPE:
				BoardFree(g_board);
				g_board = NULL;
				CreateMainMenu();
			case VK_F1:
				UseNextTextureLevel();
				break;
			}
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

HWND window;
HDC device_context;
GLuint shader;
GLuint textshader;

//
//Window initialisation
//Register the window class and create the window
//
inline void InitWindow(HINSTANCE instance) {
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

	window = CreateWindowA("tetris_window", "Tetris", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, instance, NULL);

	device_context = GetDC(window);

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
}

inline void InitShaders(HINSTANCE instance) {
	char *frag_src = LoadStringResource(instance, ID_SHADER_FRAG);
	char *vert_src = LoadStringResource(instance, ID_SHADER_VERT);
	char *text_frag_src = LoadStringResource(instance, ID_SHADER_TEXTFRAG);

	if (frag_src == NULL || vert_src == NULL || text_frag_src == NULL) {
		ErrorMessage("Would you care to explain as to why the shaders don't exist mate?");
		return;
	}

	shader = CreateShaderProgram(frag_src, vert_src);
	textshader = CreateShaderProgram(text_frag_src, vert_src);
}

HDvar sv_gravity;

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd_str, int cmd_show) {
	InitWindow(instance);
	InitCommonControls();

	//
	//OpenGL initialisation
	//
	HGLRC glcontext = wglCreateContext(device_context);
	wglMakeCurrent(device_context, glcontext);

	glewInit();
	glClearColor(0.f, .2f, 0.f, 1.f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	wglSwapIntervalEXT(0); //No vsync

	//
	//Other
	//
	InitShaders(instance);

	G_Init();
	ConsoleInit();
	SettingsInit();
	TimerInit();

	ConsolePrint("Running config.cfg...\n");
	RunConfig("config.cfg");
	ConsolePrint("Done!\n");

	sv_gravity = GetDvar("sv_gravity");

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

		Frame();
	}

	//
	//Close Game
	//
	G_Free();
	FreeDvars();

	return 0;
}

float next_block_timer = 0.f;
const int title_size = 64;
char title[64];

void Frame() {
	TimerStart();
	snprintf(title, title_size, "Tetris (%d FPS)", (int)(1.f / g_delta));
	SetWindowTextA(window, title);

	if (g_board) {
		next_block_timer += g_delta;
		if (next_block_timer >= 1.f / HDFloatValue(sv_gravity)) {
			next_block_timer = 0.f;
			BoardInputDown(g_board);
		}
	}

	Render();
	g_delta = TimerDelta();
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (g_board) {
		UseGLProgram(shader);
		ShaderSetUniformMat3(shader, "u_projection", projection);
		BoardRender(g_board);
	}

	UseGLProgram(textshader);
	ShaderSetUniformMat3(textshader, "u_projection", projection);
	Menus_Render();

	SwapBuffers(device_context);
}

void FullscreenToggle() {
	static bool		fullscreen = false;
	static bool		maximised;
	static RECT		winSize;
	static HMENU	menu;

	fullscreen = !fullscreen;

	if (fullscreen)
	{
		maximised = IsZoomed(window);

		HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
		MONITORINFO info = { sizeof(info) };
		GetMonitorInfo(monitor, &info);

		GetWindowRect(window, &winSize);

		SetWindowLong(window, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(window, NULL, 0, 0, info.rcMonitor.right, info.rcMonitor.bottom, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
	else
	{
		SetWindowLong(window, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

		if (maximised)
			SendMessage(window, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		else
			SetWindowPos(window, NULL, winSize.left, winSize.top, winSize.right - winSize.left, winSize.bottom - winSize.top,
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}
