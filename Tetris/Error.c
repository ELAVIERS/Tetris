#include "Error.h"

#include <Windows.h>

void ErrorMessage(const char* message) {
	MessageBeep(MB_ICONERROR);
	MessageBoxA(NULL, message, "Error", MB_OK);

#ifdef _DEBUG
	DebugBreak();
#endif
}
