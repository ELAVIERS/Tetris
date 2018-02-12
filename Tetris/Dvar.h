#pragma once

typedef enum {
	DVT_FUNCTION,
	DVT_FLOAT,
	DVT_STRING
} DvarType;

typedef float DFloat;
typedef void DFunc(const char **tokens, unsigned int count);
typedef DFunc* DFuncPtr;

typedef union {
	DFloat dfloat;
	DFuncPtr dfunc;
	char *dstring;
} DvarValue;

typedef void DvarCallback(DvarValue);
typedef DvarCallback* DvarCallbackPtr;

typedef void* HDvar;

void AddDvarC(const char *name, DvarType, DvarValue, DvarCallbackPtr);
void SetDvar(HDvar, DvarValue);

HDvar GetDvar(const char *name);
const char*	HDvarName(HDvar);
char*		HDvarValueAsString(HDvar);
float		HDFloatValue(HDvar);

inline void AddDFloatC(const char *name, DFloat value, DvarCallbackPtr callback) {
	DvarValue v = { .dfloat = value };
	AddDvarC(name, DVT_FLOAT, v, callback);
}
inline void SetDFloat(HDvar dvar, DFloat value) {
	DvarValue v = { .dfloat = value };
	SetDvar(dvar, v);
}

inline void AddDFunctionC(const char *name, DFuncPtr value, DvarCallbackPtr callback) {
	DvarValue v = { .dfunc = value };
	AddDvarC(name, DVT_FUNCTION, v, callback);
}
inline void SetDFunction(HDvar dvar, DFuncPtr value) {
	DvarValue v = { .dfunc = value };
	SetDvar(dvar, v);
}

inline void AddDStringC(const char *name, char *value, DvarCallbackPtr callback) {
	DvarValue v = { .dstring = value };
	AddDvarC(name, DVT_STRING, v, callback);
}
inline void SetDString(HDvar dvar, char *value) {
	DvarValue v = { .dstring = value };
	SetDvar(dvar, v);
}


#define AddDFloat(NAME, VALUE)		AddDFloatC(NAME, VALUE, 0)
#define AddDFunction(NAME, VALUE)	AddDFunctionC(NAME, VALUE, 0)
#define AddDString(NAME, VALUE)		AddDStringC(NAME, VALUE, 0)

#define RegDStringC(DVAR, VALUE, CALLBACK) \
	AddDStringC(#DVAR, VALUE, CALLBACK); \
	DVAR = GetDvar(#DVAR)

void ListDvars();
void FreeDvars();

void HandleCommand(const char **tokens, unsigned int count);
