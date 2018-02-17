#pragma once

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
} Dvar;

//Dvar creation
Dvar* AddDvarC(const char *name, DvarType, DvarValue, DvarCallback*);

inline Dvar* AddDCallC(const char *name, void (*value)(), DvarCallback *callback) {
	DvarValue v = { .call = value };
	return AddDvarC(name, DVT_CALL, v, callback);
}

inline Dvar* AddDFunctionC(const char *name, DFunc *value, DvarCallback *callback) {
	DvarValue v = { .function = value };
	return AddDvarC(name, DVT_FUNCTION, v, callback);
}

inline Dvar* AddDFloatC(const char *name, float value, DvarCallback *callback) {
	DvarValue v = { .number = value };
	return AddDvarC(name, DVT_FLOAT, v, callback);
}

inline Dvar* AddDStringC(const char *name, char *value, DvarCallback *callback) {
	DvarValue v = { .string = value };
	return AddDvarC(name, DVT_STRING, v, callback);
}

#define AddDCall(NAME, VALUE)		AddDCallC(NAME, VALUE, 0)
#define AddDFunction(NAME, VALUE)	AddDFunctionC(NAME, VALUE, 0)
#define AddDFloat(NAME, VALUE)		AddDFloatC(NAME, VALUE, 0)
#define AddDString(NAME, VALUE)		AddDStringC(NAME, VALUE, 0)

//Dvar retrieval
Dvar* GetDvar(const char *name);
char* DvarAllocValueString(const Dvar *dvar);

//
void SetDvar(Dvar *dvar, DvarValue value);
inline void SetDvarFloat(Dvar *dvar, float value) {
	DvarValue v = { .number = value };
	SetDvar(dvar, v);
}
inline void SetDvarString(Dvar *dvar, char *string) {
	DvarValue v = { .string = string };
	SetDvar(dvar, v);
}

//Dvar commands
void DvarCommand(Dvar *dvar, const char **tokens, unsigned int count);
void HandleCommand(const char **tokens, unsigned int count);

////
void ListDvars();
void FreeDvars();
