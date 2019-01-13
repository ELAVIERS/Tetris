#include "Config.h"
#include "Console.h"
#include "Game.h"
#include "Globals.h"
#include "Menu.h"
#include "Networking.h"
#include "Lobby.h"
#include "Server.h"
#include "Settings.h"
#include "SoundManager.h"
#include "Timing.h"
#include "Variables.h"
#include "Window.h"
#include <CommCtrl.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <objbase.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>

/*
	Main.c
	Entry point

	Initialise stuff and run windows message loop
*/

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd_str, int cmd_show) {
	InitWindow(instance);
	InitCommonControls();

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

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
	SMStart(16, 100);
	G_Init();
	CreateVariables();
	ConsoleInit();
	SettingsInit();
	LobbyInit();
	TimerInit();
	NetworkingInit();
	GameInit();
	MenuInit();
	StartLocalServer();
	
	ConsolePrint("Running config.cfg...\n");
	RunConfig("config.cfg");
	ConsolePrint("Done!\n");

	//
	//Seed RNG
	//
	srand((unsigned)time(NULL));
	rand();

	//
	//Start Game
	//Show the window, run message loop, and call frames 
	//
	CreateMenu_Main();
	ShowWindow(g_hwnd, cmd_show);
	g_running = true;

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
	WSACleanup();
	G_Free();
	FreeDvars();
	SMStop();

	return 0;
}
