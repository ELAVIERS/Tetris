#pragma once
#include <stdbool.h>

typedef void DvarCallback(DvarValue);
typedef void DFunc(const char **tokens, unsigned int count);

typedef enum {
	DVT_CALL,
	DVT_FUNCTION,
	DVT_FLOAT,
	DVT_STRING
} DvarType;

typedef union DvarValue_u {
	void (*call)();
	DFunc*	function;
	float	number;
	char*	string;
} DvarValue;

/*
	Dvar(struct)

	name		the name of the variable
	value		the value of the variable
	type		specifies what type of data value holds
	callback	function to call when value is changed
*/
typedef struct Dvar_s {
	char *name;
	DvarType type;
	DvarValue value;
	DvarCallback *callback;

	bool server;
} Dvar;

//Dvar creation
Dvar* AddDvarC(const char *name, DvarType, DvarValue, DvarCallback*, bool server);

inline Dvar* AddDCallC(const char *name, void (*value)(), DvarCallback *callback, bool server) {
	DvarValue v = { .call = value };
	return AddDvarC(name, DVT_CALL, v, callback, server);
}

inline Dvar* AddDFunctionC(const char *name, DFunc *value, DvarCallback *callback, bool server) {
	DvarValue v = { .function = value };
	return AddDvarC(name, DVT_FUNCTION, v, callback, server);
}

inline Dvar* AddDFloatC(const char *name, float value, DvarCallback *callback, bool server) {
	DvarValue v = { .number = value };
	return AddDvarC(name, DVT_FLOAT, v, callback, server);
}

inline Dvar* AddDStringC(const char *name, char *value, DvarCallback *callback, bool server) {
	DvarValue v = { .string = value };
	return AddDvarC(name, DVT_STRING, v, callback, server);
}

#define AddDCall(NAME, VALUE, SERVER)		AddDCallC(NAME, VALUE, 0, SERVER)
#define AddDFunction(NAME, VALUE, SERVER)	AddDFunctionC(NAME, VALUE, 0, SERVER)
#define AddDFloat(NAME, VALUE, SERVER)		AddDFloatC(NAME, VALUE, 0, SERVER)
#define AddDString(NAME, VALUE, SERVER)		AddDStringC(NAME, VALUE, 0, SERVER)

//Dvar retrieval
Dvar* GetDvar(const char *name);
char* DvarAllocValueString(const Dvar *dvar);

/*
	DvarGetCommandString

	gets command string that would set dvar to its current value
	returns length of string
*/
unsigned int DvarGetCommandString(const Dvar *dvar, char dest[], unsigned int dest_size);

//
void SetDvar(Dvar *dvar, DvarValue value, bool print);
inline void SetDvarFloat(Dvar *dvar, float value, bool print) {
	DvarValue v = { .number = value };
	SetDvar(dvar, v, print);
}
inline void SetDvarString(Dvar *dvar, char *string, bool print) {
	DvarValue v = { .string = string };
	SetDvar(dvar, v, print);
}

//Dvar commands
void DvarCommand(Dvar *dvar, const char **tokens, unsigned int count, bool print);
void HandleCommandTokens(const char **tokens, unsigned int count);
void HandleCommandString(const char *command, bool message_server);

////
void ListDvars();
void FreeDvars();

void SendServerDvars(int playerid);
