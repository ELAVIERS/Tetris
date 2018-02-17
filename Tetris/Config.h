#pragma once
#include "Dvar.h"
#include <stdbool.h>

/*
	RunConfig

	Runs config file
	Returns false if no file was found
*/
bool RunConfig(const char *filepath);

/*
	AddCvar

	Adds dvar to the cvar linked list
*/
void AddCvar(const Dvar *dvar);

//
void SaveCvars();

//Frees the cvar linked list
void FreeCvars();
