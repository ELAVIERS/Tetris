#include "Bitmap.h"
#include "Error.h"
#include "IO.h"
#include "Resource.h"
#include "Shader.h"
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
	//
	//Other
	//
	//Obtain shader source strings from resource
	char *frag_src = LoadStringResource(instance, SHADER_FRAG);
	char *vert_src = LoadStringResource(instance, SHADER_VERT);

	if (frag_src == NULL || vert_src == NULL) {
		ErrorMessage("Would you care to explain as to why the shaders don't exist mate?");
		return 0;
	}

	g_shader = CreateShaderProgram(frag_src, vert_src);

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

		Render();
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
typedef struct {
	float position[2];
	float uv[2];
} Test_Vertex;

GLuint test_vao;
GLuint test_vbo;
GLuint test_texture;
Bitmap test_bmp;

void Test_Init() {
	Test_Vertex quad_verts[6] = {
		//	 POSITION		   UV
		{-1.f,		-1.f,	0.f, 0.f},
		{ 1.f,		-1.f,	1.f, 0.f },
		{ -1.f,		1.f,	0.f, 1.f },
		{ 1.f,		1.f,	1.f, 1.f },
		{ -1.f,		1.f,	0.f, 1.f },
		{ 1.f,		-1.f,	1.f, 0.f }
	};

	//Generate VAO and VBO
	glGenVertexArrays(1, &test_vao);
	glGenBuffers(1, &test_vbo);

	//Set VBO data
	glBindVertexArray(test_vao);
	glBindBuffer(GL_ARRAY_BUFFER, test_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);

	//Set VAO (shader) attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Test_Vertex), (GLvoid*)offsetof(Test_Vertex, position));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Test_Vertex), (GLvoid*)offsetof(Test_Vertex, uv));

	test_bmp = LoadBMP("Textures/bitmap.bmp");

	//Generate and bind texture, set data and parameters
	glGenTextures(1, &test_texture);
	glBindTexture(GL_TEXTURE_2D, test_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 
		test_bmp.width, test_bmp.height, 0, GL_BGR, GL_UNSIGNED_BYTE, test_bmp.buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

//Render test vao and texture
void Test_Render() {
	glUseProgram(g_shader);
	glBindVertexArray(test_vao);
	glBindTexture(GL_TEXTURE_2D, test_texture);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}
