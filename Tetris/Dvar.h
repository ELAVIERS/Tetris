#pragma once
#include <stdbool.h>

typedef enum {
	DVT_FUNCTION,
	DVT_FLOAT,
	DVT_STRING
} DvarType;

typedef float DFloat;
typedef char* DString;
typedef void (*DFunc)(const char **tokens, unsigned int count);

typedef union {
	DFunc function;
	DFloat number;
	DString string;
} DvarValue;

typedef struct Dvar_s {
	char *name;
	DvarType type;
	DvarValue data;
} Dvar;

void _AddDvar(const char *name, DvarType type, DvarValue value);
void* _GetDvar(const char *name, DvarType type);

#define DFLOAT(VAR) VAR = (DFloat*)_GetDvar(#VAR, DVT_FLOAT)
#define DSTRING(VAR) VAR = (DString*)_GetDvar(#VAR, DVT_STRING)
#define DFUNC(VAR) VAR = (DFunc*)_GetDvar(#VAR, DVT_FUNCTION)

inline void AddDFloat(const char *name, DFloat value) { 
	DvarValue data = {.number = value};
	_AddDvar(name, DVT_FLOAT, data);
}

inline void AddDString(const char *name, DString value) {
	DvarValue data = { .string = value };
	_AddDvar(name, DVT_STRING, data);
}

inline void AddDFunction(const char *name, DFunc value) {
	DvarValue data = { .function = value };
	_AddDvar(name, DVT_FUNCTION, data);
}

void FreeDvars();

void HandleCommand(const char **tokens, unsigned int count);
