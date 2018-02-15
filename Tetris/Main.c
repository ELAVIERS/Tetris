#include "Config.h"
#include "Console.h"
#include "Game.h"
#include "Globals.h"
#include "Menu.h"
#include "Settings.h"
#include "Timing.h"
#include "Window.h"
#include <CommCtrl.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <Windows.h>

/*
	Main.c
	Entry point

	Initialise stuff and run windows message loop
*/

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd_str, int cmd_show) {
	InitWindow(instance);
	InitCommonControls();

	//
	//OpenGL initialisation
	//
	HGLRC glcontext = wglCreateContext(g_devcontext);
	wglMakeCurrent(g_devcontext, glcontext);

	glewInit();
	glClearColor(0.f, .2f, 0.f, 1.f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	wglSwapIntervalEXT(0); //No vsync

	//
	//Other
	//
	G_Init();
	ConsoleInit();
	SettingsInit();
	TimerInit();

	GameInit();

	ConsolePrint("Running config.cfg...\n");
	RunConfig("config.cfg");
	ConsolePrint("Done!\n");

	//
	//Seed RNG
	//
	SYSTEMTIME st;
	GetSystemTime(&st);
	srand(st.wMilliseconds);

	//
	//Start Game
	//Show the window, run message loop, and call frames 
	//
	CreateMainMenu();
	ShowWindow(g_hwnd, cmd_show);
	g_running = true;
	g_menu_active = true;

	MSG msg;
	while (g_running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		GameFrame();
	}

	//
	//Close Game
	//
	G_Free();
	FreeDvars();

	return 0;
}
