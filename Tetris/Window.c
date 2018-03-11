#include "Window.h"
#include "Game.h"
#include "Globals.h"
#include "InputManager.h"
#include "Resource.h"
#include <stdbool.h>
#include <Windows.h>

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
		Mat3Ortho(g_projection, LOWORD(lparam), HIWORD(lparam));

		GameSizeUpdate(LOWORD(lparam), HIWORD(lparam));

		GameRender();
		break;

	case WM_KEYDOWN:
		if ((lparam & (1 << 30)) == 0)
			KeyDown((WORD)wparam);
		break;

	case WM_KEYUP:
		KeyUp((WORD)wparam);
		break;

	default: return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

void InitWindow(HINSTANCE instance) {
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

	g_hwnd = CreateWindowA("tetris_window", "Tetris", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, instance, NULL);

	g_devcontext = GetDC(g_hwnd);

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

	int pfd_id = ChoosePixelFormat(g_devcontext, &pfd);
	SetPixelFormat(g_devcontext, pfd_id, &pfd);
}

void FullscreenToggle() {
	static bool		fullscreen = false;
	static bool		maximised;
	static RECT		winSize;
	static HMENU	menu;

	fullscreen = !fullscreen;

	if (fullscreen)
	{
		maximised = IsZoomed(g_hwnd);

		HMONITOR monitor = MonitorFromWindow(g_hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO info = { sizeof(info) };
		GetMonitorInfo(monitor, &info);

		GetWindowRect(g_hwnd, &winSize);

		SetWindowLong(g_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(g_hwnd, NULL, 0, 0, info.rcMonitor.right, info.rcMonitor.bottom, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
	else
	{
		SetWindowLong(g_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

		if (maximised)
			SendMessage(g_hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		else
			SetWindowPos(g_hwnd, NULL, winSize.left, winSize.top, winSize.right - winSize.left, winSize.bottom - winSize.top,
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

/*{
case VK_ESCAPE:
BoardFree(g_board);
g_board = NULL;
CreateMainMenu();
case VK_F1:
UseNextTextureLevel();
break;
}
}*/
